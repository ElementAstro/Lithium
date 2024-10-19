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

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on
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
    LOG_F(INFO, "getSystemInfo called");
    std::stringstream sss;

    auto osInfo = getOperatingSystemInfo();
    sss << "System Information:\n";
    sss << "-------------------\n";
    sss << std::format("Operating system: {} {}\n", osInfo.osName, osInfo.osVersion);
    sss << std::format("Architecture: {}\n", osInfo.architecture);
    sss << std::format("Kernel version: {}\n", osInfo.kernelVersion);
    sss << std::format("Computer name: {}\n", osInfo.computerName);
    sss << std::format("Compiler: {}\n", osInfo.compiler);
    sss << std::format("GUI: {}\n\n", ATOM_HAS_GUI() ? "Yes" : "No");

    sss << "CPU Information:\n";
    sss << "----------------\n";
    sss << std::format("Usage: {}%\n", getCurrentCpuUsage());
    sss << std::format("Model: {}\n", getCPUModel());
    sss << std::format("Frequency: {} GHz\n", getProcessorFrequency());
    sss << std::format("Temperature: {} °C\n", getCurrentCpuTemperature());
    sss << std::format("Cores: {}\n", getNumberOfPhysicalCPUs());
    sss << std::format("Packages: {}\n\n", getNumberOfPhysicalPackages());

    sss << "Memory Status:\n";
    sss << "--------------\n";
    sss << std::format("Usage: {}%\n", getMemoryUsage());
    sss << std::format("Total: {} MB\n", getTotalMemorySize());
    sss << std::format("Free: {} MB\n\n", getAvailableMemorySize());

    sss << "Disk Usage:\n";
    sss << "-----------\n";
    for (const auto &[drive, usage] : getDiskUsage()) {
        sss << std::format("{}: {}%\n", drive, usage);
    }

    LOG_F(INFO, "getSystemInfo completed");
    return sss.str();
}

void saveCrashLog(std::string_view error_msg) {
    LOG_F(INFO, "saveCrashLog called with error_msg: {}", error_msg);
    std::string systemInfo = getSystemInfo();
    std::string environmentInfo;
    for (const auto &[key, value] : utils::Env::Environ()) {
        environmentInfo += std::format("{}: {}\n", key, value);
    }

    std::stringstream sss;
    sss << std::format("Program crashed at: {}\n", utils::getChinaTimestampString());
    sss << std::format("Error message: {}\n\n", error_msg);

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
    sss << std::format("============ Famous Saying: {} ============\n", quotes.getRandomQuote());

    std::stringstream ssss;
    auto now = std::chrono::system_clock::now();
    std::time_t nowC = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;
    if (localtime_s(&localTime, &nowC) != 0) {
        LOG_F(ERROR, "Failed to get local time.");
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
        LOG_F(INFO, "Crash log saved to {}", ssss.str());
    } else {
        LOG_F(ERROR, "Failed to save crash log to {}", ssss.str());
    }

    // Create a dump file
#ifdef _WIN32
    std::stringstream wss;
    wss << "crash_report/crash_" << std::put_time(&localTime, "%Y%m%d_%H%M%S")
        << ".dmp";
    std::string dumpFile = wss.str();
    HANDLE hFile =
        CreateFile(dumpFile.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to create dump file {}", dumpFile);
        return;
    }
    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    EXCEPTION_POINTERS *pep = nullptr;
    mdei.ExceptionPointers = pep;
    mdei.ClientPointers = FALSE;
    if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile,
                          MiniDumpNormal, (pep != nullptr) ? &mdei : nullptr,
                          nullptr, nullptr) != 0) {
        LOG_F(INFO, "Dump file created at {}", dumpFile);
    } else {
        LOG_F(ERROR, "Failed to write dump file {}", dumpFile);
    }
    CloseHandle(hFile);
#endif
    LOG_F(INFO, "saveCrashLog completed");
}

}  // namespace atom::system
