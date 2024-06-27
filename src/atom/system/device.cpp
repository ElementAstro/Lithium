#include "device.hpp"

#include <string>
#include <vector>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <setupapi.h>
#include <bluetoothapis.h>
// clang-format on
#else
#include <dirent.h>
#include <fcntl.h>
#include <libusb-1.0/libusb.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#if __has_include(<bluetooth/bluetooth.h>)
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <memory>
#endif
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
#ifdef _WIN32
std::vector<DeviceInfo> enumerate_usb_devices() {
    std::vector<DeviceInfo> devices;
    HDEVINFO device_info_set = SetupDiGetClassDevs(
        nullptr, "USB", nullptr, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (device_info_set == INVALID_HANDLE_VALUE)
        return devices;

    SP_DEVINFO_DATA device_info_data;
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
    for (int i = 0;
         SetupDiEnumDeviceInfo(device_info_set, i, &device_info_data); i++) {
        DWORD data_type, size;
        char buffer[512];
        if (SetupDiGetDeviceRegistryProperty(
                device_info_set, &device_info_data, SPDRP_DEVICEDESC,
                &data_type, (PBYTE)buffer, sizeof(buffer), &size)) {
            devices.push_back({buffer, ""});
        }
    }

    SetupDiDestroyDeviceInfoList(device_info_set);
    return devices;
}

std::vector<DeviceInfo> enumerate_serial_ports() {
    std::vector<DeviceInfo> devices;
    HDEVINFO device_info_set =
        SetupDiGetClassDevs(nullptr, "COM", nullptr, DIGCF_PRESENT);
    if (device_info_set == INVALID_HANDLE_VALUE)
        return devices;

    SP_DEVINFO_DATA device_info_data;
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);
    for (int i = 0;
         SetupDiEnumDeviceInfo(device_info_set, i, &device_info_data); i++) {
        DWORD data_type, size;
        char buffer[512];
        if (SetupDiGetDeviceRegistryProperty(
                device_info_set, &device_info_data, SPDRP_DEVICEDESC,
                &data_type, (PBYTE)buffer, sizeof(buffer), &size)) {
            devices.push_back({buffer, ""});
        }
    }

    SetupDiDestroyDeviceInfoList(device_info_set);
    return devices;
}

std::vector<DeviceInfo> enumerate_bluetooth_devices() {
    std::vector<DeviceInfo> devices;
    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {
        sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS), 1, 0, 1, 1, 1, 15, NULL};

    BLUETOOTH_DEVICE_INFO deviceInfo;
    deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
    HBLUETOOTH_DEVICE_FIND btFind =
        BluetoothFindFirstDevice(&searchParams, &deviceInfo);

    if (btFind != NULL) {
        do {
            std::wstring ws(deviceInfo.szName);
            std::string name(ws.begin(), ws.end());
            char address[18];
            snprintf(
                address, sizeof(address), "%02X:%02X:%02X:%02X:%02X:%02X",
                deviceInfo.Address.rgBytes[5], deviceInfo.Address.rgBytes[4],
                deviceInfo.Address.rgBytes[3], deviceInfo.Address.rgBytes[2],
                deviceInfo.Address.rgBytes[1], deviceInfo.Address.rgBytes[0]);
            devices.push_back({name, address});
        } while (BluetoothFindNextDevice(btFind, &deviceInfo));
        BluetoothFindDeviceClose(btFind);
    }
    return devices;
}

#else  // Linux

