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

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
namespace fs = std::filesystem;

#ifdef _WIN32
#include <Windows.h>
#include <Psapi.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <pdh.h>
#include <tlhelp32.h>
#include <wincon.h>
#elif __linux__
#include <dirent.h>
#include <limits.h>
#include <signal.h>
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
float getCurrentCpuUsage() {
    float cpu_usage = 0.0;

#ifdef _WIN32
    PDH_HQUERY query;
    PdhOpenQuery(nullptr, 0, &query);

    PDH_HCOUNTER counter;
    PdhAddCounter(query, "\\Processor(_Total)\\% Processor Time", 0, &counter);
    PdhCollectQueryData(query);

    PDH_FMT_COUNTERVALUE counter_value;
    PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr,
                                &counter_value);

    cpu_usage = static_cast<float>(counter_value.doubleValue);

    PdhCloseQuery(query);
#elif __linux__
    std::ifstream file("/proc/stat");
    if (!file.is_open()) {
        LOG_F(ERROR, "Failed to open /proc/stat");
        return cpu_usage;
    }
    std::string line;
    std::getline(file, line);  // 读取第一行

    std::istringstream iss(line);
    std::vector<std::string> tokens(std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>());

    unsigned long total_time = 0;
    for (size_t i = 1; i < tokens.size(); i++) {
        total_time += std::stoul(tokens[i]);
    }

    unsigned long idle_time = std::stoul(tokens[4]);

    float usage = static_cast<float>(total_time - idle_time) / total_time;
    cpu_usage = usage * 100.0;
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

        cpu_usage = static_cast<float>(user_time + sys_time) / total_time;
        cpu_usage *= 100.0;
    } else {
        LOG_F(ERROR, "Failed to get CPU temperature");
    }
#elif __ANDROID__
    // Android 实现
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

    cpu_usage = static_cast<float>(user_time + system_time) / total_time;
    cpu_usage *= 100.0;
#endif

    return cpu_usage;
}

float getCurrentCpuTemperature() {
    float temperature = 0.0f;

#ifdef _WIN32
    HKEY hKey;
    DWORD temperatureValue = 0;
    DWORD size = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&temperatureValue,
                            &size) == ERROR_SUCCESS) {
            temperature = static_cast<float>(temperatureValue) / 10.0f;
        }
        RegCloseKey(hKey);
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
                } catch (const std::exception &e) {
                    LOG_F(ERROR, "GetCpuTemperature error: {}", e.what());
                }
            }
        } else {
            LOG_F(ERROR, "GetCpuTemperature error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int temp = 0;
        tempFile >> temp;
        tempFile.close();
        temperature = static_cast<float>(temp) / 1000.0f;
    } else {
        LOG_F(ERROR, "GetMemoryUsage error: open /proc/meminfo error");
    }
#elif defined(__ANDROID__)
    // Android 实现
    std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
    if (tempFile.is_open()) {
        int temp = 0;
        tempFile >> temp;
        tempFile.close();
        temperature = static_cast<float>(temp) / 1000.0f;
    } else {
        LOG_F(ERROR,
              "GetCpuTemperature error: open "
              "/sys/class/thermal/thermal_zone0/temp error");
    }
#endif

    return temperature;
}

std::string getCPUModel() {
    std::string cpuModel;
#ifdef _WIN32

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        char cpuName[1024];
        DWORD size = sizeof(cpuName);
        if (RegQueryValueEx(hKey, "ProcessorNameString", NULL, NULL,
                            (LPBYTE)cpuName, &size) == ERROR_SUCCESS) {
            cpuModel = cpuName;
        }
        RegCloseKey(hKey);
    }

#elif __linux__

    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 10) == "model name") {
            cpuModel = line.substr(line.find(":") + 2);
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
        } else {
            LOG_F(ERROR, "GetCPUModel error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__ANDROID__)
    FILE *pipe = popen("getprop ro.product.model", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            cpuModel = buffer;
            cpuModel.erase(std::remove(cpuModel.begin(), cpuModel.end(), '\n'),
                           cpuModel.end());
        } else {
            LOG_F(ERROR, "GetCPUModel error: popen error");
        }
        pclose(pipe);
    }
#endif
    return cpuModel;
}

std::string getProcessorIdentifier() {
    std::string identifier;

#ifdef _WIN32
    HKEY hKey;
    char identifierValue[256];
    DWORD bufSize = sizeof(identifierValue);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "Identifier", NULL, NULL, (LPBYTE)identifierValue,
                        &bufSize);
        RegCloseKey(hKey);

        identifier = identifierValue;
    }
#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 9) == "processor") {
            identifier = line.substr(line.find(":") + 2);
            break;
        }
    }
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n machdep.cpu.brand_string", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            identifier = buffer;
            identifier.erase(
                std::remove(identifier.begin(), identifier.end(), '\n'),
                identifier.end());
        } else {
            LOG_F(ERROR, "GetProcessorIdentifier error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__ANDROID__)
    // Android 实现
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 9) == "processor") {
            identifier = line.substr(line.find(":") + 2);
            break;
        }
    }
    cpuinfo.close();
#endif

    return identifier;
}

double getProcessorFrequency() {
    double frequency = 0;

#ifdef _WIN32
    HKEY hKey;
    DWORD frequencyValue;
    DWORD bufSize = sizeof(frequencyValue);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&frequencyValue,
                        &bufSize);
        RegCloseKey(hKey);

        frequency = static_cast<double>(frequencyValue) /
                    1000.0;  // Convert frequency to GHz
    }
#elif defined(__linux__)
    // Linux 实现
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 7) == "cpu MHz") {
            std::size_t pos = line.find(":") + 2;
            frequency = std::stod(line.substr(pos)) /
                        1000.0;  // Convert frequency to GHz
            break;
        }
    }
    cpuinfo.close();
#elif defined(__APPLE__)
    // macOS 实现
    FILE *pipe = popen("sysctl -n hw.cpufrequency", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            frequency = std::stod(buffer) / 1e9;  // Convert frequency to GHz
        } else {
            LOG_F(ERROR, "GetProcessorFrequency error: popen error");
        }
        pclose(pipe);
    }
#endif

    return frequency;
}

int getNumberOfPhysicalPackages() {
    int numberOfPackages = 0;

#ifdef _WIN32
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    numberOfPackages = systemInfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n hw.packages", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            numberOfPackages = std::stoi(buffer);
        } else {
            LOG_F(ERROR, "GetNumberOfPhysicalPackages error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    numberOfPackages = sysconf(_SC_PHYS_PAGES);
#endif

    return numberOfPackages;
}

int getNumberOfPhysicalCPUs() {
    int numberOfCPUs = 0;

#ifdef _WIN32
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    numberOfCPUs = systemInfo.dwNumberOfProcessors;
#elif defined(__APPLE__)
    FILE *pipe = popen("sysctl -n hw.physicalcpu", "r");
    if (pipe != nullptr) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            numberOfCPUs = std::stoi(buffer);
        } else {
            LOG_F(ERROR, "GetNumberOfPhysicalCPUs error: popen error");
        }
        pclose(pipe);
    }
#elif defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 7) == "physical") {
            numberOfCPUs = std::stoi(line.substr(line.find(":") + 2));
            break;
        }
    }
#endif

    return numberOfCPUs;
}

}  // namespace atom::system
