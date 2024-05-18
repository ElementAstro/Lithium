/*
 * _component.cpp
 *
 * Copyright (C) 2023-2024 Max Qian
 */

/*************************************************

Date: 2024-05-18

Description: A collector for system information, not the same as atom.system

**************************************************/

#include "_component.hpp"

#include "atom/sysinfo/battery.hpp"
#include "atom/sysinfo/cpu.hpp"
#include "atom/sysinfo/disk.hpp"
#include "atom/sysinfo/gpu.hpp"
#include "atom/sysinfo/memory.hpp"
#include "atom/sysinfo/os.hpp"
#include "atom/sysinfo/wifi.hpp"

#include "atom/log/loguru.hpp"

SysInfoComponent::SysInfoComponent(const std::string& name)
    : Component(name) {
    LOG_F(INFO, "SysInfoComponent Constructed");

    def("cpu_usage", &getCurrentCpuUsage, "cpu",
                    "Get current CPU usage percentage");
    def("cpu_temperature", &getCurrentCpuTemperature, "cpu",
                    "Get current CPU temperature");
    def("memory_usage", &getMemoryUsage, "memory",
                    "Get current memory usage percentage");
    def("is_charging", &isBatteryCharging, PointerSentinel(this),
                    "battery", "Check if the battery is charging");
    def("battery_level", &getCurrentBatteryLevel,
                    PointerSentinel(this), "battery",
                    "Get current battery level");
    def("disk_usage", &getDiskUsage, "disk",
                    "Get current disk usage percentage");
    def("is_hotspot_connected", &isHotspotConnected, "wifi",
                    "Check if the hotspot is connected");
    def("wired_network", &getCurrentWiredNetwork, "wifi",
                    "Get current wired network");
    def("wifi_name", &getCurrentWifi, "wifi",
                    "Get current wifi name");
    def("current_ip", &getHostIPs, "network",
                    "Get current IP address");
    def("gpu_info", &getGPUInfo, "gpu", "Get GPU info");
    def("os_name", &getOSName, PointerSentinel(this), "os",
                    "Get OS name");
    def("os_version", &getOSVersion, PointerSentinel(this), "os",
                    "Get OS version");
}

SysInfoComponent::~SysInfoComponent() {
    LOG_F(INFO, "SysInfoComponent Destructed");
}

bool SysInfoComponent::initialize() {
    LOG_F(INFO, "SysInfoComponent Initialized");
    return true;
}

bool SysInfoComponent::destroy() {
    LOG_F(INFO, "SysInfoComponent Destroyed");
    return true;
}

double SysInfoComponent::getCurrentBatteryLevel() {
    return atom::system::getBatteryInfo().currentNow;
}

bool SysInfoComponent::isBatteryCharging() {
    return atom::system::getBatteryInfo().isCharging;
}

std::string SysInfoComponent::getOSName() {
    return atom::system::getOperatingSystemInfo().osName;
}

std::string SysInfoComponent::getOSVersion() {
    return atom::system::getOperatingSystemInfo().osVersion;
}

std::string SysInfoComponent::getKernelVersion() {
    return atom::system::getOperatingSystemInfo().kernelVersion;
}

std::string SysInfoComponent::getArchitecture() {
    return atom::system::getOperatingSystemInfo().architecture;
}
