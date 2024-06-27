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

SysInfoComponent::SysInfoComponent(const std::string& name) : Component(name) {
    LOG_F(INFO, "SysInfoComponent Constructed");

    def("cpu_usage", &atom::system::getCurrentCpuUsage, "cpu",
        "Get current CPU usage percentage");
    def("cpu_temperature", &atom::system::getCurrentCpuTemperature, "cpu",
        "Get current CPU temperature");
    def("memory_usage", &atom::system::getMemoryUsage, "memory",
        "Get current memory usage percentage");
    def("is_charging", &SysInfoComponent::isBatteryCharging,
        PointerSentinel(this), "battery", "Check if the battery is charging");
    def("battery_level", &SysInfoComponent::getCurrentBatteryLevel,
        PointerSentinel(this), "battery", "Get current battery level");
    def("disk_usage", &atom::system::getDiskUsage, "disk",
        "Get current disk usage percentage");
    def("is_hotspot_connected", &atom::system::isHotspotConnected, "wifi",
        "Check if the hotspot is connected");
    def("wired_network", &atom::system::getCurrentWiredNetwork, "wifi",
        "Get current wired network");
    def("wifi_name", &atom::system::getCurrentWifi, "wifi",
        "Get current wifi name");
    def("current_ip", &atom::system::getHostIPs, "network",
        "Get current IP address");
    def("gpu_info", &atom::system::getGPUInfo, "gpu", "Get GPU info");
    def("os_name", &SysInfoComponent::getOSName, PointerSentinel(this), "os",
        "Get OS name");
    def("os_version", &SysInfoComponent::getOSVersion, PointerSentinel(this),
        "os", "Get OS version");
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
