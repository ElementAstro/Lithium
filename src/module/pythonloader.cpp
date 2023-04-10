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

    bool PyModuleLoader::load_local_module(const std::string &path)
    {
        std::string module_name = path.substr(path.find_last_of('/') + 1);
        module_name = module_name.substr(0, module_name.find_last_of('.'));
        auto it = modules_.find(module_name);
        if (it != modules_.end())
        {
            std::cout << "Module " << module_name << " has been loaded." << std::endl;
            return true;
        }
        PyObject *py_module = PyImport_Import(PyUnicode_FromString(path.c_str()));
        if (!py_module)
        {
            std::cerr << "Failed to load module " << module_name << std::endl;
            Py_XDECREF(py_module);
            return false;
        }
        modules_[module_name] = py_module;
        std::cout << "Loaded module " << module_name << std::endl;
        return true;
    }

    bool PyModuleLoader::set_env(const std::string &name, const std::string &value)
    {
        int res = setenv(name.c_str(), value.c_str(), 1);
        return res == 0;
    }

    bool PyModuleLoader::load_py_module(const std::string &name)
    {
        PyObject *py_module = PyImport_ImportModule(name.c_str());
        if (!py_module)
        {
            std::cerr << "Failed to load Python module " << name << std::endl;
            PyErr_Print();
            Py_XDECREF(py_module);
            return false;
        }
        modules_[name] = py_module;
        return true;
    }

    bool PyModuleLoader::register_cpp_function(const std::string &module_name, const std::string &function_name, PyObject *(*func)(PyObject *, PyObject *))
    {
        auto it = modules_.find(module_name);
        if (it == modules_.end())
        {
            std::cerr << "Failed to find module " << module_name << std::endl;
            return false;
        }
        PyObject *py_module = it->second;
        PyObject *py_func = PyCapsule_New((void *)func, NULL, NULL);
        if (!py_func)
        {
            std::cerr << "Failed to create Python capsule for C++ function " << function_name << std::endl;
            PyErr_Print();
            Py_XDECREF(py_func);
            return false;
        }
        static PyMethodDef method_def = {function_name.c_str(), (PyCFunction)func, METH_VARARGS, NULL};
        PyObject_SetAttrString(py_module, function_name.c_str(), PyCFunction_New(&method_def, NULL));
        Py_DECREF(py_func);
        return true;
    }

    bool PyModuleLoader::cache_py_module(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Failed to open file " << path << std::endl;
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    PyCompilerFlags flags = {};
    flags.cf_flags |= PyCF_ONLY_AST;
    PyObject *py_ast = Py_CompileStringFlags(content.c_str(), path.c_str(), Py_file_input, &flags);

    file.close();
    if (!py_ast)
    {
        return false;
    }

    nlohmann::json cache;

    // Cache all functions
    PyObject *py_node_iter = PyObject_GetIter(py_ast);
    PyObject *py_node;
    while ((py_node = PyIter_Next(py_node_iter)))
    {
        const char *node_type = PyUnicode_AsUTF8(PyObject_GetAttrString(py_node, "kind"));
        if (strcmp(node_type, "FunctionDef") == 0)
        {
            const char *func_name = PyUnicode_AsUTF8(PyObject_GetAttrString(py_node, "name"));
            PyObject *py_args = PyObject_GetAttrString(py_node, "args");
            PyObject *py_arg_list = PyObject_GetAttrString(py_args, "args");

            std::vector<std::string> arg_types;
            for (int i = 0; i < PyList_Size(py_arg_list); i++)
            {
                PyObject *py_arg = PyList_GetItem(py_arg_list, i);
                const char *arg_name = PyUnicode_AsUTF8(PyObject_GetAttrString(py_arg, "arg"));
                const char *arg_type = PyUnicode_AsUTF8(PyObject_GetAttrString(PyObject_GetAttrString(py_arg, "annotation"), "_name"));
                arg_types.push_back(arg_type);
            }

            cache["functions"][func_name]["args"] = arg_types;
        }
        Py_DECREF(py_node);
    }
    Py_DECREF(py_node_iter);

    // Cache all global variables
    py_node_iter = PyObject_GetIter(py_ast);
    while ((py_node = PyIter_Next(py_node_iter)))
    {
        const char *node_type = PyUnicode_AsUTF8(PyObject_GetAttrString(py_node, "kind"));
        if (strcmp(node_type, "Assign") == 0)
        {
            PyObject *py_targets = PyObject_GetAttrString(py_node, "targets");
            PyObject *py_value = PyObject_GetAttrString(py_node, "value");

            if (PyList_Size(py_targets) == 1)
            {
                const char *var_name = PyUnicode_AsUTF8(PyObject_GetAttrString(PyList_GetItem(py_targets, 0), "id"));
                cache["globals"].push_back(var_name);
            }

            Py_DECREF(py_targets);
            Py_DECREF(py_value);
        }
        Py_DECREF(py_node);
    }
    Py_DECREF(py_node_iter);

    // Save cache to file
    std::ofstream cache_file(path + ".cache");
    if (cache_file.is_open())
    {
        cache_file << cache.dump();
        cache_file.close();
    }
    else
    {
        std::cerr << "Failed to open cache file " << path << ".cache" << std::endl;
    }

    module_cache_[path] = cache;

    Py_DECREF(py_ast);
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

    PyObject *PyModuleLoader::get_function(const std::string &module_name, const std::string &function_name)
    {
        spdlog::debug("get_function: module_name = {}, function_name = {}", module_name, function_name);
        auto it = modules_.find(module_name);
        if (it == modules_.end())
        {
            std::cerr << "Failed to find module " << module_name << std::endl;
            return nullptr;
        }
        PyObject *py_module = it->second;
        PyObject *py_func = PyObject_GetAttrString(py_module, function_name.c_str());
        if (!py_func || !PyCallable_Check(py_func))
        {
            std::cerr << "Failed to get function " << function_name << " from module " << module_name << std::endl;
            Py_XDECREF(py_func);
            return nullptr;
        }
        spdlog::debug("get_function: return function");
        return py_func;
    }

    std::vector<std::string> PyModuleLoader::get_all_functions(const std::string &module_name)
    {
        std::vector<std::string> functions;
        auto it = modules_.find(module_name);
        if (it == modules_.end())
        {
            spdlog::error("Failed to find module {}", module_name);
            return functions;
        }
        PyObject *py_module = it->second;
        PyObject *py_dict = PyModule_GetDict(py_module);
        PyObject *py_items = PyDict_Items(py_dict);
        if (!py_items)
        {
            spdlog::error("Failed to get items from module dict for {}", module_name);
            return functions;
        }
        Py_ssize_t size = PyList_Size(py_items);
        for (Py_ssize_t i = 0; i < size; ++i)
        {
            PyObject *item = PyList_GetItem(py_items, i);
            if (!item || !PyTuple_Check(item))
            {
                continue;
            }
            PyObject *key = PyTuple_GetItem(item, 0);
            PyObject *value = PyTuple_GetItem(item, 1);
            if (!key || !value || !PyUnicode_Check(key) || !PyCallable_Check(value))
            {
                continue;
            }
            const char *c_key = PyUnicode_AsUTF8(key);
            functions.emplace_back(c_key);
            ;
        }
        Py_DECREF(py_dict);
        Py_DECREF(py_items);
        return functions;
    }

    bool PyModuleLoader::check_function(const std::string &module_name, const std::string &function_name)
    {
        auto it = module_cache_.find(module_name);
        if (it == module_cache_.end())
        {
            load_module(module_name);
            it = module_cache_.find(module_name);
            if (it == module_cache_.end())
            {
                std::cerr << "Failed to load module " << module_name << std::endl;
                return false;
            }
        }

        nlohmann::json module_info = it->second;
        if (!module_info.contains("functions"))
        {
            return false;
        }

        auto function_it = module_info["functions"].find(function_name);
        if (function_it == module_info["functions"].end())
        {
            return false;
        }

        PyObject *module_obj = PyImport_ImportModule(module_name.c_str());
        if (!module_obj)
        {
            return false;
        }

        PyObject *func_name = PyUnicode_FromString(function_name.c_str());
        PyObject *func_obj = PyObject_GetAttr(module_obj, func_name);

        Py_DECREF(func_name);
        Py_DECREF(module_obj);

        return func_obj && PyCallable_Check(func_obj);
    }


    std::string get_function_signature(const std::string &module_name, const std::string &function_name)
    {
        // Load the module
        PyObject *module = PyImport_ImportModule(module_name.c_str());
        if (!module)
        {
            PyErr_Print();
            return "";
        }

        // Get the function object
        PyObject *func = PyObject_GetAttrString(module, function_name.c_str());
        if (!func || !PyCallable_Check(func))
        {
            PyErr_Print();
            Py_XDECREF(func);
            Py_DECREF(module);
            return "";
        }

        // Get the function signature
        PyObject *signature = PyObject_Str(PyObject_GetAttrString(func, "__signature__"));
        Py_DECREF(func);
        Py_DECREF(module);

        // Extract the string representation of the signature
        std::string result;
        if (signature && PyUnicode_Check(signature))
        {
            PyObject *utf8 = PyUnicode_AsUTF8String(signature);
            if (utf8)
            {
                result = std::string(PyBytes_AsString(utf8));
                Py_DECREF(utf8);
            }
        }
        Py_XDECREF(signature);

        return result;
    }

}
