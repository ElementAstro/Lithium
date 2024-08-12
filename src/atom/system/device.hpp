#ifndef ATOM_SYSTEM_DEVICE_HPP
#define ATOM_SYSTEM_DEVICE_HPP

#include <string>
#include <vector>

#include "macro.hpp"

namespace atom::system {
struct DeviceInfo {
    std::string description;
    std::string address;
} ATOM_ALIGNAS(64);

auto enumerateUsbDevices() -> std::vector<DeviceInfo>;

auto enumerateSerialPorts() -> std::vector<DeviceInfo>;

auto enumerateBluetoothDevices() -> std::vector<DeviceInfo>;

}  // namespace atom::system

#endif
