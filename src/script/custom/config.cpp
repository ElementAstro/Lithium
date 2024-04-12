/*
 * config.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-13

Description: Config module for PocketPy(builtin)

**************************************************/

#include "pocketpy/include/pocketpy/bindings.h"
using namespace pkpy;

#include "atom/log/loguru.hpp"
#include "atom/server/global_ptr.hpp"
#include "atom/system/system.hpp"
#include "atom/type/json.hpp"
#include "config/configor.hpp"

using json = nlohmann::json;

namespace Lithium {
template <typename T>
pkpy::PyObject *get_config(pkpy::VM *vm, pkpy::Str &key,
                           const std::string &log_type) {
    json value = GetPtr<Lithium::ConfigManager>("lithium.config")
                     .value()
                     ->getValue(key.str())
                     .value();
    if (value.is_null() || value.type_name() != typeid(T).name()) {
        LOG_F(ERROR, "Failed to get config value: {}", key.str());
        return pkpy::py_var(vm, T());
    }
    DLOG_F(INFO, "Config value: {}", value.get<T>());
    return pkpy::py_var(vm, value.get<T>());
}

template <typename T>
pkpy::PyObject *set_config(pkpy::VM *vm, pkpy::Str &key, T value) {
    return pkpy::py_var(vm, GetPtr<Lithium::ConfigManager>("lithium.config")
                                .value()
                                ->setValue(key.str(), value));
}

void addConfigModule(VM *vm) {
    DLOG_F(INFO, "Adding config module");
    PyObject *mod = vm->new_module("li_config");

    vm->bind(mod, "get_str_config(key : str) -> str",
             "get specified config value and return in string type",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 return get_config<std::string>(vm, key, "str");
             });

    vm->bind(mod, "get_int_config(key : str) -> int",
             "get specified config value and return in int type",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 return get_config<int>(vm, key, "int");
             });

    vm->bind(mod, "get_float_config(key : str) -> float",
             "get specified config value and return in float type",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 return get_config<float>(vm, key, "float");
             });

    vm->bind(mod, "get_bool_config(key : str) -> bool",
             "get specified config value and return in bool type",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 return get_config<bool>(vm, key, "bool");
             });

    vm->bind(mod,
             "set_str_config(key : str, value : str) -> bool"
             "set specified config value in string type",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 pkpy::PyObject *value_obj = args[1];
                 pkpy::Str &value = pkpy::py_cast<pkpy::Str &>(vm, value_obj);
                 return set_config(vm, key, value.str());
             });

    vm->bind(mod,
             "set_int_config(key : str, value : int) -> bool"
             "set specified config value in int type",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 pkpy::PyObject *value_obj = args[1];
                 int value = pkpy::py_cast<int>(vm, value_obj);
                 return set_config(vm, key, value);
             });

    vm->bind(mod,
             "set_float_config(key : str, value : float) -> bool"
             "set specified config value in float type",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 pkpy::PyObject *value_obj = args[1];
                 float value = pkpy::py_cast<float>(vm, value_obj);
                 return set_config(vm, key, value);
             });

    vm->bind(mod,
             "set_bool_config(key : str, value : bool) -> bool"
             "set specified config value in bool type",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 pkpy::PyObject *value_obj = args[1];
                 bool value = pkpy::py_cast<bool>(vm, value_obj);
                 return set_config(vm, key, value);
             });

    vm->bind(mod, "delete_config(key : str) -> bool",
             "delete specified config value",
             [](pkpy::VM *vm, pkpy::ArgsView args) {
                 pkpy::PyObject *key_obj = args[0];
                 pkpy::Str &key = pkpy::py_cast<pkpy::Str &>(vm, key_obj);
                 return pkpy::py_var(
                     vm, GetPtr<Lithium::ConfigManager>("lithium.config")
                             .value()
                             ->deleteValue(key.str()));
             });

    vm->bind(
        mod, "save_config(path : str) -> bool", "save config to specified path",
        [](pkpy::VM *vm, pkpy::ArgsView args) {
            pkpy::PyObject *path_obj = args[0];
            pkpy::Str &path = pkpy::py_cast<pkpy::Str &>(vm, path_obj);
            return pkpy::py_var(
                vm, GetPtr<Lithium::ConfigManager>("lithium.config")
                        .value()
                        ->saveToFile(path.str().empty() ? "config/config.json"
                                                        : path.str()));
        });

    vm->bind(
        mod, "load_config(path : str) -> bool",
        "load config from specified path",
        [](pkpy::VM *vm, pkpy::ArgsView args) {
            pkpy::PyObject *path_obj = args[0];
            pkpy::Str &path = pkpy::py_cast<pkpy::Str &>(vm, path_obj);
            return pkpy::py_var(
                vm, GetPtr<Lithium::ConfigManager>("lithium.config")
                        .value()
                        ->loadFromFile(path.str().empty() ? "config/config.json"
                                                          : path.str()));
        });
}
}  // namespace Lithium
