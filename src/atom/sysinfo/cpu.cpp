/*
 * cpu.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - CPU

**************************************************/

#include "atom/sysinfo/cpu.hpp"
#include "os.hpp"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>

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

#include "atom/log/loguru.hpp"

namespace atom::system {

auto getCurrentCpuUsage() -> float {
    LOG_F(INFO, "Starting getCurrentCpuUsage function");
    float cpuUsage = 0.0;

#ifdef _WIN32
    PDH_HQUERY query;
    PdhOpenQuery(nullptr, 0, &query);

    PDH_HCOUNTER counter;
    PdhAddCounter(query, "\\Processor(_Total)\\% Processor Time", 0, &counter);
    PdhCollectQueryData(query);

    PDH_FMT_COUNTERVALUE counterValue;
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr,
                                &counterValue);

    cpuUsage = static_cast<float>(counterValue.doubleValue);

    PdhCloseQuery(query);
    LOG_F(INFO, "CPU Usage: %.2f", cpuUsage);
#elif __linux__
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        LOG_F(ERROR, "Failed to open /proc/stat");
        return cpuUsage;
    }
    std::string line;
    std::getline(file, line);  // Read the first line

    std::istringstream iss(line);
    std::vector<std::string> tokens(std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>());

    unsigned long totalTime = 0;
    for (size_t i = 1; i < tokens.size(); i++) {
        totalTime += std::stoul(tokens[i]);
    }

    unsigned long idleTime = std::stoul(tokens[4]);

    float usage = static_cast<float>(totalTime - idleTime) / totalTime;
    cpuUsage = usage * 100.0;
    LOG_F(INFO, "CPU Usage: %.2f", cpuUsage);
#elif __APPLE__
    host_cpu_load_info_data_t cpu_load;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    if (host_statistics64(mach_host_self(), HOST_CPU_LOAD_INFO,
                          reinterpret_cast<host_info_t>(&cpu_load),
                          &count) == KERN_SUCCESS) {
        uint64_t user_time = cpu_load.cpu_ticks[CPU_STATE_USER] -
                             cpu_load.cpu_ticks[CPU_STATE_NICE];
        uint64_t sys_time = cpu_load.cpu_ticks[CPU_STATE_SYSTEM] +
                            cpu_load.cpu_ticks[CPU_STATE_NICE];
        uint64_t idle_time = cpu_load.cpu_ticks[CPU_STATE_IDLE];
        uint64_t total_time = user_time + sys_time + idle_time;

        cpuUsage = static_cast<float>(user_time + sys_time) / total_time;
        cpuUsage *= 100.0;
        LOG_F(INFO, "CPU Usage: %.2f", cpuUsage);
    } else {
        LOG_F(ERROR, "Failed to get CPU usage");
    }
#elif __ANDROID__
    // Android implementation
    android::sp<android::IBatteryStats> battery_stat_service =
        android::interface_cast<android::IBatteryStats>(
            android::defaultServiceManager()->getService(
                android::String16("batterystats")));
    android::BatteryStats::Uid uid =
        battery_stat_service->getUidStats(android::Process::myUid());
    int32_t user_time = uid.getUidCpuTime(android::BatteryStats::UID_TIME_USER);
    int32_t system_time =
        uid.getUidCpuTime(android::BatteryStats::UID_TIME_SYSTEM);
    int32_t idle_time = uid.getUidCpuTime(android::BatteryStats::UID_TIME_IDLE);
    int32_t total_time = user_time + system_time + idle_time;

    cpuUsage = static_cast<float>(user_time + system_time) / total_time;
    cpuUsage *= 100.0;
    LOG_F(INFO, "CPU Usage: %.2f", cpuUsage);
#endif

    LOG_F(INFO, "Finished getCurrentCpuUsage function");
    return cpuUsage;
}

