#ifndef ATOM_SYSINFO_SN_HPP
#define ATOM_SYSINFO_SN_HPP

#include <string>
#include <vector>

class HardwareInfo {
public:
    HardwareInfo();
    ~HardwareInfo();

    // Copy constructor
    HardwareInfo(const HardwareInfo& other);

    // Copy assignment operator
    HardwareInfo& operator=(const HardwareInfo& other);

    // Move constructor
    HardwareInfo(HardwareInfo&& other) noexcept;

    // Move assignment operator
    HardwareInfo& operator=(HardwareInfo&& other) noexcept;

    auto getBiosSerialNumber() -> std::string;
    auto getMotherboardSerialNumber() -> std::string;
    auto getCpuSerialNumber() -> std::string;
    auto getDiskSerialNumbers() -> std::vector<std::string>;

private:
    class Impl;   // Pimpl (Private Implementation) class
    Impl* impl_;  // Pointer to the implementation
};

#endif  // ATOM_SYSINFO_SN_HPP