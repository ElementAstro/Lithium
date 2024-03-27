/*
 * pci_generator.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-10

Description: PCI info generator

*************************************************/

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct Vendor {
    uint64_t pciId;
    size_t nameIndex;
    std::vector<size_t> devices;
};

struct Device {
    uint64_t pciId;
    size_t nameIndex;
};

void parseAndGeneratePCIInfo(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Couldn't open input file\n";
        return;
    }

    std::vector<Vendor> vendors;
    std::vector<Device> devices;
    std::vector<std::string> vendorDeviceNames;

    for (std::string line; std::getline(in, line);) {
        if (line.empty())
            continue;
        if (line[0] == 'C')  // Got to device classes. which we don't want
            break;

        const auto tabcount = line.find_first_not_of('\t');
        if (!std::isxdigit(line[tabcount]) || tabcount >= 3)
            continue;

        if (*line.rbegin() ==
            '\r')  // Remove carriage return if present for CRLF encoded files.
            line.erase(line.length() - 1);

        char* current_name{};
        auto current_number =
            std::strtoull(line.c_str() + tabcount, &current_name, 16);
        while (std::isspace(*current_name))
            ++current_name;

        if (tabcount == 0)  // Vendor
            vendors.push_back({current_number, vendorDeviceNames.size(), {}});
        else if (tabcount == 1) {  // Device
            vendors.back().devices.push_back(devices.size());
            devices.push_back({current_number, vendorDeviceNames.size()});
        }

        vendorDeviceNames.emplace_back(current_name);
    }

    // Sorting vendors by pciId
    std::sort(
        vendors.begin(), vendors.end(),
        [](const Vendor& a, const Vendor& b) { return a.pciId < b.pciId; });

    auto& out = std::cout;
    out << std::hex << std::showbase;

    // Output generated PCI indices
    out << "#define ATOM_SYSTEM_GENERATED_PCI_INDICES\n";
    for (size_t idx = 0; const auto& vendor : vendors) {
        out << " \\\n\t{" << vendor.pciId << ", " << idx << "},";
        ++idx;
    }

    // Output generated PCI vendors
    out << "\n\n\n#define ATOM_SYSTEM_GENERATED_PCI_VENDORS";
    for (const auto& vendor : vendors) {
        out << " \\\n\t{" << vendor.pciId << ", R\"("
            << vendorDeviceNames[vendor.nameIndex] << ")\", {";
        for (auto i : vendor.devices)
            out << i << ", ";
        out << "}},";
    }

    // Output generated PCI devices
    out << "\n\n\n#define ATOM_SYSTEM_GENERATED_PCI_DEVICES";
    for (const auto& device : devices)
        out << " \\\n\t{" << device.pciId << ", R\"("
            << vendorDeviceNames[device.nameIndex] << ")\"},";

    out << "\n\n\nnamespace {}\n";
}

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Input file missing\n";
        return 1;
    }

    parseAndGeneratePCIInfo(argv[1]);

    return 0;
}
