/*
 * python_system.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: System module for Python scripting engine

**************************************************/

#include "pocketpy/include/pocketpy/bindings.h"
using namespace pkpy;

#include "atom/system/module/battery.hpp"
#include "atom/system/module/cpu.hpp"
#include "atom/system/module/disk.hpp"
#include "atom/system/module/memory.hpp"
#include "atom/system/module/os.hpp"
#include "atom/system/module/wifi.hpp"

#include "atom/system/system.hpp"

#include "atom/log/loguru.hpp"

namespace Lithium {
/*
struct Wrapped_Process
{
    PY_CLASS(Wrapped_Process, mod, Process)

    Atom::System::ProcessInfo value;

    Atom::System::ProcessInfo *_() { return &value; }

    Wrapped_Process() = default;
    Wrapped_Process(const Wrapped_Process &) = default;

    Wrapped_Process(Atom::System::ProcessInfo value)
        : value(value)
    {
    }

    static void _register(VM *vm, PyObject *mod, PyObject *type)
    {
        // enable default constructor and struct-like methods
        // if you don't use this, you must bind a `__new__` method as
constructor PY_STRUCT_LIKE(Wrapped_Process)

        // wrap field processId
        PY_FIELD(Wrapped_Process, "id", _, processID)
        // wrap field parentProcessID
        PY_FIELD(Wrapped_Process, "parent_id", _, parentProcessID)
        // wrap field basePriority
        PY_FIELD(Wrapped_Process, "priority", _, basePriority)
        // wrap field executableFile
        PY_FIELD(Wrapped_Process, "executable", _, executableFile)

        // __init__ method
        vm->bind(type, "__init__(self, id : int, parent_id : int, priority :
int, executable : str)", [](VM *vm, ArgsView args)
                 {
        Wrapped_Process& self = _py_cast<Wrapped_Process&>(vm, args[0]);
        self.value.processID = py_cast<int>(vm, args[1]);
        self.value.parentProcessID = py_cast<int>(vm, args[2]);
        self.value.basePriority = py_cast<int>(vm, args[3]);
        self.value.executableFile = py_cast<std::string>(vm, args[4]);
        return vm->None; });
    }
};
*/

void addSysModule(VM *vm) {
    PyObject *mod = vm->new_module("li_sys");
    vm->setattr(mod, "version", VAR("1.0.0"));
    vm->bind(mod, "get_cpu_usage() -> float",
             "get CPU usage, and return a float value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get CPU usage");
                 float cpu_usage = Atom::System::getCurrentCpuUsage();
                 if (cpu_usage < 0.0f) {
                     LOG_F(ERROR, "Failed to get cpu usage: {}", cpu_usage);
                 }
                 DLOG_F(INFO, "Cpu usage: {}", cpu_usage);
                 return py_var(vm, cpu_usage);
             });

    vm->bind(mod, "get_cpu_temperature() -> float",
             "get CPU temperature, and return a float value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get CPU temperature");
                 float cpu_temperature =
                     Atom::System::getCurrentCpuTemperature();
                 if (cpu_temperature < 0.0f) {
                     LOG_F(ERROR, "Failed to get cpu temperature: {}",
                           cpu_temperature);
                 }
                 DLOG_F(INFO, "Cpu temperature: {}", cpu_temperature);
                 return py_var(vm, cpu_temperature);
             });

    vm->bind(mod, "get_cpu_info() -> dict", "get CPU infomation in dict format",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get CPU infomation");
                 auto cpu_model = Atom::System::getCPUModel();
                 auto cpu_freq = Atom::System::getProcessorFrequency();
                 auto cpu_id = Atom::System::getProcessorIdentifier();
                 auto cpu_package = Atom::System::getNumberOfPhysicalPackages();
                 auto cpu_core = Atom::System::getNumberOfPhysicalCPUs();

                 Dict d(vm);
                 d.set(py_var(vm, "model"), py_var(vm, cpu_model));
                 d.set(py_var(vm, "frequency"), py_var(vm, cpu_freq));
                 d.set(py_var(vm, "id"), py_var(vm, cpu_id));
                 d.set(py_var(vm, "package"), py_var(vm, cpu_package));
                 d.set(py_var(vm, "core"), py_var(vm, cpu_core));
                 return py_var(vm, std::move(d));
             });

    vm->bind(mod, "get_cpu_model() -> str",
             "get CPU model, and return a string value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get CPU model");
                 std::string cpu_model = Atom::System::getCPUModel();
                 if (cpu_model.empty()) {
                     LOG_F(ERROR, "Failed to get cpu model: {}", cpu_model);
                 }
                 DLOG_F(INFO, "Cpu model: {}", cpu_model);
                 return py_var(vm, cpu_model);
             });

    vm->bind(mod, "get_memory_usage() -> float",
             "get memory usage, and return a float value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get memory usage");
                 float memory_usage = Atom::System::getMemoryUsage();
                 if (memory_usage < 0.0f) {
                     LOG_F(ERROR, "Failed to get memory usage: {}",
                           memory_usage);
                 }
                 DLOG_F(INFO, "Memory usage: {}", memory_usage);
                 return py_var(vm, memory_usage);
             });

    vm->bind(mod, "get_memory_total() -> float",
             "get total memory size, and return a float value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get memory total");
                 float memory_total = Atom::System::getTotalMemorySize();
                 if (memory_total < 0.0f) {
                     LOG_F(ERROR, "Failed to get memory total: {}",
                           memory_total);
                 }
                 DLOG_F(INFO, "Memory total: {}", memory_total);
                 return py_var(vm, memory_total);
             });

    vm->bind(mod, "get_available_memory() -> float",
             "get available memory size, and return a float value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get available memory");
                 float available_memory =
                     Atom::System::getAvailableMemorySize();
                 if (available_memory < 0.0f) {
                     LOG_F(ERROR, "Failed to get available memory: {}",
                           available_memory);
                 }
                 DLOG_F(INFO, "Available memory: {}", available_memory);
                 return py_var(vm, available_memory);
             });

    vm->bind(
        mod, "get_memory_info() -> dict",
        "get memory usage, and return a dict value", [](VM *vm, ArgsView args) {
            DLOG_F(INFO, "Get memory info");
            auto total_memory = Atom::System::getTotalMemorySize();
            auto available_memory = Atom::System::getAvailableMemorySize();
            auto virtual_memory_max = Atom::System::getVirtualMemoryMax();
            auto virtual_memory_used = Atom::System::getVirtualMemoryUsed();
            auto swap_memory_total = Atom::System::getSwapMemoryTotal();
            auto swap_memory_used = Atom::System::getSwapMemoryUsed();
            Dict d(vm);
            d.set(py_var(vm, "total"), py_var(vm, total_memory));
            d.set(py_var(vm, "available"), py_var(vm, available_memory));
            d.set(py_var(vm, "virtual_max"), py_var(vm, virtual_memory_max));
            d.set(py_var(vm, "virtual_used"), py_var(vm, virtual_memory_used));
            d.set(py_var(vm, "swap_total"), py_var(vm, swap_memory_total));
            d.set(py_var(vm, "swap_used"), py_var(vm, swap_memory_used));
            return py_var(vm, std::move(d));
        });

    vm->bind(mod, "get_disk_usage() -> dict",
             "get disk usage, and return a dict value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get disk usage");
                 std::vector<std::pair<std::string, float>> disk_usage =
                     Atom::System::getDiskUsage();
                 if (disk_usage.empty()) {
                     // LOG_F(ERROR, "Failed to get disk usage: {}",
                     // disk_usage);
                 }
                 Dict d(vm);
                 for (auto &item : disk_usage) {
                     DLOG_F(INFO, "Disk usage: {} {}", item.first, item.second);
                     d.set(py_var(vm, item.first), py_var(vm, item.second));
                 }
                 return py_var(vm, std::move(d));
             });

    vm->bind(mod, "get_disk_model(name : str) -> str",
             "get disk model, and return a string value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get disk model");
                 PyObject *name_obj = args[0];
                 Str &name = py_cast<Str &>(vm, name_obj);

                 std::string driver_model =
                     Atom::System::getDriveModel(name.c_str());
                 if (driver_model.empty()) {
                     LOG_F(ERROR, "Failed to get disk model: {}", driver_model);
                 }
                 DLOG_F(INFO, "Disk model: {}", driver_model);
                 return py_var(vm, driver_model);
             });

    vm->bind(mod, "get_disk_models() -> dict",
             "get disk models, and return a dict value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get disk models");
                 std::vector<std::pair<std::string, std::string>> disk_models =
                     Atom::System::getStorageDeviceModels();
                 if (disk_models.empty()) {
                     // LOG_F(ERROR, "Failed to get disk models: {}",
                     // disk_models);
                 }
                 Dict d(vm);
                 for (auto &item : disk_models) {
                     DLOG_F(INFO, "Disk model: {} {}", item.first, item.second);
                     d.set(py_var(vm, item.first), py_var(vm, item.second));
                 }
                 return py_var(vm, std::move(d));
             });

    vm->bind(mod, "is_root() -> bool",
             "check if the current process is running as root, and return a "
             "bool value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO,
                        "Check if the current process is running as "
                        "root");
                 bool is_root = Atom::System::IsRoot();
                 return py_var(vm, is_root);
             });

    vm->bind(mod, "get_current_username() -> str",
             "get current username, and return a string value",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Get current username");
                 std::string current_username =
                     Atom::System::GetCurrentUsername();
                 if (current_username.empty()) {
                     LOG_F(ERROR, "Failed to get current username: {}",
                           current_username);
                 }
                 DLOG_F(INFO, "Current username: {}", current_username);
                 return py_var(vm, current_username);
             });

    vm->bind(mod, "shutdown() -> bool", "shutdown the system",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Shutdown the system");
                 Atom::System::Shutdown();
                 return py_var(vm, true);
             });

    vm->bind(mod, "reboot() -> bool", "reboot the system",
             [](VM *vm, ArgsView args) {
                 DLOG_F(INFO, "Reboot the system");
                 Atom::System::Reboot();
                 return py_var(vm, true);
             });

    vm->bind(mod, "check_duplicate_process(name : str) -> bool",
             "check if the process is running, and return a bool value",
             [](VM *vm, ArgsView args) {
                 PyObject *name_obj = args[0];
                 Str &name = py_cast<Str &>(vm, name_obj);

                 bool is_duplicate =
                     Atom::System::CheckDuplicateProcess(name.c_str());
                 if (is_duplicate) {
                     LOG_F(ERROR, "Failed to check duplicate process: {}",
                           is_duplicate);
                 }
                 DLOG_F(INFO, "Duplicate process: {}", is_duplicate);
                 return py_var(vm, is_duplicate);
             });

    vm->bind(mod, "is_process_running(name : str) -> bool",
             "check if the process is running, and return a bool value",
             [](VM *vm, ArgsView args) {
                 PyObject *name_obj = args[0];
                 Str &name = py_cast<Str &>(vm, name_obj);

                 bool is_running = Atom::System::isProcessRunning(name.c_str());
                 DLOG_F(INFO, "Process running: {}",
                        is_running ? "true" : "false");
                 return py_var(vm, is_running);
             });

    vm->bind(mod, "get_process_by_name(name : str) -> dict",
             "get process info by name, and return a dict value",
             [](VM *vm, ArgsView args) {
                 PyObject *name_obj = args[0];
                 Str &name = py_cast<Str &>(vm, name_obj);
                 Atom::System::ProcessInfo info;
                 Dict d(vm);
                 if (Atom::System::GetProcessInfoByName(name.c_str(), info)) {
                     d.set(VAR("id"), VAR(info.processID));
                     d.set(VAR("parent_id"), VAR(info.parentProcessID));
                     d.set(VAR("priority"), VAR(info.basePriority));
                     d.set(VAR("executable"), VAR(info.executableFile));
                 } else {
                     LOG_F(ERROR, "Failed to get process info!");
                 }
                 return py_var(vm, std::move(d));
             });

    vm->bind(mod, "get_process_by_id(id : int) -> dict",
             "get process info by id, and return a dict value",
             [](VM *vm, ArgsView args) {
                 int id = py_cast<int>(vm, args[0]);
                 Atom::System::ProcessInfo info;
                 Dict d(vm);
                 if (Atom::System::GetProcessInfoByID(id, info)) {
                     d.set(VAR("id"), VAR(info.processID));
                     d.set(VAR("parent_id"), VAR(info.parentProcessID));
                     d.set(VAR("priority"), VAR(info.basePriority));
                     d.set(VAR("executable"), VAR(info.executableFile));
                 } else {
                     LOG_F(ERROR, "Failed to get process info!");
                 }
                 return py_var(vm, std::move(d));
             });
}
}  // namespace Lithium
