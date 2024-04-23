/*
 * _script.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-23

Description: Carbon binding for Atom-System

**************************************************/

#ifndef ATOM_SYSTEM_SCRIPT_HPP
#define ATOM_SYSTEM_SCRIPT_HPP

#include "carbon/carbon.hpp"

#include "module/battery.hpp"
#include "module/cpu.hpp"
#include "module/disk.hpp"
#include "module/gpu.hpp"
#include "module/memory.hpp"
#include "module/os.hpp"
#include "module/wifi.hpp"

#include "command.hpp"
#include "crash.hpp"
#include "crash_quotes.hpp"
#include "lregistry.hpp"
#include "os.hpp"
#include "pidwatcher.hpp"
#include "process.hpp"
#include "register.hpp"
#include "storage.hpp"
#include "system.hpp"
#include "user.hpp"

using namespace Atom::System;

namespace Atom::_Script::System {
/**
 * Adds the String Methods to the given Carbon m.
 */
Carbon::ModulePtr bootstrap(
    Carbon::ModulePtr m = std::make_shared<Carbon::Module>()) {

    m->add(user_type<BatteryInfo>(), "BatteryInfo");
    //m->add(Carbon::constructor<BatteryInfo>(), "BatteryInfo");
    m->add(Carbon::fun(&BatteryInfo::isBatteryPresent), "is_battery_present");
    m->add(Carbon::fun(&BatteryInfo::batteryLifeTime), "battery_life_time");
    m->add(Carbon::fun(&BatteryInfo::batteryLifePercent),
           "battery_life_percent");
    m->add(Carbon::fun(&BatteryInfo::batteryFullLifeTime),
           "battery_full_life_time");
    m->add(Carbon::fun(&BatteryInfo::energyNow), "energy_now");
    m->add(Carbon::fun(&BatteryInfo::energyFull), "energy_full");
    m->add(Carbon::fun(&BatteryInfo::energyDesign), "energy_design");
    m->add(Carbon::fun(&BatteryInfo::currentNow), "current_now");
    m->add(Carbon::fun(&BatteryInfo::voltageNow), "voltage_now");
    m->add(Carbon::fun(&BatteryInfo::isCharging), "is_charging");

    m->add(Carbon::fun(&getBatteryInfo), "get_battery_info");

    m->add(Carbon::fun(&getCurrentCpuUsage), "get_current_cpu_usage");
    m->add(Carbon::fun(&getCurrentCpuTemperature),
           "get_current_cpu_temperature");
    m->add(Carbon::fun(&getCPUModel), "get_cpu_model");
    m->add(Carbon::fun(getProcessorIdentifier), "get_processor_identifier");
    m->add(Carbon::fun(&getProcessorFrequency), "get_processor_frequency");
    m->add(Carbon::fun(&getNumberOfPhysicalCPUs),
           "get_number_of_physical_cpus");
    m->add(Carbon::fun(&getNumberOfPhysicalPackages),
           "get_number_of_physical_packages");

    m->add(Carbon::fun(&getDiskUsage), "get_disk_usage");
    m->add(Carbon::fun(&getDriveModel), "get_drive_model");
    m->add(Carbon::fun(&getAvailableDrives), "get_available_drives");
    m->add(Carbon::fun(&getStorageDeviceModels), "get_storage_device_models");
    m->add(Carbon::fun(&calculateDiskUsagePercentage),
           "calculate_disk_usage_percentage");

    m->add(Carbon::fun(&getGPUInfo), "get_gpu_info");

    m->add(user_type<MemoryInfo::MemorySlot>(), "MemorySlot");
    //m->add(Carbon::constructor<MemoryInfo::MemorySlot>(), "MemorySlot");
    m->add(
        Carbon::constructor<MemoryInfo::MemorySlot(
            const std::string &, const std::string &, const std::string &)>(),
        "size");
    m->add(Carbon::fun(&MemoryInfo::MemorySlot::capacity), "capacity");
    m->add(Carbon::fun(&MemoryInfo::MemorySlot::type), "type");
    m->add(Carbon::fun(&MemoryInfo::MemorySlot::clockSpeed), "speed");

    m->add(user_type<MemoryInfo>(), "MemoryInfo");
    //m->add(Carbon::constructor<MemoryInfo>(), "MemoryInfo");
    m->add(Carbon::fun(&MemoryInfo::slots), "slots");
    m->add(Carbon::fun(&MemoryInfo::swapMemoryTotal), "swap_memory_total");
    m->add(Carbon::fun(&MemoryInfo::swapMemoryUsed), "swap_memory_used");
    m->add(Carbon::fun(&MemoryInfo::virtualMemoryMax), "virtual_memory_max");
    m->add(Carbon::fun(&MemoryInfo::virtualMemoryUsed), "virtual_memory_used");

    m->add(Carbon::fun(&getMemoryUsage), "get_memory_usage");
    m->add(Carbon::fun(&getPhysicalMemoryInfo), "get_physical_memory_info");
    m->add(Carbon::fun(&getVirtualMemoryMax), "get_virtual_memory_max");
    m->add(Carbon::fun(&getVirtualMemoryUsed), "get_virtual_memory_used");
    m->add(Carbon::fun(&getSwapMemoryTotal), "get_swap_memory_total");
    m->add(Carbon::fun(&getSwapMemoryUsed), "get_swap_memory_used");
    m->add(Carbon::fun(&getTotalMemorySize), "get_total_memory_size");
    m->add(Carbon::fun(&getAvailableMemorySize), "get_available_memory_size");

    m->add(user_type<OperatingSystemInfo>(), "OperatingSystemInfo");
    //m->add(Carbon::constructor<OperatingSystemInfo>(), "OperatingSystemInfo");
    m->add(Carbon::fun(&OperatingSystemInfo::osName), "os_name");
    m->add(Carbon::fun(&OperatingSystemInfo::osVersion), "os_version");
    m->add(Carbon::fun(&OperatingSystemInfo::kernelVersion), "kernel_version");
    m->add(Carbon::fun(&OperatingSystemInfo::architecture), "architecture");
    m->add(Carbon::fun(&OperatingSystemInfo::compiler), "compiler");
    m->add(Carbon::fun(&OperatingSystemInfo::toJson), "to_json");

    m->add(Carbon::fun(&getOperatingSystemInfo), "get_operating_system_info");

    m->add(Carbon::fun(&getCurrentWifi), "get_current_wifi");
    m->add(Carbon::fun(&getCurrentWiredNetwork), "get_current_wired_network");
    m->add(Carbon::fun(&isHotspotConnected), "is_hotspot_connected");
    m->add(Carbon::fun(&getHostIPs), "get_host_ips");

    m->add(user_type<ProcessHandle>(), "ProcessHandle");
    //m->add(Carbon::constructor<ProcessHandle>(), "ProcessHandle");

    m->add(
        Carbon::fun<std::string (*)(const std::string &, bool,
                                    std::function<void(const std::string &)>)>(
            &executeCommand),
        "execute_command");
    m->add(Carbon::fun<ProcessHandle (*)(const std::string &)>(&executeCommand),
           "execute_command");
    m->add(Carbon::fun(&executeCommands), "execute_commands");
    m->add(Carbon::fun(&killProcess), "kill_process");
    m->add(Carbon::fun(&executeCommandWithEnv), "execute_command_with_env");
    m->add(Carbon::fun(&executeCommandWithStatus),
           "execute_command_with_status");

    m->add(user_type<Quote>(), "Quote");
    m->add(
        Carbon::constructor<Quote(const std::string &, const std::string &)>(),
        "Quote");
    m->add(Carbon::fun(&Quote::getAuthor), "author");
    m->add(Carbon::fun(&Quote::getText), "text");

    m->add(user_type<QuoteManager>(), "QuoteManager");
    //m->add(Carbon::constructor<QuoteManager>(), "QuoteManager");
#if ENABLE_DEBUG
    m->add(Carbon::fun(&QuoteManager::displayQuotes), "display_quotes");
#endif
    m->add(Carbon::fun(&QuoteManager::clearQuotes), "clear_quotes");
    m->add(Carbon::fun(&QuoteManager::saveQuotesToFile), "save_quotes_to_file");
    m->add(Carbon::fun(&QuoteManager::loadQuotesFromFile),
           "load_quotes_from_file");
    m->add(Carbon::fun(&QuoteManager::shuffleQuotes), "shuffle_quotes");
    m->add(Carbon::fun(&QuoteManager::addQuote), "add_quote");
    m->add(Carbon::fun(&QuoteManager::removeQuote), "remove_quote");
    m->add(Carbon::fun(&QuoteManager::searchQuotes), "search_quotes");
    m->add(Carbon::fun(&QuoteManager::getRandomQuote), "get_random_quote");
    m->add(Carbon::fun(&QuoteManager::filterQuotesByAuthor),
           "filter_quotes_by_author");

    m->add(Carbon::fun(&saveCrashLog), "save_crash_log");

    m->add(user_type<Registry>(), "Registry");

    //m->add(Carbon::constructor<Registry()>(), "Registry");

    m->add(Carbon::fun(&Registry::loadRegistryFromFile),
           "loadRegistryFromFile");
    m->add(Carbon::fun(&Registry::createKey), "createKey");
    m->add(Carbon::fun(&Registry::deleteKey), "deleteKey");
    m->add(Carbon::fun(&Registry::setValue), "setValue");
    m->add(Carbon::fun(&Registry::getValue), "getValue");
    m->add(Carbon::fun(&Registry::deleteValue), "deleteValue");
    m->add(Carbon::fun(&Registry::backupRegistryData), "backupRegistryData");
    m->add(Carbon::fun(&Registry::restoreRegistryData), "restoreRegistryData");
    m->add(Carbon::fun(&Registry::keyExists), "keyExists");
    m->add(Carbon::fun(&Registry::valueExists), "valueExists");
    m->add(Carbon::fun(&Registry::getValueNames), "getValueNames");

    m->add(user_type<Utsname>(), "Utsname");
    //m->add(Carbon::constructor<Utsname>(), "Utsname");
    m->add(Carbon::fun(&Utsname::nodename), "nodename");
    m->add(Carbon::fun(&Utsname::sysname), "sysname");
    m->add(Carbon::fun(&Utsname::release), "release");
    m->add(Carbon::fun(&Utsname::version), "version");
    m->add(Carbon::fun(&Utsname::machine), "machine");

    m->add(Carbon::fun(&walk), "walk");
    m->add(Carbon::fun(&jwalk), "jwalk");
    m->add(Carbon::fun(&fwalk), "fwalk");
    m->add(Carbon::fun(&Environ), "Environ");
    m->add(Carbon::fun(&ctermid), "ctermid");
    m->add(Carbon::fun(&getpriority), "getpriority");
    m->add(Carbon::fun(&getlogin), "getlogin");
    m->add(Carbon::fun(&uname), "uname");

    m->add(user_type<PidWatcher>(), "PidWatcher");
    //m->add(Carbon::constructor<PidWatcher>(), "PidWatcher");
    m->add(Carbon::fun(&PidWatcher::SetExitCallback), "set_exit_callback");
    m->add(Carbon::fun(&PidWatcher::SetMonitorFunction),
           "set_monitor_function");
    m->add(Carbon::fun(&PidWatcher::Start), "start");
    m->add(Carbon::fun(&PidWatcher::Stop), "stop");
    m->add(Carbon::fun(&PidWatcher::Switch), "switch");
    m->add(Carbon::fun(&PidWatcher::GetPidByName), "get_pid_by_name");

    m->add(user_type<ProcessManager>(), "ProcessManager");
    //m->add(Carbon::constructor<ProcessManager>(), "ProcessManager");
    m->add(Carbon::fun(&ProcessManager::createProcess), "create_process");
    m->add(Carbon::fun(&ProcessManager::terminateProcess), "terminate_process");
    m->add(Carbon::fun(&ProcessManager::terminateProcessByName),
           "terminate_process_by_name");
    m->add(Carbon::fun(&ProcessManager::hasProcess), "has_process");
    m->add(Carbon::fun(&ProcessManager::getProcessOutput),
           "get_process_output");
    m->add(Carbon::fun(&ProcessManager::runScript), "run_script");
    m->add(Carbon::fun(&ProcessManager::waitForCompletion),
           "wait_for_completion");
    m->add(Carbon::fun(&ProcessManager::getRunningProcesses),
           "get_running_processes");

#ifdef _WIN32

    m->add(Carbon::fun(&getRegistrySubKeys), "get_registry_sub_keys");
    m->add(Carbon::fun(&getRegistryValues), "get_registry_values");
    m->add(Carbon::fun(&findRegistryKey), "find_registry_key");
    m->add(Carbon::fun(&findRegistryValue), "find_registry_value");
    m->add(Carbon::fun(&recursivelyEnumerateRegistrySubKeys),
           "recursively_enumerate_registry_sub_keys");
    m->add(Carbon::fun(&exportRegistry), "export_registry");
    m->add(Carbon::fun(&backupRegistry), "backup_registry");
    m->add(Carbon::fun(&modifyRegistryValue), "modify_registry_value");
    m->add(Carbon::fun(&deleteRegistrySubKey), "delete_registry_sub_key");
    m->add(Carbon::fun(&deleteRegistryValue), "delete_registry_value");

#endif

    m->add(user_type<StorageMonitor>(), "StorageMonitor");
    //m->add(Carbon::constructor<StorageMonitor>(), "StorageMonitor");
    m->add(Carbon::fun(&StorageMonitor::registerCallback), "register_callback");
    m->add(Carbon::fun(&StorageMonitor::startMonitoring), "start_monitoring");
    m->add(Carbon::fun(&StorageMonitor::stopMonitoring), "stop_monitoring");
    m->add(Carbon::fun(&StorageMonitor::triggerCallbacks), "trigger_callbacks");

    m->add(Carbon::fun(&checkSoftwareInstalled), "check_software_installed");
    m->add(Carbon::fun(&checkDuplicateProcess), "check_duplicate_process");
    m->add(Carbon::fun(&getProcessInfoByName), "get_process_info_by_name");
    m->add(Carbon::fun(&getProcessInfoByID), "get_process_info_by_id");
    m->add(Carbon::fun(&GetProcessDetails), "get_process_details");
    m->add(Carbon::fun(&getProcessInfo), "get_process_info");
    m->add(Carbon::fun(&GetAllProcesses), "get_all_processes");
    m->add(Carbon::fun(&GetSelfProcessInfo), "get_self_process_info");
    m->add(Carbon::fun(&isProcessRunning), "is_process_running");
    m->add(Carbon::fun(&isRoot), "is_root");
    m->add(Carbon::fun(&reboot), "reboot");
    m->add(Carbon::fun(&shutdown), "shutdown");

    m->add(Carbon::fun(&getUsername), "get_username");
    m->add(Carbon::fun(&getUserId), "get_user_id");
    m->add(Carbon::fun(&getGroupId), "get_group_id");
    m->add(Carbon::fun(&getHomeDirectory), "get_home_directory");
    m->add(Carbon::fun(&getLoginShell), "get_login_shell");
    m->add(Carbon::fun(&getUserGroups), "get_user_groups");
    m->add(Carbon::fun(&getHostname), "get_hostname");

    return m;
}
}  // namespace Atom::_Script::System


#endif

