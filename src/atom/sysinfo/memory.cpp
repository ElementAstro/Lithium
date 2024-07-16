/*
 * memory.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - Memory

**************************************************/

#include "atom/sysinfo/memory.hpp"

#include <fstream>
#include <sstream>

#include "atom/log/loguru.hpp"

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <psapi.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <pdh.h>
#include <tlhelp32.h>
#include <wincon.h>
// clang-format on
#elif __linux__
#include <dirent.h>
#include <limits.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <csignal>
#include <iterator>
#elif __APPLE__
#include <mach/mach_init.h>
#include <mach/task_info.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/mount.h>
#include <sys/param.h>
#endif

namespace atom::system {
float getMemoryUsage() {
    float memoryUsage = 0.0;

#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    float total_memory = 0.0f;
    float available_memory = 0.0f;
    if (GlobalMemoryStatusEx(&status)) {
        total_memory = static_cast<float>(status.ullTotalPhys / 1024 / 1024);
        available_memory =
            static_cast<float>(status.ullAvailPhys / 1024 / 1024);
        memory_usage = (total_memory - available_memory) / total_memory * 100.0;
    } else {
        LOG_F(ERROR, "GetMemoryUsage error: GlobalMemoryStatusEx error");
    }
#elif __linux__
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        LOG_F(ERROR, "GetMemoryUsage error: open /proc/meminfo error");
    }
    std::string line;

    unsigned long totalMemory = 0;
    unsigned long freeMemory = 0;
    unsigned long bufferMemory = 0;
    unsigned long cacheMemory = 0;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string name;
        unsigned long value;

        if (iss >> name >> value) {
            if (name == "MemTotal:") {
                totalMemory = value;
            } else if (name == "MemFree:") {
                freeMemory = value;
            } else if (name == "Buffers:") {
                bufferMemory = value;
            } else if (name == "Cached:") {
                cacheMemory = value;
            }
        }
    }

    unsigned long usedMemory =
        totalMemory - freeMemory - bufferMemory - cacheMemory;
    memoryUsage = static_cast<float>(usedMemory) / totalMemory * 100.0;
#elif __APPLE__
    struct statfs stats;
    statfs("/", &stats);

    unsigned long long total_space = stats.f_blocks * stats.f_bsize;
    unsigned long long free_space = stats.f_bfree * stats.f_bsize;

    unsigned long long used_space = total_space - free_space;
    memory_usage = static_cast<float>(used_space) / total_space * 100.0;
#elif defined(__ANDROID__)
    LOG_F(ERROR, "GetTotalMemorySize error: not support");
#endif

    return memoryUsage;
}