auto getCurrentCpuTemperature() -> float {
    LOG_F(INFO, "Starting getCurrentCpuTemperature function");
    float temperature = 0.0F;

#ifdef _WIN32
    HKEY hKey;
    DWORD temperatureValue = 0;
    DWORD size = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)", 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, "~MHz", nullptr, nullptr,
                            (LPBYTE)&temperatureValue,
                            &size) == ERROR_SUCCESS) {
            temperature = static_cast<float>(temperatureValue) / 10.0F;
            LOG_F(INFO, "CPU Temperature: %.2f", temperature);
        }
        RegCloseKey(hKey);
    } else {
        LOG_F(ERROR, "Failed to open registry key for CPU temperature");
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -a | grep machdep.xcpm.cpu_thermal_level", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string result(buffer);
            size_t pos1 = result.find(": ");
            size_t pos2 = result.find("\n");
            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                std::string tempStr = result.substr(pos1 + 2, pos2 - pos1 - 2);
                try {
                    temperature = std::stof(tempStr);
                    LOG_F(INFO, "CPU Temperature: %.2f", temperature);
                } catch (const std::exception &e) {
                    LOG_F(ERROR, "GetCpuTemperature error: %s", e.what());
                }
            }
        } else {
            LOG_F(ERROR, "GetCpuTemperature error: popen error");
        }
        pclose(pipe);
    } else {
        LOG_F(ERROR, "Failed to open pipe for CPU temperature");
    }
#elif defined(__linux__)
    if (isWsl()) {
        LOG_F(WARNING, "GetCpuTemperature error: WSL not supported");
    } else {
        std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
        if (tempFile.is_open()) {
            int temp = 0;
            tempFile >> temp;
            tempFile.close();
            temperature = static_cast<float>(temp) / 1000.0F;
            LOG_F(INFO, "CPU Temperature: %.2f", temperature);
        } else {
            LOG_F(ERROR,
                  "GetCpuTemperature error: open "
                  "/sys/class/thermal/thermal_zone0/temp error");
        }
    }
#elif defined(__ANDROID__)
    // Android implementation
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int temp = 0;
        tempFile >> temp;
        tempFile.close();
        temperature = static_cast<float>(temp) / 1000.0f;
        LOG_F(INFO, "CPU Temperature: %.2f", temperature);
    } else {
        LOG_F(ERROR,
              "GetCpuTemperature error: open "
              "/sys/class/thermal/thermal_zone0/temp error");
    }
#endif

    LOG_F(INFO, "Finished getCurrentCpuTemperature function");
    return temperature;
}

auto getCPUModel() -> std::string {
    LOG_F(INFO, "Starting getCPUModel function");
    std::string cpuModel;
#ifdef _WIN32

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)", 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        char cpuName[1024];
        DWORD size = sizeof(cpuName);
        if (RegQueryValueEx(hKey, "ProcessorNameString", nullptr, nullptr,
                            (LPBYTE)cpuName, &size) == ERROR_SUCCESS) {
            cpuModel = cpuName;
            LOG_F(INFO, "CPU Model: %s", cpuModel.c_str());
        }
        RegCloseKey(hKey);
    } else {
        LOG_F(ERROR, "Failed to open registry key for CPU model");
    }

#elif __linux__

    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 10) == "model name") {
            cpuModel = line.substr(line.find(':') + 2);
            LOG_F(INFO, "CPU Model: %s", cpuModel.c_str());
            break;
        }
    }
    cpuinfo.close();
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n machdep.cpu.brand_string", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            cpuModel = buffer;
            cpuModel.erase(std::remove(cpuModel.begin(), cpuModel.end(), '\n'),
                           cpuModel.end());
            LOG_F(INFO, "CPU Model: %s", cpuModel.c_str());
        } else {
            LOG_F(ERROR, "GetCPUModel error: popen error");
        }
        pclose(pipe);
    } else {
        LOG_F(ERROR, "Failed to open pipe for CPU model");
    }
#elif defined(__ANDROID__)
    FILE *pipe = popen("getprop ro.product.model", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            cpuModel = buffer;
            cpuModel.erase(std::remove(cpuModel.begin(), cpuModel.end(), '\n'),
                           cpuModel.end());
            LOG_F(INFO, "CPU Model: %s", cpuModel.c_str());
        } else {
            LOG_F(ERROR, "GetCPUModel error: popen error");
        }
        pclose(pipe);
    } else {
        LOG_F(ERROR, "Failed to open pipe for CPU model");
    }
#endif
    LOG_F(INFO, "Finished getCPUModel function");
    return cpuModel;
}

