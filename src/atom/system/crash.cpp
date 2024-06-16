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
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/error/stacktrace.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/sysinfo/cpu.hpp"
#include "atom/sysinfo/disk.hpp"
#include "atom/sysinfo/memory.hpp"
#include "atom/sysinfo/os.hpp"
#include "atom/utils/string.hpp"
#include "atom/utils/time.hpp"
#include "crash_quotes.hpp"
#include "env.hpp"
#include "platform.hpp"

namespace atom::system {
// 获取系统信息
std::string getSystemInfo() {
    std::stringstream ss;

    auto osInfo = getOperatingSystemInfo();
    ss << "System Information:" << std::endl;
    ss << "- Operating system: " << osInfo.osName << " " << osInfo.osVersion
       << std::endl;
    ss << "- Architecture: " << osInfo.architecture << std::endl;
    ss << "- Kernel version: " << osInfo.kernelVersion << std::endl;
    ss << "- Computer name: " << osInfo.computerName << std::endl;
    ss << "- Compiler: " << osInfo.compiler << std::endl;
    ss << "- GUI: " << (hasGUI() ? "Yes" : "No") << std::endl;

    ss << "CPU:" << std::endl;
    ss << "- Usage: " << getCurrentCpuUsage() << "%" << std::endl;
    ss << "- Model: " << getCPUModel() << std::endl;
    ss << "- Frequency: " << getProcessorFrequency() << " GHz" << std::endl;
    ss << "- Temperature: " << getCurrentCpuTemperature() << " °C" << std::endl;
    ss << "- Core: " << getNumberOfPhysicalCPUs() << std::endl;
    ss << "- Package: " << getNumberOfPhysicalPackages() << std::endl;

    ss << "Current Memory Status:" << std::endl;
    ss << "- Usage: " << getMemoryUsage() << "%" << std::endl;
    ss << "- Total: " << getTotalMemorySize() << " MB" << std::endl;
    ss << "- Free: " << getAvailableMemorySize() << " MB" << std::endl;

    ss << "Disk:" << std::endl;
    ss << "- Usage: " << std::endl;
    for (const auto &[drive, usage] : getDiskUsage()) {
        ss << "- " << drive << ": " << usage << "%" << std::endl;
    }

    return ss.str();
}

void saveCrashLog(const std::string &error_msg) {
    std::string system_info = getSystemInfo();
    std::string environment_info;
    for (const auto &[key, value] : utils::Env::Environ()) {
        environment_info += key + ": " + value + "\n";
    }

    std::stringstream ss;
    ss << "Program crashed at: " << utils::getChinaTimestampString()
       << std::endl;
    ss << "Error message: " << error_msg << std::endl;

    ss << "==================== StackTrace ====================" << std::endl;
    atom::error::StackTrace stack_trace;
    ss << stack_trace.toString() << std::endl;
    ss << "==================== System Information ===================="
       << std::endl;
    ss << system_info << std::endl;
    ss << "================= Environment Variables Information "
          "=================="
       << std::endl;
    if (environment_info.empty()) {
        ss << "Failed to get environment information." << std::endl;
    } else {
        ss << environment_info << std::endl;
    }

    QuoteManager quotes;

    ss << "============ Famous saying: " << quotes.getRandomQuote()
       << " ============" << std::endl;

    std::stringstream sss;
    std::time_t now = std::time(nullptr);
    sss << "crash_report/crash_"
        << std::put_time(std::localtime(&now), "%Y%m%d_%H%M%S") << ".log";
    std::filesystem::path dir_path("crash_report");
    if (!std::filesystem::exists(dir_path)) {
        std::filesystem::create_directory(dir_path);
    }
    std::ofstream ofs(sss.str());
    if (ofs.good()) {
        ofs << ss.str();
        ofs.close();
    }
}
}  // namespace atom::system
