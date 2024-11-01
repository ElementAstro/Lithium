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
auto getMemoryUsage() -> float {
    LOG_F(INFO, "Starting getMemoryUsage function");
    float memoryUsage = 0.0;

#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    float totalMemory = 0.0f;
    float availableMemory = 0.0f;
    if (GlobalMemoryStatusEx(&status) != 0) {
        totalMemory = static_cast<float>(status.ullTotalPhys / 1024 / 1024);
        availableMemory = static_cast<float>(status.ullAvailPhys / 1024 / 1024);
        memoryUsage = (totalMemory - availableMemory) / totalMemory * 100.0;
        LOG_F(INFO,
              "Total Memory: %.2f MB, Available Memory: %.2f MB, Memory Usage: "
              "%.2f%%",
              totalMemory, availableMemory, memoryUsage);
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
    LOG_F(INFO,
          "Total Memory: {} kB, Free Memory: {} kB, Buffer Memory: {} kB, "
          "Cache Memory: {} kB, Memory Usage: {}",
          totalMemory, freeMemory, bufferMemory, cacheMemory, memoryUsage);
#elif __APPLE__
    struct statfs stats;
    statfs("/", &stats);

    unsigned long long total_space = stats.f_blocks * stats.f_bsize;
    unsigned long long free_space = stats.f_bfree * stats.f_bsize;

    unsigned long long used_space = total_space - free_space;
    memory_usage = static_cast<float>(used_space) / total_space * 100.0;
    LOG_F(INFO,
          "Total Space: {} bytes, Free Space: {} bytes, Used Space: {} "
          "bytes, Memory Usage: %.2f%%",
          total_space, free_space, used_space, memory_usage);
#elif defined(__ANDROID__)
    LOG_F(ERROR, "GetTotalMemorySize error: not support");
#endif

    LOG_F(INFO, "Finished getMemoryUsage function");
    return memoryUsage;
}

auto getTotalMemorySize() -> unsigned long long {
    LOG_F(INFO, "Starting getTotalMemorySize function");
    unsigned long long totalMemorySize = 0;

#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        totalMemorySize = status.ullTotalPhys;
        LOG_F(INFO, "Total Memory Size: {} bytes", totalMemorySize);
    } else {
        LOG_F(ERROR, "GetTotalMemorySize error: GlobalMemoryStatusEx error");
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n hw.memsize", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            totalMemorySize = std::stoull(buffer);
            LOG_F(INFO, "Total Memory Size: {} bytes", totalMemorySize);
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
    LOG_F(INFO, "Total Memory Size: {} bytes", totalMemorySize);
#endif

    LOG_F(INFO, "Finished getTotalMemorySize function");
    return totalMemorySize;
}

auto getAvailableMemorySize() -> unsigned long long {
    LOG_F(INFO, "Starting getAvailableMemorySize function");
    unsigned long long availableMemorySize = 0;

#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        availableMemorySize = status.ullAvailPhys;
        LOG_F(INFO, "Available Memory Size: {} bytes", availableMemorySize);
    } else {
        LOG_F(ERROR,
              "GetAvailableMemorySize error: GlobalMemoryStatusEx error");
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("vm_stat | grep 'Pages free:' | awk '{print $3}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            availableMemorySize = std::stoull(buffer) * getpagesize();
            LOG_F(INFO, "Available Memory Size: {} bytes", availableMemorySize);
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
            if (std::sscanf(line, "MemAvailable: {} kB", &availableMemory) ==
                1) {
                availableMemorySize =
                    availableMemory * 1024;  // Convert from kB to bytes
                found = true;
                LOG_F(INFO, "Available Memory Size: {} bytes",
                      availableMemorySize);
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
    LOG_F(INFO, "Finished getAvailableMemorySize function");
    return availableMemorySize;
}

auto getPhysicalMemoryInfo() -> MemoryInfo::MemorySlot {
    LOG_F(INFO, "Starting getPhysicalMemoryInfo function");
    MemoryInfo::MemorySlot slot;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    if (GlobalMemoryStatusEx(&memoryStatus)) {
        slot.capacity = std::to_string(memoryStatus.ullTotalPhys /
                                       (1024 * 1024));  // Convert bytes to MB
        LOG_F(INFO, "Physical Memory Capacity: {} MB", slot.capacity);
    } else {
        LOG_F(ERROR, "GetPhysicalMemoryInfo error: GlobalMemoryStatusEx error");
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl hw.memsize | awk '{print $2}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            slot.capacity = std::string(buffer) / (1024 * 1024);
            LOG_F(INFO, "Physical Memory Capacity: {} MB", slot.capacity);
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
            LOG_F(INFO, "Physical Memory Capacity: {} kB", slot.capacity);
            break;
        }
    }
#endif

    LOG_F(INFO, "Finished getPhysicalMemoryInfo function");
    return slot;
}

auto getVirtualMemoryMax() -> unsigned long long {
    LOG_F(INFO, "Starting getVirtualMemoryMax function");
    unsigned long long virtualMemoryMax = 0;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    if (GlobalMemoryStatusEx(&memoryStatus)) {
        virtualMemoryMax = memoryStatus.ullTotalVirtual / (1024 * 1024);
        LOG_F(INFO, "Virtual Memory Max: {} MB", virtualMemoryMax);
    } else {
        LOG_F(ERROR, "GetVirtualMemoryMax error: GlobalMemoryStatusEx error");
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl vm.swapusage | awk '{print $2}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            virtualMemoryMax = std::stoull(buffer) / (1024 * 1024);
            LOG_F(INFO, "Virtual Memory Max: {} MB", virtualMemoryMax);
        } else {
            LOG_F(ERROR, "GetVirtualMemoryMax error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    struct sysinfo si {};
    if (sysinfo(&si) == 0) {
        virtualMemoryMax = (si.totalram + si.totalswap) / 1024;
        LOG_F(INFO, "Virtual Memory Max: {} kB", virtualMemoryMax);
    } else {
        LOG_F(ERROR, "GetVirtualMemoryMax error: sysinfo error");
    }
#endif

    LOG_F(INFO, "Finished getVirtualMemoryMax function");
    return virtualMemoryMax;
}

auto getVirtualMemoryUsed() -> unsigned long long {
    LOG_F(INFO, "Starting getVirtualMemoryUsed function");
    unsigned long long virtualMemoryUsed = 0;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    if (GlobalMemoryStatusEx(&memoryStatus)) {
        virtualMemoryUsed =
            (memoryStatus.ullTotalVirtual - memoryStatus.ullAvailVirtual) /
            (1024 * 1024);
        LOG_F(INFO, "Virtual Memory Used: {} MB", virtualMemoryUsed);
    } else {
        LOG_F(ERROR, "GetVirtualMemoryUsed error: GlobalMemoryStatusEx error");
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl vm.swapusage | awk '{print $6}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            virtualMemoryUsed = std::stoull(buffer) / (1024 * 1024);
            LOG_F(INFO, "Virtual Memory Used: {} MB", virtualMemoryUsed);
        } else {
            LOG_F(ERROR, "GetVirtualMemoryUsed error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    struct sysinfo si {};
    if (sysinfo(&si) == 0) {
        virtualMemoryUsed =
            (si.totalram - si.freeram + si.totalswap - si.freeswap) / 1024;
        LOG_F(INFO, "Virtual Memory Used: {} kB", virtualMemoryUsed);
    } else {
        LOG_F(ERROR, "GetVirtualMemoryUsed error: sysinfo error");
    }
#endif

    LOG_F(INFO, "Finished getVirtualMemoryUsed function");
    return virtualMemoryUsed;
}

auto getSwapMemoryTotal() -> unsigned long long {
    LOG_F(INFO, "Starting getSwapMemoryTotal function");
    unsigned long long swapMemoryTotal = 0;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    if (GlobalMemoryStatusEx(&memoryStatus)) {
        swapMemoryTotal = memoryStatus.ullTotalPageFile / (1024 * 1024);
        LOG_F(INFO, "Swap Memory Total: {} MB", swapMemoryTotal);
    } else {
        LOG_F(ERROR, "GetSwapMemoryTotal error: GlobalMemoryStatusEx error");
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl vm.swapusage | awk '{print $2}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            swapMemoryTotal = std::stoull(buffer) / (1024 * 1024);
            LOG_F(INFO, "Swap Memory Total: {} MB", swapMemoryTotal);
        } else {
            LOG_F(ERROR, "GetSwapMemoryTotal error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    struct sysinfo si {};
    if (sysinfo(&si) == 0) {
        swapMemoryTotal = si.totalswap / 1024;
        LOG_F(INFO, "Swap Memory Total: {} kB", swapMemoryTotal);
    } else {
        LOG_F(ERROR, "GetSwapMemoryTotal error: sysinfo error");
    }
#endif

    LOG_F(INFO, "Finished getSwapMemoryTotal function");
    return swapMemoryTotal;
}

unsigned long long getSwapMemoryUsed() {
    LOG_F(INFO, "Starting getSwapMemoryUsed function");
    unsigned long long swapMemoryUsed = 0;

#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);
    if (GlobalMemoryStatusEx(&memoryStatus)) {
        swapMemoryUsed =
            (memoryStatus.ullTotalPageFile - memoryStatus.ullAvailPageFile) /
            (1024 * 1024);
        LOG_F(INFO, "Swap Memory Used: {} MB", swapMemoryUsed);
    } else {
        LOG_F(ERROR, "GetSwapMemoryUsed error: GlobalMemoryStatusEx error");
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl vm.swapusage | awk '{print $6}'", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            swapMemoryUsed = std::stoull(buffer) / (1024 * 1024);
            LOG_F(INFO, "Swap Memory Used: {} MB", swapMemoryUsed);
        } else {
            LOG_F(ERROR, "GetSwapMemoryUsed error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    struct sysinfo si {};
    if (sysinfo(&si) == 0) {
        swapMemoryUsed = (si.totalswap - si.freeswap) / 1024;
        LOG_F(INFO, "Swap Memory Used: {} kB", swapMemoryUsed);
    } else {
        LOG_F(ERROR, "GetSwapMemoryUsed error: sysinfo error");
    }
#endif

    LOG_F(INFO, "Finished getSwapMemoryUsed function");
    return swapMemoryUsed;
}

auto getTotalMemory() -> size_t {
    LOG_F(INFO, "Starting getTotalMemory function");
    size_t totalMemory = 0;

#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        totalMemory = status.ullTotalPhys;
        LOG_F(INFO, "Total Memory: {} bytes", totalMemory);
    } else {
        LOG_F(ERROR, "GetTotalMemory error: GlobalMemoryStatusEx error");
    }
#elif defined(__linux__)
    std::ifstream memInfoFile("/proc/meminfo");
    std::string line;
    while (std::getline(memInfoFile, line)) {
        size_t value;
        if (sscanf(line, "MemTotal: {} kB", &value) == 1) {
            totalMemory = value * 1024;  // Convert kB to bytes
            LOG_F(INFO, "Total Memory: {} bytes", totalMemory);
            break;
        }
    }
#elif defined(__APPLE__)
    int mib[2];
    size_t length = sizeof(size_t);
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    if (sysctl(mib, 2, &totalMemory, &length, nullptr, 0) == 0) {
        LOG_F(INFO, "Total Memory: {} bytes", totalMemory);
    } else {
        LOG_F(ERROR, "GetTotalMemory error: sysctl error");
    }
#endif

    LOG_F(INFO, "Finished getTotalMemory function");
    return totalMemory;
}

auto getAvailableMemory() -> size_t {
#ifdef _WIN32
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status)) {
        return status.ullAvailPhys;
    }
    return 0;
#elif defined(__linux__)
    std::ifstream memInfoFile("/proc/meminfo");
    std::string line;
    size_t availableMemory = 0;
    while (std::getline(memInfoFile, line)) {
        size_t value;
        if (sscanf(line, "MemAvailable: {} kB", &value) == 1) {
            availableMemory = value * 1024;  // Convert kB to bytes
            break;
        }
    }
    return availableMemory;
#elif defined(__APPLE__)
    int mib[2];
    size_t length = sizeof(vm_statistics64);
    struct vm_statistics64 vm_stats;

    mib[0] = CTL_VM;
    mib[1] = VM_LOADAVG;
    if (sysctl(mib, 2, &vm_stats, &length, nullptr, 0) == 0) {
        return vm_stats.free_count * vm_page_size;
    }
    return 0;
#endif
}

auto getCommittedMemory() -> size_t {
    size_t totalMemory = getTotalMemory();
    size_t availableMemory = getAvailableMemory();
    return totalMemory - availableMemory;
}

auto getUncommittedMemory() -> size_t {
    size_t totalMemory = getTotalMemory();
    size_t committedMemory = getCommittedMemory();
    return totalMemory - committedMemory;
}

}  // namespace atom::system