auto getProcessorIdentifier() -> std::string {
    LOG_F(INFO, "Starting getProcessorIdentifier function");
    std::string identifier;

#ifdef _WIN32
    HKEY hKey;
    char identifierValue[256];
    DWORD bufSize = sizeof(identifierValue);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)", 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "Identifier", nullptr, nullptr,
                        (LPBYTE)identifierValue, &bufSize);
        RegCloseKey(hKey);

        identifier = identifierValue;
        LOG_F(INFO, "Processor Identifier: %s", identifier.c_str());
    } else {
        LOG_F(ERROR, "Failed to open registry key for processor identifier");
    }
#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 9) == "processor") {
            identifier = line.substr(line.find(':') + 2);
            LOG_F(INFO, "Processor Identifier: %s", identifier.c_str());
            break;
        }
    }
    cpuinfo.close();
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n machdep.cpu.brand_string", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            identifier = buffer;
            identifier.erase(
                std::remove(identifier.begin(), identifier.end(), '\n'),
                identifier.end());
            LOG_F(INFO, "Processor Identifier: %s", identifier.c_str());
        } else {
            LOG_F(ERROR, "GetProcessorIdentifier error: popen error");
        }
        pclose(pipe);
    } else {
        LOG_F(ERROR, "Failed to open pipe for processor identifier");
    }
#elif defined(__ANDROID__)
    // Android implementation
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 9) == "processor") {
            identifier = line.substr(line.find(":") + 2);
            LOG_F(INFO, "Processor Identifier: %s", identifier.c_str());
            break;
        }
    }
    cpuinfo.close();
#endif

    LOG_F(INFO, "Finished getProcessorIdentifier function");
    return identifier;
}

auto getProcessorFrequency() -> double {
    LOG_F(INFO, "Starting getProcessorFrequency function");
    double frequency = 0;

#ifdef _WIN32
    HKEY hKey;
    DWORD frequencyValue;
    DWORD bufSize = sizeof(frequencyValue);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     R"(HARDWARE\DESCRIPTION\System\CentralProcessor\0)", 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "~MHz", nullptr, nullptr, (LPBYTE)&frequencyValue,
                        &bufSize);
        RegCloseKey(hKey);

        frequency = static_cast<double>(frequencyValue) /
                    1000.0;  // Convert frequency to GHz
        LOG_F(INFO, "Processor Frequency: %.2f GHz", frequency);
    } else {
        LOG_F(ERROR, "Failed to open registry key for processor frequency");
    }
#elif defined(__linux__)
    // Linux implementation
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 7) == "cpu MHz") {
            std::size_t pos = line.find(':') + 2;
            frequency = std::stod(line.substr(pos)) /
                        1000.0;  // Convert frequency to GHz
            LOG_F(INFO, "Processor Frequency: %.2f GHz", frequency);
            break;
        }
    }
    cpuinfo.close();
#elif defined(__APPLE__)
    // macOS implementation
    FILE *pipe = popen("sysctl -n hw.cpufrequency", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            frequency = std::stod(buffer) / 1e9;  // Convert frequency to GHz
            LOG_F(INFO, "Processor Frequency: %.2f GHz", frequency);
        } else {
            LOG_F(ERROR, "GetProcessorFrequency error: popen error");
        }
        pclose(pipe);
    } else {
        LOG_F(ERROR, "Failed to open pipe for processor frequency");
    }
#endif

    LOG_F(INFO, "Finished getProcessorFrequency function");
    return frequency;
}

auto getNumberOfPhysicalPackages() -> int {
    LOG_F(INFO, "Starting getNumberOfPhysicalPackages function");
    int numberOfPackages = 0;

#ifdef _WIN32
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    numberOfPackages = systemInfo.dwNumberOfProcessors;
    LOG_F(INFO, "Number of Physical Packages: %d", numberOfPackages);
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n hw.packages", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            numberOfPackages = std::stoi(buffer);
            LOG_F(INFO, "Number of Physical Packages: %d", numberOfPackages);
        } else {
            LOG_F(ERROR, "GetNumberOfPhysicalPackages error: popen error");
        }
        pclose(pipe);
    } else {
        LOG_F(ERROR, "Failed to open pipe for number of physical packages");
    }
#elif defined(__linux__)
    numberOfPackages = static_cast<int>(sysconf(_SC_PHYS_PAGES));
    LOG_F(INFO, "Number of Physical Packages: %d", numberOfPackages);
#endif

    LOG_F(INFO, "Finished getNumberOfPhysicalPackages function");
    return numberOfPackages;
}