std::vector<DeviceInfo> enumerate_usb_devices() {
    std::vector<DeviceInfo> devices;
    libusb_context *ctx = nullptr;
    libusb_device **devList = nullptr;
    int count;
    int ret;

    // Initialize libusb
    ret = libusb_init(&ctx);
    if (ret < 0) {
        LOG_F(ERROR, "Failed to initialize libusb: {}", libusb_error_name(ret));
        return devices;
    }

    // Get a list of all USB devices
    count = libusb_get_device_list(ctx, &devList);
    if (count < 0) {
        LOG_F(ERROR, "Failed to get device list: {}", libusb_error_name(count));
        libusb_exit(ctx);
        return devices;
    }

    for (int i = 0; i < count; i++) {
        libusb_device *dev = devList[i];
        libusb_device_descriptor desc{};
        libusb_config_descriptor *cfg;
        char buf[256];

        // Get the device descriptor
        ret = libusb_get_device_descriptor(dev, &desc);
        if (ret < 0) {
            LOG_F(ERROR, "Failed to get device descriptor: {}",
                  libusb_error_name(ret));
            continue;
        }

        // Get the first configuration descriptor
        ret = libusb_get_config_descriptor(dev, 0, &cfg);
        if (ret < 0) {
            LOG_F(ERROR, "Failed to get configuration descriptor: {}",
                  libusb_error_name(ret));
            continue;
        }

        // Get the device address
        std::string address =
            "Bus " + std::to_string(libusb_get_bus_number(dev)) + " Device " +
            std::to_string(libusb_get_device_address(dev));

        libusb_device_handle *handle;
        ret = libusb_open(dev, &handle);
        if (ret == 0 && (desc.iManufacturer != 0U)) {
            int len = libusb_get_string_descriptor_ascii(
                handle, desc.iManufacturer, (unsigned char *)buf, sizeof(buf));
            if (len > 0) {
                address += " (" + std::string(buf, len) + ")";
            }
            libusb_close(handle);
        }

        // Create a DeviceInfo struct and add it to the vector
        DeviceInfo deviceInfo;
        deviceInfo.description = address;
        deviceInfo.address = address;
        devices.push_back(deviceInfo);

        // Free the configuration descriptor
        libusb_free_config_descriptor(cfg);
    }

    // Free the device list
    libusb_free_device_list(devList, 1);

    // Clean up
    libusb_exit(ctx);

    return devices;
}

std::vector<DeviceInfo> enumerate_serial_ports() {
    std::vector<DeviceInfo> devices;
    struct dirent *entry;
    DIR *dp = opendir("/dev");

    if (dp == nullptr) {
        perror("opendir");
        return devices;
    }

    while ((entry = readdir(dp))) {
        std::string filename(entry->d_name);
        if (filename.find("ttyS") != std::string::npos ||
            filename.find("ttyUSB") != std::string::npos) {
            devices.push_back({filename, ""});
        }
    }

    closedir(dp);
    return devices;
}

std::vector<DeviceInfo> enumerate_bluetooth_devices() {
    std::vector<DeviceInfo> devices;
#if __has_include(<bluetooth/bluetooth.h>)
    int devId = hci_get_route(nullptr);
    if (devId < 0) {
        LOG_F(ERROR, "No Bluetooth adapter available: {}",
              std::strerror(errno));
        return devices;
    }

    int sock = hci_open_dev(devId);
    if (sock < 0) {
        LOG_F(ERROR, "Failed to open socket to Bluetooth adapter: {}",
              std::strerror(errno));
        return devices;
    }

    // RAII to manage the socket
    struct CloseSocket {
        void operator()(const int *ptr) const {
            if (ptr != nullptr) {
                auto sock = *ptr;
                close(sock);
                delete ptr;
            }
        }
    };

    auto sockGuard =
        std::unique_ptr<int, CloseSocket>(new int(sock), CloseSocket());

    int maxRsp = 255;
    int len = 8;
    int flags = IREQ_CACHE_FLUSH;
    std::array<char, 19> addr{};
    std::array<char, 248> name{};

    // Use unique_ptr with custom deleter for automatic memory management
    auto ii = std::unique_ptr<inquiry_info[]>(new inquiry_info[maxRsp]);
    inquiry_info *localIi = ii.get();
    int numRsp = hci_inquiry(devId, len, maxRsp, nullptr, &localIi, flags);
    if (numRsp < 0) {
        LOG_F(ERROR, "HCI inquiry failed: {}", std::strerror(errno));
        return devices;
    }

    for (int i = 0; i < numRsp; i++) {
        ba2str(&(ii.get()[i].bdaddr), addr.data());

        memset(name.data(), 0, name.size());
        if (hci_read_remote_name(*sockGuard, &(ii.get()[i].bdaddr), name.size(),
                                 name.data(), 0) < 0) {
            std::strcpy(name.data(), "[unknown]");
        }
        devices.push_back(
            DeviceInfo{std::string(name.data()), std::string(addr.data())});
    }
#endif
    return devices;
}

#endif

}  // namespace atom::system
