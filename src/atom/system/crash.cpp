/*
 * crash.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-4

Description: Crash Report

**************************************************/

#include "crash.hpp"

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#include "atom/error/stacktrace.hpp"
#include "atom/log/loguru.hpp"
#include "atom/sysinfo/cpu.hpp"
#include "atom/sysinfo/disk.hpp"
#include "atom/sysinfo/memory.hpp"
#include "atom/sysinfo/os.hpp"
#include "atom/utils/time.hpp"
#include "crash_quotes.hpp"
#include "env.hpp"
#include "platform.hpp"

namespace atom::system {
auto getSystemInfo() -> std::string {
    std::stringstream sss;

    auto osInfo = getOperatingSystemInfo();
    sss << "System Information:" << std::endl;
    sss << "- Operating system: " << osInfo.osName << " " << osInfo.osVersion
        << std::endl;
    sss << "- Architecture: " << osInfo.architecture << std::endl;
    sss << "- Kernel version: " << osInfo.kernelVersion << std::endl;
    sss << "- Computer name: " << osInfo.computerName << std::endl;
    sss << "- Compiler: " << osInfo.compiler << std::endl;
    sss << "- GUI: " << (hasGUI() ? "Yes" : "No") << std::endl;

    sss << "CPU:" << std::endl;
    sss << "- Usage: " << getCurrentCpuUsage() << "%" << std::endl;
    sss << "- Model: " << getCPUModel() << std::endl;
    sss << "- Frequency: " << getProcessorFrequency() << " GHz" << std::endl;
    sss << "- Temperature: " << getCurrentCpuTemperature() << " °C"
        << std::endl;
    sss << "- Core: " << getNumberOfPhysicalCPUs() << std::endl;
    sss << "- Package: " << getNumberOfPhysicalPackages() << std::endl;

    sss << "Current Memory Status:" << std::endl;
    sss << "- Usage: " << getMemoryUsage() << "%" << std::endl;
    sss << "- Total: " << getTotalMemorySize() << " MB" << std::endl;
    sss << "- Free: " << getAvailableMemorySize() << " MB" << std::endl;

    sss << "Disk:" << std::endl;
    sss << "- Usage: " << std::endl;
    for (const auto &[drive, usage] : getDiskUsage()) {
        sss << "- " << drive << ": " << usage << "%" << std::endl;
    }

    return sss.str();
}

void saveCrashLog(std::string_view error_msg) {
    std::string systemInfo = getSystemInfo();
    std::string environmentInfo;
    for (const auto &[key, value] : utils::Env::Environ()) {
        environmentInfo += key + ": " + value + "\n";
    }

    std::stringstream sss;
    sss << "Program crashed at: " << utils::getChinaTimestampString()
        << std::endl;
    sss << "Error messsage: " << error_msg << std::endl;

    sss << "==================== StackTrace ====================" << std::endl;
    atom::error::StackTrace stackTrace;
    sss << stackTrace.toString() << std::endl;
    sss << "==================== System Information ===================="
        << std::endl;
    sss << systemInfo << std::endl;
    sss << "================= Environment Variables Information "
           "=================="
        << std::endl;
    if (environmentInfo.empty()) {
        sss << "Failed to get environment information." << std::endl;
    } else {
        sss << environmentInfo << std::endl;
    }

    QuoteManager quotes;
    quotes.loadQuotesFromJson("./quotes.json");
    sss << "============ Famous saying: " << quotes.getRandomQuote()
        << " ============" << std::endl;

    std::stringstream ssss;
    std::time_t now = std::time(nullptr);
    ssss << "crash_report/crash_"
         << std::put_time(std::localtime(&now), "%Y%m%d_%H%M%S") << ".log";
    std::filesystem::path dirPath("crash_report");
    if (!std::filesystem::exists(dirPath)) {
        std::filesystem::create_directory(dirPath);
    }
    std::ofstream ofs(ssss.str());
    if (ofs.good()) {
        ofs << sss.str();
        ofs.close();
    }
}
}  // namespace atom::system
