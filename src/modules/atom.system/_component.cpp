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

#include "module/battery.hpp"
#include "module/cpu.hpp"
#include "module/disk.hpp"
#include "module/gpu.hpp"
#include "module/memory.hpp"
#include "module/os.hpp"
#include "module/wifi.hpp"

#include "command.hpp"
#include "crash.hpp"
#include "os.hpp"
#include "pidwatcher.hpp"
#include "platform.hpp"
#include "register.hpp"
#include "user.hpp"

#include "_constant.hpp"

using namespace atom::system;

SystemComponent::SystemComponent(const std::string &name) : Component(name) {
    DLOG_F(INFO, "SystemComponent::SystemComponent");
    registerCommand("cpu_usage", &getCurrentCpuUsage, "cpu",
                    "Get current CPU usage percentage");
    registerCommand("cpu_temperature", &getCurrentCpuTemperature, "cpu",
                    "Get current CPU temperature");
    registerCommand("memory_usage", &getMemoryUsage, "memory",
                    "Get current memory usage percentage");
    registerCommand("is_charging", &isBatteryCharging, PointerSentinel(this),
                    "battery", "Check if the battery is charging");
    registerCommand("battery_level", &getCurrentBatteryLevel,
                    PointerSentinel(this), "battery",
                    "Get current battery level");
    registerCommand("disk_usage", &getDiskUsage, "disk",
                    "Get current disk usage percentage");
    registerCommand("is_hotspot_connected", &isHotspotConnected, "wifi",
                    "Check if the hotspot is connected");
    registerCommand("wired_network", &getCurrentWiredNetwork, "wifi",
                    "Get current wired network");
    registerCommand("wifi_name", &getCurrentWifi, "wifi",
                    "Get current wifi name");
    registerCommand("current_ip", &getHostIPs, "network",
                    "Get current IP address");
    registerCommand("gpu_info", &getGPUInfo, "gpu", "Get GPU info");
    registerCommand("os_name", &getOSName, PointerSentinel(this), "os",
                    "Get OS name");
    registerCommand("os_version", &getOSVersion, PointerSentinel(this), "os",
                    "Get OS version");
    registerCommand("run_commands", &executeCommands, "os",
                    "Run a list of system commands");
    registerCommand("run_command_env", &executeCommandWithEnv, "os",
                    "Run a system command with environment variables");
    registerCommand("run_command_status", &executeCommandWithStatus, "os",
                    "Run a system command and get its status");
    registerCommand("kill_process", &killProcess, "os",
                    "Kill a process by its PID");

    registerCommand("walk", &walk, "os", "Walk a directory");
    registerCommand("fwalk", &fwalk, "os", "Walk a directory");
    registerCommand("uname", &uname, "os", "Get uname information");
    registerCommand("ctermid", &ctermid, "os", "Get current terminal ID");
    registerCommand("jwalk", &jwalk, "os", "Walk a directory");
    registerCommand("getpriority", &getpriority, "os",
                    "Get current process priority");
    registerCommand("getlogin", &getlogin, "os", "Get current user name");
    registerCommand("Environ", &Environ, "os", "Get environment variables");

    registerCommand("user_group", &getUserGroups, "user",
                    "Get current user groups");
    registerCommand("user_id", &getUserId, "user", "Get current user ID");
    registerCommand("user_host", &getHostname, "user",
                    "Get current user hostname");
    registerCommand("user_name", &getUsername, "user", "Get current user name");
    registerCommand("user_home", &getHomeDirectory, "user",
                    "Get current user home directory");

    registerCommand("user_shell", &getLoginShell, "user",
                    "Get current user login shell");

    registerCommand("user_groups", &getUserGroups, "user",
                    "Get current user groups");

    addVariable("platform", platform, "Platform", "os_name", "os");
    addVariable("architecture", architecture, "Architecture", "os_arch", "os");
    addVariable("os_version", os_version, "OS Version", "kernel_version", "os");
    addVariable("compiler", compiler, "Compiler", "builder", "os");

    registerCommand("make_pidwatcher", &makePidWatcher, PointerSentinel(this),
                    "os", "Make a PID watcher");
    registerCommand("start_watcher", &startPidWatcher, PointerSentinel(this),
                    "os", "Start a PID watcher");
    registerCommand("stop_watcher", &stopPidWatcher, PointerSentinel(this),
                    "os", "Stop a PID watcher");
    registerCommand("switch_watcher", &switchPidWatcher, PointerSentinel(this),
                    "os", "Switch a PID watcher");
    registerCommand("set_watcher_exit", &setPidWatcherExitCallback,
                    PointerSentinel(this), "os",
                    "Set a PID watcher exit callback");
    registerCommand("set_watcher_monitor", &setPidWatcherMonitorFunction,
                    PointerSentinel(this), "os",
                    "Set a PID watcher monitor callback");

#if ENABLE_REGISTRY_SUPPORT
    registerCommand("get_registry_subkeys", &getRegistrySubKeys, "os",
                    "Get registry subkeys");
    registerCommand("get_registry_values", &getRegistryValues, "os",
                    "Get registry values");
    registerCommand("delete_registry_subkey", &deleteRegistrySubKey, "os",
                    "Delete registry subkey");
    registerCommand("modify_registry_value", &modifyRegistryValue, "os",
                    "Modify registry value");
    registerCommand("recursively_enumerate_registry_subkeys",
                    &recursivelyEnumerateRegistrySubKeys, "os",
                    "Recursively enumerate registry subkeys");
    registerCommand("find_registry_key", &findRegistryKey, "os",
                    "Find registry key");
    registerCommand("find_registry_value", &findRegistryValue, "os",
                    "Find registry value");
    registerCommand("backup_registry", &backupRegistry, "os",
                    "Backup registry");
    registerCommand("export_registry", &exportRegistry, "os",
                    "Export registry");
    registerCommand("delete_registry_value", &deleteRegistryValue, "os",
                    "Delete registry value");
#endif

    registerCommand("save_crashreport", &saveCrashLog, "os",
                    "Save crash report");
}

