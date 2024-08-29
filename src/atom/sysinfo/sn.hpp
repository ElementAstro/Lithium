#ifndef ATOM_SYSINFO_SN_HPP
#define ATOM_SYSINFO_SN_HPP

#include <string>
#include <vector>

class HardwareInfo {
public:
    HardwareInfo();
    ~HardwareInfo();

    auto GetBiosSerialNumber() -> std::string;
    auto GetMotherboardSerialNumber() -> std::string;
    auto GetCpuSerialNumber() -> std::string;
    auto GetDiskSerialNumbers() -> std::vector<std::string>;

private:
    class Impl;   // Pimpl (Private Implementation) class
    Impl* pImpl;  // Pointer to the implementation
};

#endif  // ATOM_SYSINFO_SN_HPP
