/*
 * _pybind.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Python Binding of Atom-System Module

**************************************************/

#include <pybind11/pybind11.h>

#include "command.hpp"
#include "crash_quotes.hpp"
#include "lregistry.hpp"
#include "os.hpp"
#include "pidwatcher.hpp"
#include "user.hpp"

namespace py = pybind11;

using namespace atom::system;

PYBIND11_EMBEDDED_MODULE(atom_system, m) {
    pybind11::class_<BatteryInfo>(m, "BatteryInfo")
        .def_readwrite("isBatteryPresent", &BatteryInfo::isBatteryPresent)
        .def_readwrite("isCharging", &BatteryInfo::isCharging)
        .def_readwrite("batteryLifePercent", &BatteryInfo::batteryLifePercent)
        .def_readwrite("batteryLifeTime", &BatteryInfo::batteryLifeTime)
        .def_readwrite("batteryFullLifeTime", &BatteryInfo::batteryFullLifeTime)
        .def_readwrite("energyNow", &BatteryInfo::energyNow)
        .def_readwrite("energyFull", &BatteryInfo::energyFull)
        .def_readwrite("energyDesign", &BatteryInfo::energyDesign)
        .def_readwrite("voltageNow", &BatteryInfo::voltageNow)
        .def_readwrite("currentNow", &BatteryInfo::currentNow);

    m.def("getBatteryInfo", &getBatteryInfo);

    m.def("get_current_cpu_usage", &getCurrentCpuUsage,
          "Get the current CPU usage as a percentage.");
    m.def("get_current_cpu_temperature", &getCurrentCpuTemperature,
          "Get the current CPU temperature in degrees Celsius.");
    m.def("get_cpu_model", &getCPUModel, "Get the model of the CPU.");
    m.def("get_processor_identifier", &getProcessorIdentifier,
          "Get the identifier of the CPU processor.");
    m.def("get_processor_frequency", &getProcessorFrequency,
          "Get the frequency of the CPU in GHz.");
    m.def("get_number_of_physical_packages", &getNumberOfPhysicalPackages,
          "Get the number of physical CPU packages.");
    m.def("get_number_of_physical_cpus", &getNumberOfPhysicalCPUs,
          "Get the number of physical CPUs.");

    m.def("getDiskUsage", &getDiskUsage);
    m.def("getDriveModel", &getDriveModel);
    m.def("getStorageDeviceModels", &getStorageDeviceModels);
    m.def("getAvailableDrives", &getAvailableDrives);
    m.def("calculateDiskUsagePercentage", &calculateDiskUsagePercentage);

    m.def("get_memory_usage", &getMemoryUsage);
    m.def("get_total_memory_size", &getTotalMemorySize);
    m.def("get_available_memory_size", &getAvailableDrives);
    m.def("get_physical_memory_info", &getPhysicalMemoryInfo);
    m.def("get_virtual_memory_max", &getVirtualMemoryMax);
    m.def("get_virtual_memory_used", &getVirtualMemoryUsed);
    m.def("get_swap_memory_total", &getSwapMemoryTotal);
    m.def("get_swap_memory_used", &getSwapMemoryUsed);

    py::class_<OperatingSystemInfo>(m, "OperatingSystemInfo")
        .def(py::init<>())
        .def("toJson", &OperatingSystemInfo::toJson);

    m.def("getOperatingSystemInfo", &getOperatingSystemInfo);

    m.def("getCurrentWifi", &getCurrentWifi, "Get the current WiFi name");
    m.def("getCurrentWiredNetwork", &getCurrentWiredNetwork,
          "Get the current wired network name");
    m.def("isHotspotConnected", &isHotspotConnected,
          "Check if hotspot is connected");

    // m.def("executeCommand", &executeCommand, "Execute a command and return
    // the output as a string");
    m.def("executeCommands", &executeCommands, "Execute a list of commands");
    // m.def("executeCommand", &executeCommand, "Execute a command and return
    // the process handle");
    m.def("killProcess", &killProcess, "Kill a process");
    m.def("executeCommandWithEnv", &executeCommandWithEnv,
          "Execute a command with environment variables and return the output "
          "as a string");
    m.def("executeCommandWithStatus", &executeCommandWithStatus,
          "Execute a command and return the output along with the exit status");

    py::class_<Quote>(m, "Quote")
        .def(py::init<const std::string&, const std::string&>())
        .def("getText", &Quote::getText)
        .def("getAuthor", &Quote::getAuthor);

    py::class_<QuoteManager>(m, "QuoteManager")
        .def("addQuote", &QuoteManager::addQuote)
        .def("removeQuote", &QuoteManager::removeQuote)
#ifdef DEBUG
        .def("displayQuotes", &QuoteManager::displayQuotes)
#endif
        .def("shuffleQuotes", &QuoteManager::shuffleQuotes)
        .def("clearQuotes", &QuoteManager::clearQuotes)
        .def("loadQuotesFromFile", &QuoteManager::loadQuotesFromFile)
        .def("saveQuotesToFile", &QuoteManager::saveQuotesToFile)
        .def("searchQuotes", &QuoteManager::searchQuotes)
        .def("filterQuotesByAuthor", &QuoteManager::filterQuotesByAuthor)
        .def("getRandomQuote", &QuoteManager::getRandomQuote);

    py::class_<Registry>(m, "Registry")
        .def(py::init<>())
        .def("loadRegistryFromFile", &Registry::loadRegistryFromFile)
        .def("createKey", &Registry::createKey)
        .def("deleteKey", &Registry::deleteKey)
        .def("setValue", &Registry::setValue)
        .def("getValue", &Registry::getValue)
        .def("deleteValue", &Registry::deleteValue)
        .def("backupRegistryData", &Registry::backupRegistryData)
        .def("restoreRegistryData", &Registry::restoreRegistryData)
        .def("keyExists", &Registry::keyExists)
        .def("valueExists", &Registry::valueExists)
        .def("getValueNames", &Registry::getValueNames);

    py::class_<Utsname>(m, "Utsname")
        .def(py::init<>())
        .def_readwrite("sysname", &Utsname::sysname)
        .def_readwrite("nodename", &Utsname::nodename)
        .def_readwrite("release", &Utsname::release)
        .def_readwrite("version", &Utsname::version)
        .def_readwrite("machine", &Utsname::machine);

    m.def("walk", &walk);
    m.def("jwalk", &jwalk);
    m.def("fwalk", &fwalk);
    m.def("Environ", &Environ);
    m.def("ctermid", &atom::system::ctermid);
    m.def("getpriority", &getpriority);
    m.def("getlogin", &atom::system::getlogin);
    m.def("uname", &uname);

    py::class_<PidWatcher>(m, "PidWatcher")
        .def(py::init<>())
        .def("SetExitCallback", &PidWatcher::SetExitCallback)
        .def("SetMonitorFunction", &PidWatcher::SetMonitorFunction)
        .def("GetPidByName", &PidWatcher::GetPidByName)
        .def("Start", &PidWatcher::Start)
        .def("Stop", &PidWatcher::Stop)
        .def("Switch", &PidWatcher::Switch);

    m.def("getUserGroups", &getUserGroups);
    m.def("getUsername", &getUsername);
    m.def("getHostname", &getHostname);
    m.def("getUserId", &getUserId);
    m.def("getGroupId", &getGroupId);
    m.def("getHomeDirectory", &getHomeDirectory);
    m.def("getLoginShell", &getLoginShell);
}
