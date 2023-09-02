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

Description: C++ and Modules Loader

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

#include "nlohmann/json_fwd.hpp"
#include "loguru/loguru.hpp"

#include "thread/thread.hpp"
#include "liproperty/task/task.hpp"
#include "Hydrogen/core/device.hpp"
#include "error/error_code.hpp"

namespace tl
{
    template <class T, class E>
    class expected;
}

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
    nlohmann::json iterator_modules_dir(const std::string &dir_name);

    class ModuleLoader
    {
    public:
        ModuleLoader();
        ModuleLoader(const std::string &dir_name);
        ModuleLoader(std::shared_ptr<Thread::ThreadManager> threadManager);
        ModuleLoader(const std::string &dir_name, std::shared_ptr<Thread::ThreadManager> threadManager);

        ~ModuleLoader();

    public:
        static std::shared_ptr<ModuleLoader> createShared();
        static std::shared_ptr<ModuleLoader> createShared(const std::string &dir_name);
        static std::shared_ptr<ModuleLoader> createShared(std::shared_ptr<Thread::ThreadManager> threadManager);
        static std::shared_ptr<ModuleLoader> createShared(const std::string &dir_name, std::shared_ptr<Thread::ThreadManager> threadManager);

        bool LoadOnInit(const std::string &dir_name);

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

        bool CheckModuleExists(const std::string &name) const;

        /**
         * @brief 允许指定模块
         *
         * @param module_name 模块名称
         * @return true 成功允许模块
         * @return false 允许模块失败
         */
        bool EnableModule(const std::string &module_name);

        /**
         * @brief 禁用指定模块
         *
         * @param module_name 模块名称
         * @return true 成功禁用模块
         * @return false 禁用模块失败
         */
        bool DisableModule(const std::string &module_name);

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
         * @brief 获取指定模块的任务实例指针
         *
         * @tparam T 任务类型
         * @param module_name 模块名称
         * @param config 配置信息
         * @param instance_function_name 实例化函数名称
         * @return std::shared_ptr<T> 任务实例指针
         * std::shared_ptr<BasicTask> task = GetInstancePointer<BasicTask>(module_name, config, "GetTaskInstance");
         * std::shared_ptr<Device> device = GetInstancePointer<Device>(module_name, config, "GetDeviceInstance");
         * std::shared_ptr<Plugin> plugin = GetInstancePointer<Plugin>(module_name, config, "GetPluginInstance");
         */
        template <typename T>
        std::shared_ptr<T> GetInstancePointer(const std::string &module_name, const nlohmann::json &config, const std::string &instance_function_name)
        {
            return GetInstance<T>(module_name, config, instance_function_name);
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

        std::string GetModulePath(const std::string& module_name);

        const std::vector<std::string> GetAllExistedModules() const;

    private:
        std::unordered_map<std::string, void *> handles_;
        std::shared_ptr<Thread::ThreadManager> m_ThreadManager;
        std::unordered_map<std::string, std::string> disabled_modules_;
    };
}