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
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <dirent.h>
#include <fcntl.h>
#include <libusb.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#ifdef _WIN32

namespace atom::system
{
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
    libusb_context *ctx = NULL;
    libusb_device **devs = NULL;
    int i, ret;

    // Initialize libusb
    ctx = libusb_init(NULL);
    if (!ctx) {
        std::cerr << "Failed to initialize libusb" << std::endl;
        return devices;
    }

    // Get a list of all USB devices
    ret = libusb_get_device_list(ctx, &devs);
    if (ret < 0) {
        std::cerr << "Failed to get device list" << std::endl;
        libusb_free_device_list(devs, 1);
        libusb_exit(ctx);
        return devices;
    }

    // Iterate through the devices and extract information
    for (i = 0; i < ret; i++) {
        libusb_device *dev = devs[i];
        char buf[256];
        struct libusb_device_descriptor desc;
        struct libusb_config_descriptor *cfg;

        // Get the device descriptor
        ret = libusb_get_device_descriptor(dev, &desc);
        if (ret < 0) {
            std::cerr << "Failed to get device descriptor" << std::endl;
            continue;
        }

        // Get the device configuration
        cfg = libusb_get_config_descriptor(dev);
        if (!cfg) {
            std::cerr << "Failed to get device configuration" << std::endl;
            continue;
        }

        // Get the device address
        std::string address;
        address += "Bus ";
        address += std::to_string(libusb_get_bus_number(dev));
        address += " Device ";
        address += std::to_string(libusb_get_device_address(dev));

        // Get the device description
        if (desc.iManufacturer && desc.iManufacturer < 256) {
            libusb_get_string_descriptor_ascii(dev, desc.iManufacturer, buf,
                                               256);
            std::string manufacturer(buf);
            if (!manufacturer.empty()) {
                address += " (";
                address += manufacturer;
                address += ")";
            }
        }

        // Create a DeviceInfo struct and add it to the vector
        DeviceInfo device;
        device.description = address;
        device.address = address;
        devices.push_back(device);

        // Free the configuration descriptor
        libusb_free_config_descriptor(cfg);
    }

    // Free the device list
    libusb_free_device_list(devs, 1);

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
    int dev_id = hci_get_route(nullptr);
    int sock = hci_open_dev(dev_id);

    if (dev_id < 0 || sock < 0) {
        perror("opening socket");
        return devices;
    }

    inquiry_info *ii = nullptr;
    int max_rsp = 255;
    int num_rsp;
    int len = 8;
    int flags = IREQ_CACHE_FLUSH;
    char addr[19] = {0};
    char name[248] = {0};

    ii = (inquiry_info *)malloc(max_rsp * sizeof(inquiry_info));
    num_rsp = hci_inquiry(dev_id, len, max_rsp, nullptr, &ii, flags);
    if (num_rsp < 0)
        perror("hci_inquiry");

    for (int i = 0; i < num_rsp; i++) {
        ba2str(&(ii + i)->bdaddr, addr);
        if (hci_read_remote_name(sock, &(ii + i)->bdaddr, sizeof(name), name,
                                 0) < 0) {
            strcpy(name, "[unknown]");
        }
        devices.push_back({name, addr});
    }

    free(ii);
    close(sock);
    return devices;
}

#endif

}
