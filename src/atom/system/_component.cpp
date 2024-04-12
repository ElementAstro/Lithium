/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Component of Atom-System

**************************************************/

#include "_component.hpp"

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

#include "module/battery.hpp"
#include "module/cpu.hpp"
#include "module/disk.hpp"
#include "module/gpu.hpp"
#include "module/memory.hpp"
#include "module/os.hpp"
#include "module/wifi.hpp"

#include "_constant.hpp"

#define ATOM_SYSTEM_NO_ARGS                                                 \
    if (!m_params.is_null()) {                                              \
        LOG_F(ERROR, "SystemComponent::{}: Invalid params", __func__);      \
        return createErrorResponse(__func__,                                \
                                   {"error", Constants::INVALID_PARAMETER}, \
                                   "no argument should be found");          \
    }

#define GET_CPU_INFO(name, func)                                              \
    auto name = func();                                                       \
    if (name < 0 || name > 100) {                                             \
        LOG_F(ERROR, "SystemComponent::getCPUInfo: Failed to get {}", #name); \
        return createErrorResponse(__func__,                                  \
                                   {"error", "failed to get " #name},         \
                                   Constants::SYSTEM_ERROR);                  \
    }

#define GET_CPU_INFO_S(name, func)                                            \
    auto name = func();                                                       \
    if (name.empty()) {                                                       \
        LOG_F(ERROR, "SystemComponent::getCPUInfo: Failed to get {}", #name); \
        return createErrorResponse(__func__,                                  \
                                   {"error", "failed to get " #name},         \
                                   Constants::SYSTEM_ERROR);                  \
    }

#define GET_MEMORY_INFO(name, func)                                      \
    auto name = func();                                                  \
    if (name < 0) {                                                      \
        LOG_F(ERROR, "SystemComponent::getMemoryInfo: Failed to get {}", \
              #name);                                                    \
        return createErrorResponse(__func__,                             \
                                   {"error", "failed to get " #name},    \
                                   Constants::SYSTEM_ERROR);             \
    }

SystemComponent::SystemComponent(const std::string &name)
    : SharedComponent(name) {
    DLOG_F(INFO, "SystemComponent::SystemComponent");

    registerFunc("getCPUInfo", &SystemComponent::getCPUInfo, this);
    registerFunc("getMemoryInfo", &SystemComponent::getMemoryInfo, this);
    registerFunc("getDiskInfo", &SystemComponent::getDiskInfo, this);
    registerFunc("getNetworkInfo", &SystemComponent::getNetworkInfo, this);
    registerFunc("getBatteryInfo", &SystemComponent::getBatteryInfo, this);
    registerFunc("getGPUInfo", &SystemComponent::getGPUInfo, this);
    registerFunc("getOSInfo", &SystemComponent::getOSInfo, this);

    registerVariable("cpuUsage", "", "The CPU usage");
    registerVariable("cpuTemperature", "", "The CPU temperature");
    registerVariable("cpuModel", "", "The CPU model");
    registerVariable("cpuFrequency", "", "The CPU frequency");
    registerVariable("numberOfPhysicalPackages", "",
                     "The number of physical "
                     "packages");
    registerVariable("numberOfPhysicalCPUs", "", "The number of physical CPUs");
    registerVariable("processorIdentifier", "", "The processor identifier");
    registerVariable("processorFrequency", "", "The processor frequency");

    registerVariable("memoryUsage", "", "The memory usage");
    registerVariable("memoryTotal", "", "The total memory");
    registerVariable("memoryMax", "", "The maximum memory");
    registerVariable("memoryUsed", "", "The used memory");
    registerVariable("memoryAvailable", "", "The available memory");
    registerVariable("memorySwapTotal", "", "The total swap memory");

    registerVariable("diskUsage", "", "The disk usage");
    registerVariable("diskTotal", "", "The total disk");
    registerVariable("diskAvailable", "", "The available disk");
    registerVariable("diskUsed", "", "The used disk");

    registerVariable("networkWifi", "", "The wifi network");
    registerVariable("networkWired", "", "The wired network");
    registerVariable("networkHotspot", "", "The hotspot network");
}

SystemComponent::~SystemComponent() {
    DLOG_F(INFO, "SystemComponent::~SystemComponent");
}

bool SystemComponent::initialize() { return true; }

bool SystemComponent::destroy() { return true; }

json SystemComponent::getCPUInfo(const json &m_params) {
    DLOG_F(INFO, "SystemComponent::getCPUInfo");

    ATOM_SYSTEM_NO_ARGS;

    GET_CPU_INFO(current_cpu_usage, Atom::System::getCurrentCpuUsage);
    GET_CPU_INFO(current_cpu_temperature,
                 Atom::System::getCurrentCpuTemperature);
    GET_CPU_INFO_S(cpu_model, Atom::System::getCPUModel);
    GET_CPU_INFO_S(processor_identifier, Atom::System::getProcessorIdentifier);
    GET_CPU_INFO(processor_frequency, Atom::System::getProcessorFrequency);
    GET_CPU_INFO(number_of_physical_packages,
                 Atom::System::getNumberOfPhysicalPackages);
    GET_CPU_INFO(number_of_physical_cpus,
                 Atom::System::getNumberOfPhysicalCPUs);

    return createSuccessResponse(
        __func__, {
                      {"currentCpuUsage", current_cpu_usage},
                      {"currentCpuTemperature", current_cpu_temperature},
                      {"cpuModel", cpu_model},
                      {"processorIdentifier", processor_identifier},
                      {"processorFrequency", processor_frequency},
                      {"numberOfPhysicalPackages", number_of_physical_packages},
                      {"numberOfPhysicalCPUs", number_of_physical_cpus},
                  });
}

json SystemComponent::getMemoryInfo(const json &m_params) {
    DLOG_F(INFO, "SystemComponent::getMemoryInfo");
    ATOM_SYSTEM_NO_ARGS;

    GET_MEMORY_INFO(current_memory_usage, Atom::System::getMemoryUsage);
    GET_MEMORY_INFO(total_memory_size, Atom::System::getTotalMemorySize);
    GET_MEMORY_INFO(available_memory_size,
                    Atom::System::getAvailableMemorySize);
    GET_MEMORY_INFO(virtual_memory_max, Atom::System::getVirtualMemoryMax);
    GET_MEMORY_INFO(virtual_memory_used, Atom::System::getVirtualMemoryUsed);
    GET_MEMORY_INFO(swap_memory_used, Atom::System::getSwapMemoryUsed);
    GET_MEMORY_INFO(swap_memory_total, Atom::System::getSwapMemoryTotal);

    return createSuccessResponse(
        __func__, {
                      {"currentMemoryUsage", current_memory_usage},
                      {"totalMemorySize", total_memory_size},
                      {"availableMemorySize", available_memory_size},
                      {"virtualMemoryMax", virtual_memory_max},
                      {"virtualMemoryUsed", virtual_memory_used},
                      {"swapMemoryUsed", swap_memory_used},
                      {"swapMemoryTotal", swap_memory_total},
                  });
}

json SystemComponent::getDiskInfo(const json &m_params) {
    DLOG_F(INFO, "SystemComponent::getDiskInfo");
    ATOM_SYSTEM_NO_ARGS;
    auto disks_usage = Atom::System::getDiskUsage();
    if (disks_usage.size() == 0) {
        LOG_F(ERROR, "SystemComponent::getDiskInfo: Failed to get disk info");
        return createErrorResponse(__func__,
                                   {"error", "failed to get disk info"},
                                   Constants::SYSTEM_ERROR);
    }
    json res;
    for (auto &disk : disks_usage) {
        res.push_back({{"drive", disk.first}, {"usage", disk.second}});
    }
    return createSuccessResponse(__func__, res);
}

json SystemComponent::getBatteryInfo(const json &m_params) {
    DLOG_F(INFO, "SystemComponent::getBatteryInfo");
    ATOM_SYSTEM_NO_ARGS;
    auto battery_info = Atom::System::getBatteryInfo();
    if (battery_info.isBatteryPresent == false) {
        LOG_F(ERROR,
              "SystemComponent::getBatteryInfo: Failed to get battery info");
        return createErrorResponse(__func__,
                                   {"error", "failed to get battery info"},
                                   Constants::SYSTEM_ERROR);
    }
    return createSuccessResponse(
        __func__, {{"energyNow", battery_info.energyNow},
                   {"energyDesign", battery_info.energyDesign},
                   {"currentNow", battery_info.currentNow},
                   {"batteryLifeTime", battery_info.batteryLifeTime},
                   {"batteryFullLifeTime", battery_info.batteryFullLifeTime},
                   {"batteryLifePercent", battery_info.batteryLifePercent},
                   {"energyFull", battery_info.energyFull},
                   {"voltageNow", battery_info.voltageNow},
                   {"isBatteryPresent", battery_info.isBatteryPresent},
                   {"isCharging", battery_info.isCharging}});
}

json SystemComponent::getNetworkInfo(const json &m_params) {
    DLOG_F(INFO, "SystemComponent::getNetworkInfo");
    ATOM_SYSTEM_NO_ARGS;
    auto wifi = Atom::System::getCurrentWifi();
    auto wired = Atom::System::getCurrentWiredNetwork();
    auto hotspot = Atom::System::isHotspotConnected();
    return createSuccessResponse(
        __func__, {{"wifi", wifi}, {"wired", wired}, {"hotspot", hotspot}});
}

json SystemComponent::getGPUInfo(const json &m_params) {
    DLOG_F(INFO, "SystemComponent::getGPUInfo");
    ATOM_SYSTEM_NO_ARGS;
    auto gpu_info = Atom::System::getGPUInfo();
    if (gpu_info.empty()) {
        LOG_F(ERROR, "SystemComponent::getGPUInfo: Failed to get GPU info");
        return createErrorResponse(__func__,
                                   {"error", "failed to get GPU info"},
                                   Constants::SYSTEM_ERROR);
    }
    return createSuccessResponse(__func__, {"gpu1", gpu_info});
}

json SystemComponent::getOSInfo(const json &m_params) {
    DLOG_F(INFO, "SystemComponent::getOSInfo");
    ATOM_SYSTEM_NO_ARGS;
    auto os_info = Atom::System::getOperatingSystemInfo();
    return createSuccessResponse(__func__,
                                 {{"osName", os_info.osName},
                                  {"osVersion", os_info.osVersion},
                                  {"kernelVersion", os_info.kernelVersion},
                                  {"architecture", os_info.architecture},
                                  {"compiler", os_info.compiler}});
}