SystemComponent::~SystemComponent() {
    DLOG_F(INFO, "SystemComponent::~SystemComponent");
}

bool SystemComponent::initialize() { return true; }

bool SystemComponent::destroy() { return true; }

double SystemComponent::getCurrentBatteryLevel() {
    return atom::system::getBatteryInfo().currentNow;
}

bool SystemComponent::isBatteryCharging() {
    return atom::system::getBatteryInfo().isCharging;
}

std::string SystemComponent::getOSName() {
    return atom::system::getOperatingSystemInfo().osName;
}

std::string SystemComponent::getOSVersion() {
    return atom::system::getOperatingSystemInfo().osVersion;
}

std::string SystemComponent::getKernelVersion() {
    return atom::system::getOperatingSystemInfo().kernelVersion;
}

std::string SystemComponent::getArchitecture() {
    return atom::system::getOperatingSystemInfo().architecture;
}

void SystemComponent::makePidWatcher(const std::string &name) {
    if (m_pidWatchers.find(name) != m_pidWatchers.end()) {
        return;
    }
    m_pidWatchers[name] = std::make_shared<PidWatcher>();
}

bool SystemComponent::startPidWatcher(const std::string &name,
                                      const std::string &pid) {
    return m_pidWatchers[name]->Start(pid);
}

void SystemComponent::stopPidWatcher(const std::string &name) {
    m_pidWatchers[name]->Stop();
}

bool SystemComponent::switchPidWatcher(const std::string &name,
                                       const std::string &pid) {
    return m_pidWatchers[name]->Switch(pid);
}
void SystemComponent::setPidWatcherExitCallback(
    const std::string &name, const std::function<void()> &callback) {
    m_pidWatchers[name]->SetExitCallback(callback);
}

void SystemComponent::setPidWatcherMonitorFunction(
    const std::string &name, const std::function<void()> &callback,
    std::chrono::milliseconds interval) {
    m_pidWatchers[name]->SetMonitorFunction(callback, interval);
}

void SystemComponent::getPidByName(const std::string &name,
                                   const std::string &pid) {
    m_pidWatchers[name]->GetPidByName(pid);
}