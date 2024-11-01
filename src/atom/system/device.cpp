#include "device.hpp"

#include <array>
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
constexpr size_t BUFFER_SIZE = 512;
constexpr size_t ADDRESS_SIZE = 18;
constexpr int BLUETOOTH_SEARCH_TIMEOUT = 15;
constexpr int BYTE_5 = 5;
constexpr int BYTE_4 = 4;
constexpr int BYTE_3 = 3;
constexpr int BYTE_2 = 2;
constexpr int BYTE_1 = 1;
constexpr int BYTE_0 = 0;

auto enumerateUsbDevices() -> std::vector<DeviceInfo> {
    LOG_F(INFO, "enumerateUsbDevices called");
    std::vector<DeviceInfo> devices;
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(
        nullptr, "USB", nullptr, DIGCF_PRESENT | DIGCF_ALLCLASSES);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to get USB device info set");
        return devices;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (int i = 0;
         SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData) != 0; i++) {
        DWORD dataType;
        DWORD size;
        std::array<char, BUFFER_SIZE> buffer;
        if (SetupDiGetDeviceRegistryProperty(
                deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, &dataType,
                reinterpret_cast<PBYTE>(buffer.data()), buffer.size(), &size)) {
            devices.push_back({buffer.data(), ""});
            LOG_F(INFO, "Found USB device: {}", buffer.data());
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    LOG_F(INFO, "enumerateUsbDevices completed with {} devices found",
          devices.size());
    return devices;
}

auto enumerateSerialPorts() -> std::vector<DeviceInfo> {
    LOG_F(INFO, "enumerateSerialPorts called");
    std::vector<DeviceInfo> devices;
    HDEVINFO deviceInfoSet =
        SetupDiGetClassDevs(nullptr, "COM", nullptr, DIGCF_PRESENT);
    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to get serial port info set");
        return devices;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (int i = 0;
         SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData) != 0; i++) {
        DWORD dataType;
        DWORD size;
        std::array<char, BUFFER_SIZE> buffer;
        if (SetupDiGetDeviceRegistryProperty(
                deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC, &dataType,
                reinterpret_cast<PBYTE>(buffer.data()), buffer.size(), &size)) {
            devices.push_back({buffer.data(), ""});
            LOG_F(INFO, "Found serial port: {}", buffer.data());
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    LOG_F(INFO, "enumerateSerialPorts completed with {} devices found",
          devices.size());
    return devices;
}

auto enumerateBluetoothDevices() -> std::vector<DeviceInfo> {
    LOG_F(INFO, "enumerateBluetoothDevices called");
    std::vector<DeviceInfo> devices;
    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {
        sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
        1,
        0,
        1,
        1,
        1,
        BLUETOOTH_SEARCH_TIMEOUT,
        nullptr};

    BLUETOOTH_DEVICE_INFO deviceInfo;
    deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);
    HBLUETOOTH_DEVICE_FIND btFind =
        BluetoothFindFirstDevice(&searchParams, &deviceInfo);

    if (btFind != nullptr) {
        do {
            std::wstring wideName(deviceInfo.szName);
            std::string name(wideName.begin(), wideName.end());
            std::array<char, ADDRESS_SIZE> address;
            std::string formattedAddress =
                std::format("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}",
                            deviceInfo.Address.rgBytes[BYTE_5],
                            deviceInfo.Address.rgBytes[BYTE_4],
                            deviceInfo.Address.rgBytes[BYTE_3],
                            deviceInfo.Address.rgBytes[BYTE_2],
                            deviceInfo.Address.rgBytes[BYTE_1],
                            deviceInfo.Address.rgBytes[BYTE_0]);
            std::copy(formattedAddress.begin(), formattedAddress.end(),
                      address.begin());
            devices.push_back({name, address.data()});
            LOG_F(INFO, "Found Bluetooth device: {} - {}", name,
                  address.data());
        } while (BluetoothFindNextDevice(btFind, &deviceInfo) != 0);
        BluetoothFindDeviceClose(btFind);
    }
    LOG_F(INFO, "enumerateBluetoothDevices completed with {} devices found",
          devices.size());
    return devices;
}

#else  // Linux

std::vector<DeviceInfo> enumerate_usb_devices() {
    LOG_F(INFO, "enumerate_usb_devices called");
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
        LOG_F(INFO, "Found USB device: {}", address);

        // Free the configuration descriptor
        libusb_free_config_descriptor(cfg);
    }

    // Free the device list
    libusb_free_device_list(devList, 1);

    // Clean up
    libusb_exit(ctx);

    LOG_F(INFO, "enumerate_usb_devices completed with {} devices found",
          devices.size());
    return devices;
}

std::vector<DeviceInfo> enumerate_serial_ports() {
    LOG_F(INFO, "enumerate_serial_ports called");
    std::vector<DeviceInfo> devices;
    struct dirent *entry;
    DIR *dp = opendir("/dev");

    if (dp == nullptr) {
        LOG_F(ERROR, "Failed to open /dev directory");
        return devices;
    }

    while ((entry = readdir(dp))) {
        std::string filename(entry->d_name);
        if (filename.find("ttyS") != std::string::npos ||
            filename.find("ttyUSB") != std::string::npos) {
            devices.push_back({filename, ""});
            LOG_F(INFO, "Found serial port: {}", filename);
        }
    }

    closedir(dp);
    LOG_F(INFO, "enumerate_serial_ports completed with {} devices found",
          devices.size());
    return devices;
}

std::vector<DeviceInfo> enumerate_bluetooth_devices() {
    LOG_F(INFO, "enumerate_bluetooth_devices called");
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
        LOG_F(INFO, "Found Bluetooth device: {} - {}", name.data(),
              addr.data());
    }
#endif
    LOG_F(INFO, "enumerate_bluetooth_devices completed with {} devices found",
          devices.size());
    return devices;
}

#endif

}  // namespace atom::system
