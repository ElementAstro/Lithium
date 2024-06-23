#ifndef ATOM_SYSTEM_DEVICE_HPP
#define ATOM_SYSTEM_DEVICE_HPP

#include <string>
#include <vector>

namespace atom::system {
struct DeviceInfo {
    std::string description;
    std::string address;
};

std::vector<DeviceInfo> enumerate_usb_devices();

std::vector<DeviceInfo> enumerate_serial_ports();

std::vector<DeviceInfo> enumerate_bluetooth_devices();

}  // namespace atom::system

#endif
