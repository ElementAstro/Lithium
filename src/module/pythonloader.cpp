/*
 * pythonloader.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-4-9

Description: Python Module Loader

**************************************************/

#include "pythonloader.hpp"

#include <iostream>
#include <spdlog/spdlog.h>

namespace OpenAPT
{
    PyModuleLoader::PyModuleLoader()
    {
        Py_Initialize();
        PyRun_SimpleString("import sys");
	    PyRun_SimpleString("sys.path.append('.')");
    }

    PyModuleLoader::~PyModuleLoader()
    {
        for (auto &item : modules_)
        {
            Py_DECREF(item.second);
        }
        Py_Finalize();
    }

    bool PyModuleLoader::load_module(const std::string &name)
    {
        auto it = modules_.find(name);
        if (it != modules_.end())
        {
            std::cerr << "Module " << name << " has already been loaded" << std::endl;
            return false;
        }
        PyObject *py_name = PyUnicode_FromString(name.c_str());
        PyObject *py_module = PyImport_Import(py_name);
        Py_DECREF(py_name);
        if (!py_module)
        {
            std::cerr << "Failed to load module " << name << std::endl;
            PyErr_Print();
            return false;
        }
        modules_[name] = py_module;
        return true;
    }

    bool PyModuleLoader::load_local_module(const std::string& path) {
        std::string module_name = path.substr(path.find_last_of('/') + 1);
        module_name = module_name.substr(0, module_name.find_last_of('.'));
        auto it = modules_.find(module_name);
        if (it != modules_.end()) {
            std::cout << "Module " << module_name << " has been loaded." << std::endl;
            return true;
        }
        PyObject* py_module = PyImport_Import(PyUnicode_FromString(path.c_str()));
        if (!py_module) {
            std::cerr << "Failed to load module " << module_name << std::endl;
            Py_XDECREF(py_module);
            return false;
        }
        modules_[module_name] = py_module;
        std::cout << "Loaded module " << module_name << std::endl;
        return true;
    }

    void PyModuleLoader::unload_module(const std::string &name)
    {
        auto it = modules_.find(name);
        if (it == modules_.end())
        {
            std::cerr << "Failed to find module " << name << std::endl;
            return;
        }
        Py_DECREF(it->second);
        modules_.erase(it);
    }

    PyObject* PyModuleLoader::get_function(const std::string& module_name, const std::string& function_name) {
        spdlog::debug("get_function: module_name = {}, function_name = {}", module_name, function_name);
        auto it = modules_.find(module_name);
        if (it == modules_.end()) {
            std::cerr << "Failed to find module " << module_name << std::endl;
            return nullptr;
        }
        PyObject* py_module = it->second;
        PyObject* py_func = PyObject_GetAttrString(py_module, function_name.c_str());
        if (!py_func || !PyCallable_Check(py_func)) {
            std::cerr << "Failed to get function " << function_name << " from module " << module_name << std::endl;
            Py_XDECREF(py_func);
            return nullptr;
        }
        spdlog::debug("get_function: return function");
        return py_func;
    }

    std::vector<std::string> PyModuleLoader::get_all_functions(const std::string& module_name) {
        std::vector<std::string> functions;
        auto it = modules_.find(module_name);
        if (it == modules_.end()) {
            spdlog::error("Failed to find module {}", module_name);
            return functions;
        }
        PyObject* py_module = it->second;
        PyObject* py_dict = PyModule_GetDict(py_module);
        PyObject* py_items = PyDict_Items(py_dict);
        if (!py_items) {
            spdlog::error("Failed to get items from module dict for {}", module_name);
            return functions;
        }
        Py_ssize_t size = PyList_Size(py_items);
        for (Py_ssize_t i = 0; i < size; ++i) {
            PyObject* item = PyList_GetItem(py_items, i);
            if (!item || !PyTuple_Check(item)) {
                continue;
            }
            PyObject* key = PyTuple_GetItem(item, 0);
            PyObject* value = PyTuple_GetItem(item, 1);
            if (!key || !value || !PyUnicode_Check(key) || !PyCallable_Check(value)) {
                continue;
            }
            const char* c_key = PyUnicode_AsUTF8(key);
            functions.emplace_back(c_key);;
        }
        Py_DECREF(py_dict);
        Py_DECREF(py_items);
        return functions;
    }



}
