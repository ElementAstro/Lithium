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

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

#ifndef _MSC_VER
#include <unistd.h>
#endif

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
    sss << "System Information:\n";
    sss << "-------------------\n";
    sss << "Operating system: " << osInfo.osName << " " << osInfo.osVersion
        << "\n";
    sss << "Architecture: " << osInfo.architecture << "\n";
    sss << "Kernel version: " << osInfo.kernelVersion << "\n";
    sss << "Computer name: " << osInfo.computerName << "\n";
    sss << "Compiler: " << osInfo.compiler << "\n";
    sss << "GUI: " << (ATOM_HAS_GUI() ? "Yes" : "No") << "\n\n";

    sss << "CPU Information:\n";
    sss << "----------------\n";
    sss << "Usage: " << getCurrentCpuUsage() << "%\n";
    sss << "Model: " << getCPUModel() << "\n";
    sss << "Frequency: " << getProcessorFrequency() << " GHz\n";
    sss << "Temperature: " << getCurrentCpuTemperature() << " °C\n";
    sss << "Cores: " << getNumberOfPhysicalCPUs() << "\n";
    sss << "Packages: " << getNumberOfPhysicalPackages() << "\n\n";

    sss << "Memory Status:\n";
    sss << "--------------\n";
    sss << "Usage: " << getMemoryUsage() << "%\n";
    sss << "Total: " << getTotalMemorySize() << " MB\n";
    sss << "Free: " << getAvailableMemorySize() << " MB\n\n";

    sss << "Disk Usage:\n";
    sss << "-----------\n";
    for (const auto &[drive, usage] : getDiskUsage()) {
        sss << drive << ": " << usage << "%\n";
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
    sss << "Program crashed at: " << utils::getChinaTimestampString() << "\n";
    sss << "Error message: " << error_msg << "\n\n";

    sss << "==================== Stack Trace ====================\n";
    atom::error::StackTrace stackTrace;
    sss << stackTrace.toString() << "\n\n";

    sss << "==================== System Information ====================\n";
    sss << systemInfo << "\n";

    sss << "================= Environment Variables ===================\n";
    if (environmentInfo.empty()) {
        sss << "Failed to get environment information.\n";
    } else {
        sss << environmentInfo << "\n";
    }

    QuoteManager quotes;
    quotes.loadQuotesFromJson("./quotes.json");
    sss << "============ Famous Saying: " << quotes.getRandomQuote()
        << " ============\n";

    std::stringstream ssss;
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    if (localtime_s(&localTime, &now) != 0) {
        // Handle error
        THROW_RUNTIME_ERROR("Failed to get local time.");
    }
    ssss << "crash_report/crash_" << std::put_time(&localTime, "%Y%m%d_%H%M%S")
         << ".log";
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