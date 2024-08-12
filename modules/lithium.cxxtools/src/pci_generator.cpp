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
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

#include "macro.hpp"

struct Vendor {
    uint64_t pciId;
    size_t nameIndex;
    std::vector<size_t> devices;
} ATOM_ALIGNAS(64);

struct Device {
    uint64_t pciId;
    size_t nameIndex;
} ATOM_ALIGNAS(16);

void parseAndGeneratePCIInfo(std::string_view filename) {
    std::ifstream in(filename.data());
    if (!in.is_open()) {
        LOG_F(ERROR, "Couldn't open input file");
        THROW_FILE_NOT_READABLE("Couldn't open input file");
    }

    std::vector<Vendor> vendors;
    std::vector<Device> devices;
    std::vector<std::string> vendorDeviceNames;

    for (std::string line; std::getline(in, line);) {
        if (line.empty() || line[0] == 'C') {
            break;
        }

        const auto TABCOUNT = line.find_first_not_of('\t');
        if ((std::isxdigit(line[TABCOUNT]) == 0) || TABCOUNT >= 3) {
            continue;
        }

        if (*line.rbegin() == '\r') {
            line.erase(line.length() - 1);
        }

        char* currentName{};
        auto currentNumber =
            std::strtoull(line.c_str() + TABCOUNT, &currentName, 16);
        while (std::isspace(*currentName) != 0) {
            ++currentName;
        }

        if (TABCOUNT == 0) {
            vendors.emplace_back(
                Vendor{currentNumber, vendorDeviceNames.size(), {}});
        } else if (TABCOUNT == 1) {
            vendors.back().devices.push_back(devices.size());
            devices.emplace_back(
                Device{currentNumber, vendorDeviceNames.size()});
        }

        vendorDeviceNames.emplace_back(currentName);
    }

    std::ranges::sort(vendors, {}, &Vendor::pciId);

    auto& out = std::cout;
    out << std::hex << std::showbase;

    out << "#define ATOM_SYSTEM_GENERATED_PCI_INDICES\n";
    for (size_t idx = 0; const auto& vendor : vendors) {
        out << " \\\n\t{" << vendor.pciId << ", " << idx << "},";
        ++idx;
    }

    out << "\n\n\n#define ATOM_SYSTEM_GENERATED_PCI_VENDORS";
    for (const auto& vendor : vendors) {
        out << " \\\n\t{" << vendor.pciId << ", R\"("
            << vendorDeviceNames[vendor.nameIndex] << ")\", {";
        for (auto i : vendor.devices) {
            out << i << ", ";
        }
        out << "}},";
    }

    out << "\n\n\n#define ATOM_SYSTEM_GENERATED_PCI_DEVICES";
    for (const auto& device : devices) {
        out << " \\\n\t{" << device.pciId << ", R\"("
            << vendorDeviceNames[device.nameIndex] << ")\"},";
    }

    out << "\n\n\nnamespace {}\n";
}

auto main(int argc, const char** argv) -> int {
    if (argc < 2) {
        LOG_F(ERROR, "Input file missing");
        return 1;
    }

    try {
        parseAndGeneratePCIInfo(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