auto getNumberOfPhysicalCPUs() -> int {
    LOG_F(INFO, "Starting getNumberOfPhysicalCPUs function");
    int numberOfCPUs = 0;

#ifdef _WIN32
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    numberOfCPUs = systemInfo.dwNumberOfProcessors;
    LOG_F(INFO, "Number of Physical CPUs: %d", numberOfCPUs);
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n hw.physicalcpu", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            numberOfCPUs = std::stoi(buffer);
            LOG_F(INFO, "Number of Physical CPUs: %d", numberOfCPUs);
        } else {
            LOG_F(ERROR, "GetNumberOfPhysicalCPUs error: popen error");
        }
        pclose(pipe);
    } else {
        LOG_F(ERROR, "Failed to open pipe for number of physical CPUs");
    }
#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 7) == "physical") {
            numberOfCPUs = std::stoi(line.substr(line.find(':') + 2));
            LOG_F(INFO, "Number of Physical CPUs: %d", numberOfCPUs);
            break;
        }
    }
    cpuinfo.close();
#endif

    LOG_F(INFO, "Finished getNumberOfPhysicalCPUs function");
    return numberOfCPUs;
}

auto getCacheSizes() -> CacheSizes {
    CacheSizes cacheSizes{0, 0, 0, 0};

#ifdef _WIN32
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *info = nullptr;
    DWORD bufferSize = 0;

    // Get required buffer size
    GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufferSize);
    info = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)malloc(bufferSize);
    if (!info)
        return cacheSizes;

    if (GetLogicalProcessorInformationEx(RelationCache, info, &bufferSize)) {
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *current = info;
        while ((char *)current < (char *)info + bufferSize) {
            if (current->Relationship == RelationCache) {
                switch (current->Cache.Type) {
                    case CacheUnified:
                        if (current->Cache.Level == 3)
                            cacheSizes.l3 = current->Cache.CacheSize / 1024;
                        break;
                    case CacheData:
                        if (current->Cache.Level == 1)
                            cacheSizes.l1d = current->Cache.CacheSize / 1024;
                        else if (current->Cache.Level == 2)
                            cacheSizes.l2 = current->Cache.CacheSize / 1024;
                        break;
                    case CacheInstruction:
                        if (current->Cache.Level == 1)
                            cacheSizes.l1i = current->Cache.CacheSize / 1024;
                        break;
                    default:
                        break;
                }
            }
            current =
                (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)((char *)current +
                                                            current->Size);
        }
    }
    free(info);

#elif defined(__linux__)
    std::vector<std::string> cacheLevels = {
        "/sys/devices/system/cpu/cpu0/cache/index1/size",
        "/sys/devices/system/cpu/cpu0/cache/index2/size",
        "/sys/devices/system/cpu/cpu0/cache/index3/size"};
    for (const auto &path : cacheLevels) {
        std::ifstream file(path);
        if (file) {
            std::string sizeStr;
            std::getline(file, sizeStr);
            size_t size = std::stoul(sizeStr) * 1024;  // Convert KB to bytes
            if (path.find("index1") != std::string::npos)
                cacheSizes.l1i = size / 1024;
            else if (path.find("index2") != std::string::npos)
                cacheSizes.l2 = size / 1024;
            else if (path.find("index3") != std::string::npos)
                cacheSizes.l3 = size / 1024;
        }
    }

#elif defined(__APPLE__)
    size_t l1i = 0, l1d = 0, l2 = 0, l3 = 0;
    size_t length = sizeof(size_t);

    if (sysctlbyname("machdep.cpu.cache.l1i.size", &l1i, &length, nullptr, 0) ==
        0)
        cacheSizes.l1i = l1i / 1024;
    if (sysctlbyname("machdep.cpu.cache.l1d.size", &l1d, &length, nullptr, 0) ==
        0)
        cacheSizes.l1d = l1d / 1024;
    if (sysctlbyname("machdep.cpu.cache.l2.size", &l2, &length, nullptr, 0) ==
        0)
        cacheSizes.l2 = l2 / 1024;
    if (sysctlbyname("machdep.cpu.cache.l3.size", &l3, &length, nullptr, 0) ==
        0)
        cacheSizes.l3 = l3 / 1024;
#endif

    return cacheSizes;
}

}  // namespace atom::system
