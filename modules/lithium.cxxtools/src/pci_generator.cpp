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
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

#include "macro.hpp"

namespace fs = std::filesystem;

struct Vendor {
    uint64_t pciId;
    size_t nameIndex;
    std::vector<size_t> devices;
} ATOM_ALIGNAS(64);

struct Device {
    uint64_t pciId;
    size_t nameIndex;
} ATOM_ALIGNAS(16);

void parseAndGeneratePCIInfo(std::string_view inputFilePath,
                             std::string_view outputFilePath) {
    if (!fs::exists(inputFilePath) || !fs::is_regular_file(inputFilePath)) {
        LOG_F(ERROR, "Input file does not exist or is not a regular file: {}",
              inputFilePath);
        THROW_FILE_NOT_READABLE(
            "Input file does not exist or is not a regular file");
    }

    std::ifstream in(inputFilePath.data());
    if (!in.is_open()) {
        LOG_F(ERROR, "Couldn't open input file: {}", inputFilePath);
        THROW_FILE_NOT_READABLE("Couldn't open input file");
    }

    std::ofstream out(outputFilePath.data());
    if (!out.is_open()) {
        LOG_F(ERROR, "Couldn't open output file: {}", outputFilePath);
        THROW_FILE_NOT_WRITABLE("Couldn't open output file");
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
    if (argc < 3) {
        LOG_F(ERROR, "Usage: {} <input_file> <output_file>", argv[0]);
        return 1;
    }

    try {
        parseAndGeneratePCIInfo(argv[1], argv[2]);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
