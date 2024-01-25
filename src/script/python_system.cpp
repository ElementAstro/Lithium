/*
 * python_system.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: System module for Python scripting engine

**************************************************/

#include "python.hpp"

#include "atom/system/system.hpp"

#include "atom/log/loguru.hpp"

namespace Lithium
{
    /*
    struct Wrapped_Process
    {
        PY_CLASS(Wrapped_Process, m_systemModule, Process)

        Atom::System::ProcessInfo value;

        Atom::System::ProcessInfo *_() { return &value; }

        Wrapped_Process() = default;
        Wrapped_Process(const Wrapped_Process &) = default;

        Wrapped_Process(Atom::System::ProcessInfo value)
            : value(value)
        {
        }

        static void _register(VM *vm, pkpy::PyObject *mod, pkpy::PyObject *type)
        {
            // enable default constructor and struct-like methods
            // if you don't use this, you must bind a `__new__` method as constructor
            PY_STRUCT_LIKE(Wrapped_Process)

            // wrap field processId
            PY_FIELD(Wrapped_Process, "id", _, processID)
            // wrap field parentProcessID
            PY_FIELD(Wrapped_Process, "parent_id", _, parentProcessID)
            // wrap field basePriority
            PY_FIELD(Wrapped_Process, "priority", _, basePriority)
            // wrap field executableFile
            PY_FIELD(Wrapped_Process, "executable", _, executableFile)

            // __init__ method
            vm->bind(type, "__init__(self, id : int, parent_id : int, priority : int, executable : str)", [](pkpy::VM *vm, pkpy::ArgsView args)
                     {
            Wrapped_Process& self = _pkpy::py_cast<Wrapped_Process&>(vm, args[0]);
            self.value.processID = pkpy::py_cast<int>(vm, args[1]);
            self.value.parentProcessID = pkpy::py_cast<int>(vm, args[2]);
            self.value.basePriority = pkpy::py_cast<int>(vm, args[3]);
            self.value.executableFile = pkpy::py_cast<std::string>(vm, args[4]);
            return vm->None; });
        }
    };
    */
    

    void PyScriptManager::InjectSystemModule()
    {
        vm->bind(m_systemModule, "get_cpu_usage() -> float",
                "get CPU usage, and return a float value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     float cpu_usage = Atom::System::GetCpuUsage();
                     if (cpu_usage < 0.0f)
                     {
                         LOG_F(ERROR, "Failed to get cpu usage: {}", cpu_usage);
                     }
                     DLOG_F(INFO, "Cpu usage: {}", cpu_usage);
                     return pkpy::py_var(vm, cpu_usage);
                 });

