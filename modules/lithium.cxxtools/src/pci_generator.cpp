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
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "loguru.hpp"

namespace fs = std::filesystem;

// Alignas is retained if necessary for specific memory alignment requirements
struct Vendor {
    uint64_t pciId;
    size_t nameIndex;
    std::vector<size_t> devices;
} ATOM_ALIGNAS(64);

struct Device {
    uint64_t pciId;
    size_t nameIndex;
} ATOM_ALIGNAS(16);

class PCIInfoGenerator {
public:
    PCIInfoGenerator(std::string_view inputFilePath,
                     std::string_view outputFilePath)
        : inputPath_(inputFilePath), outputPath_(outputFilePath) {}

    void generate() {
        validateFiles();
        parseInputFile();
        sortVendors();
        writeOutputFile();
    }

private:
    fs::path inputPath_;
    fs::path outputPath_;
    std::vector<Vendor> vendors_;
    std::vector<Device> devices_;
    std::vector<std::string> vendorDeviceNames_;

    void validateFiles() const {
        if (!fs::exists(inputPath_) || !fs::is_regular_file(inputPath_)) {
            LOG_F(ERROR,
                  "Input file does not exist or is not a regular file: {}",
                  inputPath_.string());
            throw std::runtime_error(
                "Input file does not exist or is not a regular file");
        }

        LOG_F(INFO, "Input file validated: {}", inputPath_.string());

        // Check if output directory exists
        fs::path outputDir = outputPath_.parent_path();
        if (!outputDir.empty() && !fs::exists(outputDir)) {
            LOG_F(ERROR, "Output directory does not exist: {}",
                  outputDir.string());
            throw std::runtime_error("Output directory does not exist");
        }

        LOG_F(INFO, "Output directory validated: {}", outputDir.string());
    }

    void parseInputFile() {
        LOG_F(INFO, "Opening input file: {}", inputPath_.string());

        std::ifstream inFile(inputPath_, std::ios::in);
        if (!inFile.is_open()) {
            LOG_F(ERROR, "Failed to open input file: {}", inputPath_.string());
            throw std::runtime_error("Failed to open input file");
        }

        std::string line;
        size_t lineNumber = 0;

        while (std::getline(inFile, line)) {
            ++lineNumber;
            if (line.empty() || line.front() == 'C') {
                LOG_F(INFO, "Skipping line {}: Empty or starts with 'C'",
                      lineNumber);
                continue;
            }

            size_t firstNonTab = line.find_first_not_of('\t');
            if (firstNonTab == std::string::npos || firstNonTab >= 3) {
                LOG_F(INFO, "Skipping line {}: Invalid TAB count", lineNumber);
                continue;
            }

            // Remove potential carriage return at the end
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            const char* c_str = line.c_str() + firstNonTab;
            char* endPtr = nullptr;
            uint64_t pciId = std::strtoull(c_str, &endPtr, 16);

            if (c_str == endPtr) {
                LOG_F(WARNING, "Line {}: No valid PCI ID found", lineNumber);
                continue;
            }

            // Skip any whitespace after the PCI ID
            while (std::isspace(*endPtr)) {
                ++endPtr;
            }

            std::string_view name(endPtr);
            name = trim(name);

            if (firstNonTab == 0) {
                // Vendor line
                vendors_.emplace_back(
                    Vendor{pciId, vendorDeviceNames_.size(), {}});
                LOG_F(INFO, "Parsed Vendor: PCI ID=0x{:X}, NameIndex={}", pciId,
                      vendors_.back().nameIndex);
            } else if (firstNonTab == 1) {
                // Device line
                if (vendors_.empty()) {
                    LOG_F(WARNING, "Line {}: Device found before any vendor",
                          lineNumber);
                    continue;
                }
                vendors_.back().devices.emplace_back(devices_.size());
                devices_.emplace_back(Device{pciId, vendorDeviceNames_.size()});
                LOG_F(INFO, "Parsed Device: PCI ID=0x{:X}, NameIndex={}", pciId,
                      devices_.back().nameIndex);
            }

            vendorDeviceNames_.emplace_back(std::string(name));
            LOG_F(INFO, "Parsed Name: {}", vendorDeviceNames_.back());
        }

        inFile.close();
        LOG_F(INFO, "Completed parsing input file. Vendors: {}, Devices: {}",
              vendors_.size(), devices_.size());
    }

    void sortVendors() {
        LOG_F(INFO, "Sorting vendors by PCI ID");

        std::ranges::sort(vendors_, {}, &Vendor::pciId);

        LOG_F(INFO, "Vendors sorted");
    }

    void writeOutputFile() const {
        LOG_F(INFO, "Opening output file: {}", outputPath_.string());

        std::ofstream outFile(outputPath_, std::ios::out);
        if (!outFile.is_open()) {
            LOG_F(ERROR, "Failed to open output file: {}",
                  outputPath_.string());
            throw std::runtime_error("Failed to open output file");
        }

        outFile << std::hex << std::showbase;

        // Write PCI Indices
        outFile << "#define ATOM_SYSTEM_GENERATED_PCI_INDICES \\\n";
        for (size_t idx = 0; idx < vendors_.size(); ++idx) {
            outFile << "\t{" << vendors_[idx].pciId << ", " << idx << "},\\\n";
        }

        outFile << "\n\n#define ATOM_SYSTEM_GENERATED_PCI_VENDORS \\\n";
        for (const auto& vendor : vendors_) {
            outFile << "\t{" << vendor.pciId << ", R\"("
                    << vendorDeviceNames_[vendor.nameIndex] << ")\", {";
            for (const auto& deviceIdx : vendor.devices) {
                outFile << deviceIdx << ", ";
            }
            outFile << "}},\\\n";
        }

        outFile << "\n\n#define ATOM_SYSTEM_GENERATED_PCI_DEVICES \\\n";
        for (const auto& device : devices_) {
            outFile << "\t{" << device.pciId << ", R\"("
                    << vendorDeviceNames_[device.nameIndex] << ")\"},\\\n";
        }

        outFile << "\n\nnamespace {}\n";

        outFile.close();
        LOG_F(INFO, "Output file written successfully: {}",
              outputPath_.string());
    }

    // Helper function to trim whitespace from both ends
    std::string_view trim(std::string_view sv) const {
        auto start = sv.find_first_not_of(" \t\r\n");
        auto end = sv.find_last_not_of(" \t\r\n");
        if (start == std::string_view::npos)
            return "";
        return sv.substr(start, end - start + 1);
    }
};

int main(int argc, const char** argv) {
    LOG_F(INFO, "PCIInfoGenerator application started.");

    if (argc < 3) {
        LOG_F(
            ERROR,
            "Invalid number of arguments. Usage: {} <input_file> <output_file>",
            argv[0]);
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>"
                  << std::endl;
        return EXIT_FAILURE;
    }

    std::string_view inputFilePath = argv[1];
    std::string_view outputFilePath = argv[2];

    LOG_F(INFO, "Input File: {}", inputFilePath);
    LOG_F(INFO, "Output File: {}", outputFilePath);

    try {
        PCIInfoGenerator generator(inputFilePath, outputFilePath);
        generator.generate();
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception occurred: {}", e.what());
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        LOG_F(ERROR, "Unknown exception occurred.");
        std::cerr << "An unknown error occurred." << std::endl;
        return EXIT_FAILURE;
    }

    LOG_F(INFO, "PCIInfoGenerator application terminated successfully.");
    return EXIT_SUCCESS;
}