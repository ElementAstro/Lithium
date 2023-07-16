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

/*
#ifdef _WIN32
#include <windows.h>
#define MODULE_HANDLE HMODULE
#define LOAD_LIBRARY(p) LoadLibrary(p)
#define LOAD_SHARED_LIBRARY(file, size) LoadLibraryA(NULL)
#define UNLOAD_LIBRARY(p) FreeLibrary(p)
#define LOAD_ERROR() GetLastError()
#define LOAD_FUNCTION(handle, name) GetProcAddress(handle, name)
#elif defined(__APPLE__) || defined(__linux__)
*/
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#define MODULE_HANDLE void *
#define LOAD_LIBRARY(p) dlopen(p, RTLD_NOW | RTLD_GLOBAL)
#define LOAD_SHARED_LIBRARY(file, size) dlopen(nullptr, RTLD_NOW | RTLD_GLOBAL)
#define UNLOAD_LIBRARY(p) dlclose(p)
#define LOAD_ERROR() dlerror()
#define LOAD_FUNCTION(handle, name) dlsym(handle, name)
/*
#endif
*/

#include <nlohmann/json.hpp>

#include "thread/thread.hpp"
#include "task/task.hpp"
#include "device/device.hpp"

#include "loguru/loguru.hpp"

namespace Lithium
{

    /**
     * @brief Traverse the "modules" directory and create a JSON object containing the information of all modules.
     *
     * This function iterates through the "modules" directory and its subdirectories, creates a JSON object for each subdirectory that
     * contains an "info.json" configuration file, and stores the module's name, version, author, license, description, path, and configuration
     * file path in the JSON object. It returns a JSON object containing all module information, or an error message if it fails to iterate
     * the directories or encounters any exception.
     *
     * 遍历“modules”目录并创建一个包含所有模块信息的JSON对象。
     *
     * 该函数遍历“modules”目录及其子目录，为每个包含“info.json”配置文件的子目录创建一个JSON对象，并将模块的名称、版本、作者、许可证、描述、路径和配置文件路径存储在JSON对象中。
     * 如果无法遍历目录或遇到任何异常，则返回一个包含所有模块信息的JSON对象，否则返回错误消息。
     *
     * @return json - A JSON object containing the module information or an error message.
     *                包含模块信息或错误消息的JSON对象。
     */
    nlohmann::json iterator_modules_dir();

    class ModuleLoader
    {
    public:
        ModuleLoader();
        ~ModuleLoader();
        /**
         * @brief   Loads a dynamic module from the given path.
         *
         * This function loads a dynamic module from the given path. If the loading is successful, it returns true and saves the handle to the module in the handles_ map.
         * If the loading fails, it returns false and logs an error message.
         *
         * @param[in]   path    The path of the dynamic module to load.
         * @param[in]   name    The name of the dynamic module.
         * @return      true if the loading is successful, false otherwise.
         */
        bool LoadModule(const std::string &path, const std::string &name);
        /**
         * @brief 卸载指定名称的动态库
         *
         * @param filename [in] 要卸载的动态库的文件名（包括扩展名）
         * @return true 动态库卸载成功
         * @return false 动态库卸载失败
         */
        bool UnloadModule(const std::string &name);

        bool HasModule(const std::string &name) const;

        bool CheckModuleExists(const std::string &moduleName) const;

        /**
         * @brief 获取指定模块中的函数指针
         *
         * @tparam T 函数指针类型
         * @param module_name 模块名称
         * @param function_name 函数名称
         * @return T 返回函数指针，如果获取失败则返回nullptr
         */
        template <typename T>
        T GetFunction(const std::string &module_name, const std::string &function_name)
        {
            auto handle_it = handles_.find(module_name);
            if (handle_it == handles_.end())
            {
                LOG_F(ERROR, "Failed to find module %s", module_name.c_str());
                return nullptr;
            }

            auto func_ptr = reinterpret_cast<T>(LOAD_FUNCTION(handle_it->second, function_name.c_str()));

            if (!func_ptr)
            {
                LOG_F(ERROR, "Failed to get symbol %s from module %s: %s", function_name.c_str(), module_name.c_str(), dlerror());
                return nullptr;
            }

            return func_ptr;
        }

        /**
         * @brief 从指定模块中获取实例对象
         *
         * @tparam T 实例对象类型
         * @param module_name 模块名称
         * @param config 实例对象的配置参数
         * @param symbol_name 获取实例对象的符号名称
         * @return std::shared_ptr<T> 返回实例对象的智能指针，如果获取失败则返回nullptr
         */
        template <typename T>
        std::shared_ptr<T> GetInstance(const std::string &module_name, const nlohmann::json &config,
                                       const std::string &symbol_name)
        {
            auto handle_it = handles_.find(module_name);
            if (handle_it == handles_.end())
            {
                LOG_F(ERROR, "Failed to find module %s", module_name.c_str());
                return nullptr;
            }

            auto get_instance_func = GetFunction<std::shared_ptr<T> (*)(const nlohmann::json &)>(module_name, symbol_name);
            if (!get_instance_func)
            {
                LOG_F(ERROR, "Failed to get symbol %s from module %s: %s", symbol_name.c_str(), module_name.c_str(), dlerror());
                return nullptr;
            }

            return get_instance_func(config);
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
                LOG_F(ERROR, "Failed to load symbol %s: %s", func_name.c_str(), dlerror());
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
                LOG_F(ERROR, "Failed to load symbol %s: %s", func_name.c_str(), dlerror());
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
                // spdlog::debug("Simple not async function is executed successfully!");
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

    private:
        std::unordered_map<std::string, void *> handles_;

        std::shared_ptr<Thread::ThreadManager> m_ThreadManager;
    };
}