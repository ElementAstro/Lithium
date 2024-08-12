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

#include "command.hpp"
#include "crash.hpp"
#include "macro.hpp"
#include "pidwatcher.hpp"
#include "platform.hpp"
#include "user.hpp"

using namespace atom::system;

SystemComponent::SystemComponent(const std::string &name) : Component(name) {
    DLOG_F(INFO, "SystemComponent::SystemComponent");
    def("run_commands", &executeCommands, "os",
        "Run a list of system commands");
    def("run_command_env", &executeCommandWithEnv, "os",
        "Run a system command with environment variables");
    def("run_command_status", &executeCommandWithStatus, "os",
        "Run a system command and get its status");
    def("getlogin", &getlogin, "os", "Get current user name");

    def("user_group", &getUserGroups, "user", "Get current user groups");
    def("user_id", &getUserId, "user", "Get current user ID");
    def("user_host", &getHostname, "user", "Get current user hostname");
    def("user_name", &getUsername, "user", "Get current user name");
    def("user_home", &getHomeDirectory, "user",
        "Get current user home directory");

    def("user_shell", &getLoginShell, "user", "Get current user login shell");

    def("user_groups", &getUserGroups, "user", "Get current user groups");

    addVariable("platform", platform, "Platform", "os_name", "os");
    addVariable("architecture", architecture, "Architecture", "os_arch", "os");
    addVariable("os_version", os_version, "OS Version", "kernel_version", "os");
    addVariable("compiler", compiler, "Compiler", "builder", "os");

    def("make_pidwatcher", &SystemComponent::makePidWatcher,
        PointerSentinel(this), "os", "Make a PID watcher");
    def("start_watcher", &SystemComponent::startPidWatcher,
        PointerSentinel(this), "os", "Start a PID watcher");
    def("stop_watcher", &SystemComponent::stopPidWatcher, PointerSentinel(this),
        "os", "Stop a PID watcher");
    def("switch_watcher", &SystemComponent::switchPidWatcher,
        PointerSentinel(this), "os", "Switch a PID watcher");
    def("set_watcher_exit", &SystemComponent::setPidWatcherExitCallback,
        PointerSentinel(this), "os", "Set a PID watcher exit callback");
    def("set_watcher_monitor", &SystemComponent::setPidWatcherMonitorFunction,
        PointerSentinel(this), "os", "Set a PID watcher monitor callback");

#if ENABLE_REGISTRY_SUPPORT
    def("get_registry_subkeys", &getRegistrySubKeys, "os",
        "Get registry subkeys");
    def("get_registry_values", &getRegistryValues, "os", "Get registry values");
    def("delete_registry_subkey", &deleteRegistrySubKey, "os",
        "Delete registry subkey");
    def("modify_registry_value", &modifyRegistryValue, "os",
        "Modify registry value");
    def("recursively_enumerate_registry_subkeys",
        &recursivelyEnumerateRegistrySubKeys, "os",
        "Recursively enumerate registry subkeys");
    def("find_registry_key", &findRegistryKey, "os", "Find registry key");
    def("find_registry_value", &findRegistryValue, "os", "Find registry value");
    def("backup_registry", &backupRegistry, "os", "Backup registry");
    def("export_registry", &exportRegistry, "os", "Export registry");
    def("delete_registry_value", &deleteRegistryValue, "os",
        "Delete registry value");
#endif

    def("save_crashreport", &saveCrashLog, "os", "Save crash report");
}

SystemComponent::~SystemComponent() {
    DLOG_F(INFO, "SystemComponent::~SystemComponent");
}

bool SystemComponent::initialize() { return true; }

bool SystemComponent::destroy() { return true; }

void SystemComponent::makePidWatcher(const std::string &name) {
    if (m_pidWatchers.find(name) != m_pidWatchers.end()) {
        return;
    }
    m_pidWatchers[name] = std::make_shared<PidWatcher>();
}

auto SystemComponent::startPidWatcher(const std::string &name,
                                      const std::string &pid) -> bool {
    return m_pidWatchers[name]->start(pid);
}

void SystemComponent::stopPidWatcher(const std::string &name) {
    m_pidWatchers[name]->stop();
}

bool SystemComponent::switchPidWatcher(const std::string &name,
                                       const std::string &pid) {
    return m_pidWatchers[name]->Switch(pid);
}
void SystemComponent::setPidWatcherExitCallback(
    const std::string &name, const std::function<void()> &callback) {
    m_pidWatchers[name]->setExitCallback(callback);
}

void SystemComponent::setPidWatcherMonitorFunction(
    const std::string &name, const std::function<void()> &callback,
    std::chrono::milliseconds interval) {
    m_pidWatchers[name]->setMonitorFunction(callback, interval);
}

void SystemComponent::getPidByName(const std::string &name,
                                   const std::string &pid) {
    ATOM_UNUSED_RESULT(m_pidWatchers[name]->getPidByName(pid));
}