        vm->bind(m_systemModule, "get_cpu_temperature() -> float",
                 "get CPU temperature, and return a float value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     float cpu_temperature = Atom::System::GetCpuTemperature();
                     if (cpu_temperature < 0.0f)
                     {
                         LOG_F(ERROR, "Failed to get cpu temperature: {}", cpu_temperature);
                     }
                     DLOG_F(INFO, "Cpu temperature: {}", cpu_temperature);
                     return pkpy::py_var(vm, cpu_temperature);
                 });

        vm->bind(m_systemModule, "get_cpu_model() -> str",
                 "get CPU model, and return a string value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     std::string cpu_model = Atom::System::GetCPUModel();
                     if (cpu_model.empty())
                     {
                         LOG_F(ERROR, "Failed to get cpu model: {}", cpu_model);
                     }
                     DLOG_F(INFO, "Cpu model: {}", cpu_model);
                     return pkpy::py_var(vm, cpu_model);
                 });

        vm->bind(m_systemModule, "get_memory_usage() -> float",
                 "get memory usage, and return a float value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     float memory_usage = Atom::System::GetMemoryUsage();
                     if (memory_usage < 0.0f)
                     {
                         LOG_F(ERROR, "Failed to get memory usage: {}", memory_usage);
                     }
                     DLOG_F(INFO, "Memory usage: {}", memory_usage);
                     return pkpy::py_var(vm, memory_usage);
                 });

        vm->bind(m_systemModule, "get_memory_total() -> float",
                 "get total memory size, and return a float value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     float memory_total = Atom::System::GetTotalMemorySize();
                     if (memory_total < 0.0f)
                     {
                         LOG_F(ERROR, "Failed to get memory total: {}", memory_total);
                     }
                     DLOG_F(INFO, "Memory total: {}", memory_total);
                     return pkpy::py_var(vm, memory_total);
                 });

        vm->bind(m_systemModule, "get_available_memory() -> float",
                 "get available memory size, and return a float value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     float available_memory = Atom::System::GetAvailableMemorySize();
                     if (available_memory < 0.0f)
                     {
                         LOG_F(ERROR, "Failed to get available memory: {}", available_memory);
                     }
                     DLOG_F(INFO, "Available memory: {}", available_memory);
                     return pkpy::py_var(vm, available_memory);
                 });

        vm->bind(m_systemModule, "get_disk_usage() -> dict",
                 "get disk usage, and return a dict value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     std::vector<std::pair<std::string, float>> disk_usage = Atom::System::GetDiskUsage();
                     if (disk_usage.empty())
                     {
                        // LOG_F(ERROR, "Failed to get disk usage: {}", disk_usage);
                     }
                     pkpy::Dict d(vm);
                     for (auto &item : disk_usage)
                     {
                         DLOG_F(INFO, "Disk usage: {} {}", item.first, item.second);
                         d.set(pkpy::py_var(vm, item.first), pkpy::py_var(vm, item.second));
                     }
                     return pkpy::py_var(vm, std::move(d));
                 });

        vm->bind(m_systemModule, "get_disk_model(name : str) -> str",
                 "get disk model, and return a string value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     pkpy::PyObject *name_obj = args[0];
                     pkpy::Str &name = pkpy::py_cast<pkpy::Str &>(vm, name_obj);

                     std::string driver_model = Atom::System::GetDriveModel(name.c_str());
                     if (driver_model.empty())
                     {
                         LOG_F(ERROR, "Failed to get disk model: {}", driver_model);
                     }
                     DLOG_F(INFO, "Disk model: {}", driver_model);
                     return pkpy::py_var(vm, driver_model);
                 });

        vm->bind(m_systemModule, "get_disk_models() -> dict",
                 "get disk models, and return a dict value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     std::vector<std::pair<std::string, std::string>> disk_models = Atom::System::GetStorageDeviceModels();
                     if (disk_models.empty())
                     {
                        // LOG_F(ERROR, "Failed to get disk models: {}", disk_models);
                     }
                     pkpy::Dict d(vm);
                     for (auto &item : disk_models)
                     {
                         DLOG_F(INFO, "Disk model: {} {}", item.first, item.second);
                         d.set(pkpy::py_var(vm, item.first), pkpy::py_var(vm, item.second));
                     }
                     return pkpy::py_var(vm, std::move(d)); });

        vm->bind(m_systemModule, "is_root() -> bool",
                 "check if the current process is running as root, and return a bool value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     bool is_root = Atom::System::IsRoot();
                     return pkpy::py_var(vm, is_root);
                 });

        vm->bind(m_systemModule, "get_current_username() -> str",
                 "get current username, and return a string value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     std::string current_username = Atom::System::GetCurrentUsername();
                     if (current_username.empty())
                     {
                         LOG_F(ERROR, "Failed to get current username: {}", current_username);
                     }
                     DLOG_F(INFO, "Current username: {}", current_username);
                     return pkpy::py_var(vm, current_username);
                 });

        vm->bind(m_systemModule, "shutdown() -> bool",
                 "shutdown the system",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     Atom::System::Shutdown();
                     return pkpy::py_var(vm, true);
                 });

        vm->bind(m_systemModule, "reboot() -> bool",
                 "reboot the system",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     Atom::System::Reboot();
                     return pkpy::py_var(vm, true);
                 });

        vm->bind(m_systemModule, "check_duplicate_process(name : str) -> bool",
                 "check if the process is running, and return a bool value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     pkpy::PyObject *name_obj = args[0];
                     pkpy::Str &name = pkpy::py_cast<pkpy::Str &>(vm, name_obj);

                     bool is_duplicate = Atom::System::CheckDuplicateProcess(name.c_str());
                     if (is_duplicate)
                     {
                         LOG_F(ERROR, "Failed to check duplicate process: {}", is_duplicate);
                     }
                     DLOG_F(INFO, "Duplicate process: {}", is_duplicate);
                     return pkpy::py_var(vm, is_duplicate);
                 });

        vm->bind(m_systemModule, "is_process_running(name : str) -> bool",
                 "check if the process is running, and return a bool value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     pkpy::PyObject *name_obj = args[0];
                     pkpy::Str &name = pkpy::py_cast<pkpy::Str &>(vm, name_obj);

                     bool is_running = Atom::System::isProcessRunning(name.c_str());
                     DLOG_F(INFO, "Process running: {}", is_running ? "true" : "false");
                     return pkpy::py_var(vm, is_running);
                 });

        vm->bind(m_systemModule, "get_process_by_name(name : str) -> dict",
                 "get process info by name, and return a dict value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     pkpy::PyObject *name_obj = args[0];
                     pkpy::Str &name = pkpy::py_cast<pkpy::Str &>(vm, name_obj);
                     Atom::System::ProcessInfo info;
                     pkpy::Dict d(vm);
                     if (Atom::System::GetProcessInfoByName(name.c_str(), info))
                     {
                         d.set(VAR("id"), VAR(info.processID));
                         d.set(VAR("parent_id"), VAR(info.parentProcessID));
                         d.set(VAR("priority"), VAR(info.basePriority));
                         d.set(VAR("executable"), VAR(info.executableFile));
                     }
                     else
                     {
                         LOG_F(ERROR, "Failed to get process info!");
                     }
                     return pkpy::py_var(vm, std::move(d));
                 });

        vm->bind(m_systemModule, "get_process_by_id(id : int) -> dict",
                 "get process info by id, and return a dict value",
                 [](pkpy::VM *vm, pkpy::ArgsView args)
                 {
                     int id = pkpy::py_cast<int>(vm, args[0]);
                     Atom::System::ProcessInfo info;
                     pkpy::Dict d(vm);
                     if (Atom::System::GetProcessInfoByID(id, info))
                     {
                         d.set(VAR("id"), VAR(info.processID));
                         d.set(VAR("parent_id"), VAR(info.parentProcessID));
                         d.set(VAR("priority"), VAR(info.basePriority));
                         d.set(VAR("executable"), VAR(info.executableFile));
                     }
                     else
                     {
                         LOG_F(ERROR, "Failed to get process info!");
                     }
                     return pkpy::py_var(vm, std::move(d));
                 });
    }
} // namespace Lithium
