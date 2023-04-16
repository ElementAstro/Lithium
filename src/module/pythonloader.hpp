/*
 * pythonloader.hpp
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

#ifndef PY_MODULE_LOADER_H
#define PY_MODULE_LOADER_H

#include <Python.h>
#include <map>
#include <string>

namespace OpenAPT
{
    class PyModuleLoader
    {
    public:
        PyModuleLoader();
        ~PyModuleLoader();

        bool load_module(const std::string &name);
        bool load_local_module(const std::string &path);
        void unload_module(const std::string &name);

        PyObject *get_function(const std::string &module_name, const std::string &function_name);

        template <typename T>
        T get_function(const std::string &module_name, const std::string &function_name)
        {
            return reinterpret_cast<T>(get_function(module_name, function_name));
        }

        std::vector<std::string> get_all_functions(const std::string &module_name);

        template <typename T>
        bool set_variable(const std::string &module_name, const std::string &var_name, T value)
        {
            auto it = modules_.find(module_name);
            if (it == modules_.end())
            {
                std::cerr << "Failed to find module " << module_name << std::endl;
                return false;
            }
            PyObject *py_module = it->second;
            PyObject *py_value = Py_BuildValue("i", value);
            PyModule_AddObject(py_module, var_name.c_str(), py_value);
            Py_DECREF(py_value);
            return true;
        }

        template <typename Ret, typename... Args>
        Ret call_function(const std::string &module_name, const std::string &function_name, Args &&...args)
        {
            auto it = modules_.find(module_name);
            if (it == modules_.end())
            {
                std::cerr << "Failed to find module " << module_name << std::endl;
                return Ret();
            }
            PyObject *py_module = it->second;
            PyObject *py_func = PyObject_GetAttrString(py_module, function_name.c_str());
            if (!py_func)
            {
                std::cerr << "Failed to get function " << function_name << " from module " << module_name << std::endl;
                PyErr_Print();
                Py_XDECREF(py_func);
                return Ret();
            }
            if (!PyCallable_Check(py_func))
            {
                std::cerr << function_name << " is not a callable object" << std::endl;
                PyErr_Print();
                Py_DECREF(py_func);
                return Ret();
            }
            PyObject *py_args = PyTuple_New(sizeof...(args));
            int i = 0;
            ((PyTuple_SetItem(py_args, i++, Py_BuildValue("l", static_cast<long long>(args)))), ...); // 修改了 Py_BuildValue 的格式化字符串和参数类型。
            PyObject *py_ret = PyObject_CallObject(py_func, py_args);
            Py_DECREF(py_args);
            Py_DECREF(py_func);
            py_func = NULL;
            if (!py_ret)
            {
                std::cerr << "Failed to call function " << function_name << std::endl;
                PyErr_Print();
                Py_XDECREF(py_ret);
                return Ret();
            }
            Ret ret;
            PyArg_Parse(py_ret, "i", &ret);
            Py_DECREF(py_ret);
            return ret;
        }

        bool set_env(const std::string &name, const std::string &value);

        bool load_py_module(const std::string &name);

        bool register_cpp_function(const std::string &module_name, const std::string &function_name, PyObject *(*func)(PyObject *, PyObject *));

        template <typename T>
        bool register_cpp_class(const std::string &module_name, const std::string &class_name)
        {
            auto it = modules_.find(module_name);
            if (it == modules_.end())
            {
                std::cerr << "Failed to find module " << module_name << std::endl;
                return false;
            }
            PyObject *py_module = it->second;
            PyObject *py_class = PyType_FromSpec(&T::py_type);
            if (!py_class)
            {
                std::cerr << "Failed to create Python class " << class_name << std::endl;
                PyErr_Print();
                Py_XDECREF(py_class);
                return false;
            }
            PyObject_SetAttrString(py_module, class_name.c_str(), py_class);
            Py_DECREF(py_class);
            return true;
        }

        bool cache_py_module(const std::string &path);

        bool check_function(const std::string &module_name, const std::string &function_name);

        std::string get_function_signature(const std::string &module_name, const std::string &function_name);

    private:
        std::map<std::string, PyObject *> modules_;
        std::unordered_map<std::string, nlohmann::json> module_cache_;
    };

}

#endif // PY_MODULE_LOADER_H
