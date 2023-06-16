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
#include "device.hpp"

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
         * @brief   Gets a pointer to an instance of the BasicTask class from the specified module.
         *
         * This function tries to get a pointer to an instance of the BasicTask class from the specified module.
         * If the module is not loaded or the function does not exist, this function will return nullptr.
         *
         * @param[in]   module_name     The name of the module which contains the BasicTask class implementation.
         * @param[in]   config          A JSON object containing configuration information for the task instance.
         * @return      A shared pointer to the BasicTask class instance if the loading is successful, nullptr otherwise.
         */
        std::shared_ptr<BasicTask> GetTaskPointer(const std::string &module_name, const nlohmann::json &config);

        /**
         * @brief   Gets a pointer to an instance of the Device class from the specified module.
         *
         * This function tries to get a pointer to an instance of the Device class from the specified module.
         * If the module is not loaded or the function does not exist, this function will return nullptr.
         *
         * @param[in]   module_name     The name of the module which contains the Device class implementation.
         * @param[in]   config          A JSON object containing configuration information for the device instance.
         * @return      A shared pointer to the Device class instance if the loading is successful, nullptr otherwise.
         */
        std::shared_ptr<Device> GetDevicePointer(const std::string &module_name, const nlohmann::json &config);

        /**
         * @brief   动态加载指定共享库中的函数，并在线程管理器中运行。
         *
         * 此模板函数需要一个成员函数或非成员函数的函数指针，以及该函数所属类的实例和一些参数。
         * 函数将获取指定共享库中的函数指针，使用 std::bind 函数构造出函数对象，
         * 然后将其传递给线程管理器以便管理执行。如果执行成功，函数会返回 true；否则返回 false。
         *
         * @tparam      T               函数返回值类型
         * @tparam      class_type      成员函数所属的类
         * @tparam      Args            函数除去成员函数指针之外的参数类型
         * @param[in]   module_name     共享库的名称
         * @param[in]   func_name       要加载的函数名
         * @param[in]   thread_name     要为执行创建的线程名称
         * @param[in]   runasync        是否异步执行
         * @param[in]   instance        包含成员函数的实例
         * @param[in]   args            除了成员函数指针之外的其他参数
         * @return      如果执行成功，返回 true；否则返回 false。
         */
        template <typename T, typename class_type, typename... Args>
        typename std::enable_if<std::is_class<class_type>::value, bool>::type
        LoadAndRunFunction(const std::string &module_name, const std::string &func_name,
                           const std::string &thread_name, bool runasync, class_type *instance, Args... args)
        {
            // 定义成员函数指针和函数指针类型
            typedef T (class_type::*MemberFunctionPtr)(Args...);
            typedef T (*FunctionPtr)(Args...);

            // 获取共享库句柄和函数指针
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

            // 将函数绑定到成员函数或者普通函数指针上，然后传递给线程管理器
            m_ThreadManager->addThread(std::bind(member_func_ptr, instance, args...), std::bind(func_ptr, args...), thread_name);
            return true;
        }

        /**
         * @brief   动态加载指定共享库中的函数，并直接调用它。
         *
         * 此模板函数需要一个非成员函数的函数指针以及一些参数。
         * 函数将获取指定共享库中的函数指针并直接调用它，可以选择同步或异步执行。
         * 如果执行成功，函数将返回该函数的返回值。
         *
         * @tparam      T           函数返回值类型
         * @tparam      Args        函数除去函数指针之外的参数类型
         * @param[in]   module_name 共享库的名称
         * @param[in]   func_name   要加载的函数名
         * @param[in]   thread_name 要为执行创建的线程名称
         * @param[in]   runasync    是否异步执行
         * @param[in]   args        函数除去函数指针之外的其他参数
         * @return      如果执行成功，返回该函数的返回值；否则返回 0。
         */
        template <typename T, typename... Args>
        T LoadAndRunFunction(const std::string &module_name, const std::string &func_name,
                             const std::string &thread_name, bool runasync, Args &&...args)
        {
            // 定义函数指针类型
            typedef T (*FunctionPtr)(Args...);

            // 获取共享库句柄和函数指针
            void *handle = GetHandle(module_name);
            auto sym_ptr = dlsym(handle, func_name.c_str());
            if (!sym_ptr)
            {
                spdlog::error("Failed to load symbol {}: {}", func_name, dlerror());
                return static_cast<T>(0);
            }
            FunctionPtr func_ptr = reinterpret_cast<FunctionPtr>(sym_ptr);

            if (runasync)
            {
                // 异步运行函数
                m_ThreadManager->addThread(std::bind(func_ptr, std::forward<Args>(args)...), thread_name);
            }
            else
            {
                // 同步运行函数
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