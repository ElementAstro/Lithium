/*
 * modloader.hpp
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

Date: 2023-3-29

Description: C++ and Python Modules Loader

**************************************************/

#pragma once

#include <vector>
#include <unordered_map>
#include <cstdio>
#include <functional>

#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#define MODULE_HANDLE void *
#define LOAD_LIBRARY(p) dlopen(p, RTLD_NOW | RTLD_GLOBAL)
#define LOAD_SHARED_LIBRARY(file, size) dlopen(nullptr, RTLD_NOW | RTLD_GLOBAL)
#define UNLOAD_LIBRARY(p) dlclose(p)
#define LOAD_ERROR() dlerror()
#define LOAD_FUNCTION(handle, name) dlsym(handle, name)

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "thread.hpp"
#include "task.hpp"

namespace OpenAPT
{

    nlohmann::json iterator_modules_dir();

    class ModuleLoader
    {
    public:
        ModuleLoader();
        ~ModuleLoader();
        bool LoadModule(const std::string &path, const std::string &name);
        bool UnloadModule(const std::string &name);

        template <typename T>
        T GetFunction(const std::string &module_name, const std::string &function_name)
        {
            auto handle_it = handles_.find(module_name);
            if (handle_it == handles_.end())
            {
                spdlog::error("Failed to find module {}", module_name);
                return nullptr;
            }

            auto func_ptr = reinterpret_cast<T>(LOAD_FUNCTION(handle_it->second, function_name.c_str()));

            if (!func_ptr)
            {
                spdlog::error("Failed to get symbol {} from module {}: {}", function_name, module_name, dlerror());
                return nullptr;
            }

            return func_ptr;
        }

        /**
         * @brief   Gets a pointer to the Task class instance from the specified module.
         *
         * This function tries to get a pointer to an instance of the Task class from the specified module.
         * If the module is not loaded or the function does not exist, this function will return nullptr.
         *
         * @param[in]   module_name     The name of the module which contains the Task class implementation.
         * @return      A pointer to the Task class instance if the loading is successful, nullptr otherwise.
         */
        BasicTask *GetTaskPointer(const std::string &module_name, const nlohmann::json &config);

        template <typename T, typename class_type, typename... Args>
        typename std::enable_if<std::is_class<class_type>::value, bool>::type
        LoadAndRunFunction(const std::string &module_name, const std::string &func_name,
                           const std::string &thread_name, bool runasync, class_type *instance, Args... args)
        {
            typedef T (class_type::*MemberFunctionPtr)(Args...);
            typedef T (*FunctionPtr)(Args...);
            void *handle = GetHandle(module_name);
            auto sym_ptr = dlsym(handle, func_name.c_str());
            if (!sym_ptr)
            {
                spdlog::error("Failed to load symbol {}: {}", func_name, dlerror());
                return false;
            }
            FunctionPtr func_ptr = reinterpret_cast<FunctionPtr>(sym_ptr);
            MemberFunctionPtr member_func_ptr = nullptr;
            if constexpr (std::is_member_function_pointer_v<MemberFunctionPtr>)
            {
                member_func_ptr = reinterpret_cast<MemberFunctionPtr>(sym_ptr);
            }
            m_ThreadManager->addThread(std::bind(member_func_ptr, instance, args...), std::bind(func_ptr, args...), thread_name);
            return true;
        }

        template <typename T, typename... Args>
        T LoadAndRunFunction(const std::string &module_name, const std::string &func_name,
                             const std::string &thread_name, bool runasync, Args &&...args)
        {
            // 定义函数指针类型
            typedef T (*FunctionPtr)(Args...);

            // 获取动态链接库句柄
            void *handle = GetHandle(module_name);

            // 加载函数
            auto sym_ptr = dlsym(handle, func_name.c_str());
            if (!sym_ptr)
            {
                spdlog::error("Failed to load symbol {}: {}", func_name, dlerror());
                return static_cast<T>(0);
            }
            FunctionPtr func_ptr = reinterpret_cast<FunctionPtr>(sym_ptr);

            if (runasync)
            {
                m_ThreadManager->addThread(std::bind(func_ptr, std::forward<Args>(args)...), thread_name);
            }
            else
            {
                auto funcc = std::bind(func_ptr, std::forward<Args>(args)...);
                funcc();
                spdlog::debug("Simple not async function is executed successfully!");
            }

            // 返回函数返回值
            return static_cast<T>(0);
        }

    public:
        void *GetHandle(const std::string &name) const
        {
            auto it = handles_.find(name);
            if (it == handles_.end())
            {
                return nullptr;
            }
            return it->second;
        }

        bool HasModule(const std::string &name) const;

    private:
        std::unordered_map<std::string, void *> handles_;

        ThreadManager *m_ThreadManager;
    };
}