unsigned long long getTotalMemorySize() {
    unsigned long long totalMemorySize = 0;

#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    totalMemorySize = status.ullTotalPhys;
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n hw.memsize", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            totalMemorySize = std::stoull(buffer);
        } else {
            LOG_F(ERROR, "GetTotalMemorySize error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    long pages = sysconf(_SC_PHYS_PAGES);
    long pageSize = sysconf(_SC_PAGE_SIZE);
    totalMemorySize = static_cast<unsigned long long>(pages) *
                      static_cast<unsigned long long>(pageSize);
#endif

    return totalMemorySize;
}

unsigned long long getAvailableMemorySize() {
    unsigned long long availableMemorySize = 0;

#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    availableMemorySize = status.ullAvailPhys;
#elif defined(__APPLE__)
    FILE *pipe = popen("vm_stat | grep 'Pages free:' | awk '{print $3}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            availableMemorySize = std::stoull(buffer) * getpagesize();
        } else {
            LOG_F(ERROR, "GetAvailableMemorySize error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) {
        LOG_F(ERROR, "GetAvailableMemorySize error: open /proc/meminfo error");
        return 1;  // Return error code
    }

    std::string line;
    bool found = false;

    // Read the file line by line
    while (std::getline(meminfo, line)) {
        if (line.substr(0, 13) == "MemAvailable:") {
            unsigned long long availableMemory;
            // Parse the line
            if (std::sscanf(line.c_str(), "MemAvailable: %llu kB",
                            &availableMemory) == 1) {
                availableMemorySize =
                    availableMemory * 1024;  // Convert from kB to bytes
                found = true;
                break;
            } else {
                LOG_F(ERROR, "GetAvailableMemorySize error: parse error");
                return -1;
            }
        }
    }

    meminfo.close();

    if (!found) {
        LOG_F(ERROR,
              "GetAvailableMemorySize error: MemAvailable entry not found in "
              "/proc/meminfo");
        return -1;  // Return error code
    }
#endif
    return availableMemorySize;
}

MemoryInfo::MemorySlot getPhysicalMemoryInfo() {
    MemoryInfo::MemorySlot slot;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);

    slot.capacity = std::to_string(memoryStatus.ullTotalPhys /
                                   (1024 * 1024));  // Convert bytes to MB
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl hw.memsize | awk '{print $2}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            slot.capacity = std::string(buffer) / (1024 * 1024);
        } else {
            LOG_F(ERROR, "GetPhysicalMemoryInfo error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    std::ifstream meminfo("/proc/meminfo");
    std::string line;
    while (std::getline(meminfo, line)) {
        if (line.substr(0, 10) == "MemTotal: ") {
            std::istringstream iss(line);
            std::vector<std::string> tokens{
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>{}};
            slot.capacity = tokens[1];
            break;
        }
    }
#endif

    return slot;
}

unsigned long long getVirtualMemoryMax() {
    unsigned long long virtualMemoryMax;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);
    virtualMemoryMax = memoryStatus.ullTotalVirtual / (1024 * 1024);
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl vm.swapusage | awk '{print $2}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            virtualMemoryMax = std::stoull(buffer) / (1024 * 1024);
        } else {
            LOG_F(ERROR, "GetVirtualMemoryMax error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    struct sysinfo si {};
    sysinfo(&si);
    virtualMemoryMax = (si.totalram + si.totalswap) / 1024;
#endif

    return virtualMemoryMax;
}

unsigned long long getVirtualMemoryUsed() {
    unsigned long long virtualMemoryUsed;

#ifdef _WIN32
    // Windows 实现
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);
    virtualMemoryUsed =
        (memoryStatus.ullTotalVirtual - memoryStatus.ullAvailVirtual) /
        (1024 * 1024);
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl vm.swapusage | awk '{print $6}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            virtualMemoryUsed = std::stoull(buffer) / (1024 * 1024);
        } else {
            LOG_F(ERROR, "GetVirtualMemoryUsed error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    struct sysinfo si {};
    sysinfo(&si);
    virtualMemoryUsed =
        (si.totalram - si.freeram + si.totalswap - si.freeswap) / 1024;
#endif

    return virtualMemoryUsed;
}

unsigned long long getSwapMemoryTotal() {
    unsigned long long swapMemoryTotal = 0;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);
    swapMemoryTotal = memoryStatus.ullTotalPageFile / (1024 * 1024);
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl vm.swapusage | awk '{print $2}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            swapMemoryTotal = std::stoull(buffer) / (1024 * 1024);
        } else {
            LOG_F(ERROR, "GetSwapMemoryTotal error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    struct sysinfo si {};
    sysinfo(&si);
    swapMemoryTotal = si.totalswap / 1024;
#endif

    return swapMemoryTotal;
}

unsigned long long getSwapMemoryUsed() {
    unsigned long long swapMemoryUsed = 0;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);
    swapMemoryUsed =
        (memoryStatus.ullTotalPageFile - memoryStatus.ullAvailPageFile) /
        (1024 * 1024);
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl vm.swapusage | awk '{print $6}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            swapMemoryUsed = std::stoull(buffer) / (1024 * 1024);
        } else {
            LOG_F(ERROR, "GetSwapMemoryUsed error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    struct sysinfo si {};
    sysinfo(&si);
    swapMemoryUsed = (si.totalswap - si.freeswap) / 1024;
#endif

    return swapMemoryUsed;
}

}  // namespace atom::system
