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

namespace OpenAPT {
    class PyModuleLoader {
    public:
        /**
         * @brief 构造函数
         */
        PyModuleLoader();

        /**
         * @brief 析构函数
         */
        ~PyModuleLoader();

        /**
         * @brief 加载Python模块
         * @param name 模块名
         * @return 是否加载成功
         */
        bool load_module(const std::string& name);

        bool load_local_module(const std::string& path);

        /**
         * @brief 卸载Python模块
         * @param name 模块名
         */
        void unload_module(const std::string& name);

        /**
         * @brief 获取Python模块中的函数
         * @param module_name 模块名
         * @param function_name 函数名
         * @return 函数指针
         */
        PyObject* get_function(const std::string& module_name, const std::string& function_name);

        /**
         * @brief 获取Python模块中的函数
         * @tparam T 函数指针类型
         * @param module_name 模块名
         * @param function_name 函数名
         * @return 函数指针
         */
        template<typename T>
        T get_function(const std::string& module_name, const std::string& function_name) {
            return reinterpret_cast<T>(get_function(module_name, function_name));
        }

        std::vector<std::string> get_all_functions(const std::string& module_name);

        /**
         * @brief 设置Python模块中的变量
         * @tparam T 变量类型
         * @param module_name 模块名
         * @param var_name 变量名
         * @param value 变量值
         * @return 是否设置成功
         */
        template<typename T>
        bool set_variable(const std::string& module_name, const std::string& var_name, T value) {
            auto it = modules_.find(module_name);
            if (it == modules_.end()) {
                std::cerr << "Failed to find module " << module_name << std::endl;
                return false;
            }
            PyObject* py_module = it->second;
            PyObject* py_value = Py_BuildValue("i", value);
            PyModule_AddObject(py_module, var_name.c_str(), py_value);
            Py_DECREF(py_value);
            return true;
        }

        /**
         * @brief 调用Python模块中的函数
         * @tparam Ret 返回值类型
         * @tparam Args 参数类型列表
         * @param module_name 模块名
         * @param function_name 函数名
         * @param args 参数列表
         * @return 函数返回值
         */
        template<typename Ret, typename ...Args>
        Ret call_function(const std::string& module_name, const std::string& function_name, Args&&... args) {
            auto it = modules_.find(module_name);
            if (it == modules_.end()) {
                std::cerr << "Failed to find module " << module_name << std::endl;
                return Ret();
            }
            PyObject* py_module = it->second;
            PyObject* py_func = PyObject_GetAttrString(py_module, function_name.c_str());
            if (!py_func) {
                std::cerr << "Failed to get function " << function_name << " from module " << module_name << std::endl;
                PyErr_Print();
                Py_XDECREF(py_func);
                return Ret();
            }
            if (!PyCallable_Check(py_func)) {
                std::cerr << function_name << " is not a callable object" << std::endl;
                PyErr_Print();
                Py_DECREF(py_func);
                return Ret();
            }
            PyObject* py_args = PyTuple_New(sizeof...(args));
            int i = 0;
            ((PyTuple_SetItem(py_args, i++, Py_BuildValue("i", args))), ...);
            PyObject* py_ret = PyObject_CallObject(py_func, py_args);
            Py_DECREF(py_args);
            Py_DECREF(py_func);  // 释放已分配的内存
            py_func = NULL;  // 将指针设置为 NULL
            if (!py_ret) {
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


    private:
        std::map<std::string, PyObject*> modules_;
    };
}


#endif  // PY_MODULE_LOADER_H
