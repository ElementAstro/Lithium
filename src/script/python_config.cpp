/*
 * python_config.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Config module for Python scripting engine

**************************************************/

#include "python.hpp"

#include "atom/system/system.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace Lithium
{
    void PyScriptManager::InjectConfigModule()
    {
        vm->bind(m_configModule, "get_str_config(key : str) -> str",
                 "get specified config value and return in string type",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     json value = m_configManager->getValue(key);
                     if (value.is_null() || !value.is_string())
                     {
                         LOG_F(ERROR, "Failed to get config value: {}", key);
                     }
                     DLOG_F(INFO, "Config value: {}", value.get<std::string>());
                     return py_var(vm, value.get<std::string>());
                 });

        vm->bind(m_configModule, "get_int_config(key : str) -> int",
                 "get specified config value and return in int type",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     json value = m_configManager->getValue(key);
                     if (value.is_null() || !value.is_number())
                     {
                         LOG_F(ERROR, "Failed to get config value: {}", key);
                     }
                     DLOG_F(INFO, "Config value: {}", value.get<int>());
                     return py_var(vm, value.get<int>());
                 });

        vm->bind(m_configModule, "get_float_config(key : str) -> float",
                 "get specified config value and return in float type",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     json value = m_configManager->getValue(key);
                     if (value.is_null() || !value.is_number())
                     {
                         LOG_F(ERROR, "Failed to get config value: {}", key);
                     }
                     DLOG_F(INFO, "Config value: {}", value.get<float>());
                     return py_var(vm, value.get<float>());
                 });

        vm->bind(m_configModule, "get_bool_config(key : str) -> bool",
                 "get specified config value and return in bool type",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     json value = m_configManager->getValue(key);
                     if (value.is_null() || !value.is_boolean())
                     {
                         LOG_F(ERROR, "Failed to get config value: {}", key);
                     }
                     DLOG_F(INFO, "Config value: {}", value.get<bool>());
                     return py_var(vm, value.get<bool>());
                 });

        vm->bind(m_configModule, "set_str_config(key : str, value : str) -> bool"
                                 "set specified config value in string type",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     PyObject *value_obj = args[1];
                     Str &value = py_cast<Str &>(vm, value_obj);
                     return py_var(vm, m_configManager->setValue(key, value));
                 });

        vm->bind(m_configModule, "set_int_config(key : str, value : int) -> bool"
                                 "set specified config value in int type",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     PyObject *value_obj = args[1];
                     int value = py_cast<int>(vm, value_obj);
                     return py_var(vm, m_configManager->setValue(key, value));
                 });

        vm->bind(m_configModule, "set_float_config(key : str, value : float) -> bool"
                                 "set specified config value in float type",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     PyObject *value_obj = args[1];
                     float value = py_cast<float>(vm, value_obj);
                     return py_var(vm, m_configManager->setValue(key, value));
                 });

        vm->bind(m_configModule, "set_bool_config(key : str, value : bool) -> bool"
                                 "set specified config value in bool type",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     PyObject *value_obj = args[1];
                     bool value = py_cast<bool>(vm, value_obj);
                     return py_var(vm, m_configManager->setValue(key, value));
                 });

        vm->bind(m_configModule, "delete_config(key : str) -> bool",
                 "delete specified config value",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *key_obj = args[0];
                     Str &key = py_cast<Str &>(vm, key_obj);
                     return py_var(vm, m_configManager->deleteValue(key));
                 });

        vm->bind(m_configModule, "save_config(path : str) -> bool",
                 "save config to specified path",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *path_obj = args[0];
                     Str &path = py_cast<Str &>(vm, path_obj);
                     return py_var(vm, m_configManager->saveConfig(path.empty() ? "config/config.json" : path));
                 });

        vm->bind(m_configModule, "load_config(path : str) -> bool",
                 "load config from specified path",
                 [](VM *vm, ArgsView args)
                 {
                     PyObject *path_obj = args[0];
                     Str &path = py_cast<Str &>(vm, path_obj);
                     return py_var(vm, m_configManager->loadFromFile(path.empty() ? "config/config.json" : path));
                 });
    }
} // namespace Lithium
