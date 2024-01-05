/*
 * module_loader.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-3-29

Description: C++ and Modules Loader

**************************************************/

#pragma once

#include <vector>
#include <cstdio>
#include <functional>
#include <atomic>
#include <shared_mutex>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

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

#include "atom/type/json.hpp"
#include "atom/log/loguru.hpp"
#include "error/error_code.hpp"

#include "atom/async/thread.hpp"

using json = nlohmann::json;

namespace Atom::Module
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
    json iterator_modules_dir(const std::string &dir_name);

    /**
     * @brief 用于描述一个模块。
     *
     * 用于描述一个模块。注意模组和插件不是一个东西，模组描述的动态库，而插件是从动态库中提取的指针，是可以操作的对象
     */
    class Mod
    {
        // All of the module information
    public:
        int id;
        std::string name;
        std::string description;
        std::string version;
        std::string status;
        std::string type;
        std::string author;
        std::string license;
        std::string path;
        std::string config_path;
        std::string config_file;
        json config;

        // Module enable status
        std::atomic_bool enabled;

        // Module handle pointer
        void *handle;
    };

    class ModuleLoader
    {
    public:
        /**
         * @brief 使用给定的目录名称和线程管理器参数构造 ModuleLoader 对象。
         *
         * @param dir_name 模块所在的目录名称。
         * @param threadManager 线程管理器的共享指针。
         */
        ModuleLoader(const std::string &dir_name, std::shared_ptr<Atom::Async::ThreadManager> threadManager);

        /**
         * @brief 析构函数，释放 ModuleLoader 对象。
         */
        ~ModuleLoader();

        // -------------------------------------------------------------------
        // Common methods
        // -------------------------------------------------------------------

        /**
         * @brief 使用默认参数创建一个共享的 ModuleLoader 对象。
         * @return 新创建的共享 ModuleLoader 对象。
         */
        static std::shared_ptr<ModuleLoader> createShared();

        /**
         * @brief 使用默认参数创建一个唯一的 ModuleLoader 对象。
         * @return 新创建的唯一 ModuleLoader 对象。
         */
        static std::unique_ptr<ModuleLoader> createUnique();

        /**
         * @brief 使用给定的目录名称和线程管理器参数创建一个共享的 ModuleLoader 指针对象。
         *
         * @param dir_name 模块所在的目录名称。
         * @param threadManager 线程管理器的共享指针。
         * @return 新创建的共享 ModuleLoader 指针对象。
         */
        static std::shared_ptr<ModuleLoader> createShared(const std::string &dir_name, std::shared_ptr<Atom::Async::ThreadManager> threadManager);

        /**
         * @brief 使用给定的目录名称和线程管理器参数创建一个唯一的 ModuleLoader 指针对象。
         *
         * @param dir_name 模块所在的目录名称。
         * @param threadManager 线程管理器的共享指针。
         * @return 新创建的唯一 ModuleLoader 指针对象。
         */
        static std::unique_ptr<ModuleLoader> createUnique(const std::string &dir_name, std::shared_ptr<Atom::Async::ThreadManager> threadManager);

        // -------------------------------------------------------------------
        // Module methods
        // -------------------------------------------------------------------

        /**
         * @brief 根据给定的目录名称加载模块。
         *
         * @param dir_name 模块所在的目录名称。
         * @return 如果成功加载则返回 true，否则返回 false。
         */
        bool LoadOnInit(const std::string &dir_name);

        /**
         * @brief   Loads a dynamic module from the given path.
         *
         * This function loads a dynamic module from the given path. If the loading is successful, it returns true and saves the handle to the module in the modules_ map.
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

        /**
         * @brief 卸载所有动态库
         *
         * @return true 所有动态库卸载成功
         * @return false 所有动态库卸载失败
         */
        bool UnloadAllModules();

        /**
         * @brief 判断指定名称的模块是否存在
         *
         * @param name 模块名称
         * @return true 模块存在
         * @return false 模块不存在
         */
        bool HasModule(const std::string &name) const;

        /**
         * @brief 获取指定名称的模块
         *
         * @param name 模块名称
         * @return std::shared_ptr<Mod> 模块指针
         */
        std::shared_ptr<Mod> GetModule(const std::string &name) const;

        /**
         * @brief 检查指定名称的模块是否存在
         *
         * @param name 模块名称
         * @return true 模块存在
         * @return false 模块不存在
         */
        bool CheckModuleExists(const std::string &name) const;

        /**
         * @brief 允许指定模块
         *
         * @param name 模块名称
         * @return true 成功允许模块
         * @return false 允许模块失败
         */
        bool EnableModule(const std::string &name);

        /**
         * @brief 禁用指定模块
         *
         * @param name 模块名称
         * @return true 成功禁用模块
         * @return false 禁用模块失败
         */
        bool DisableModule(const std::string &name);

        /*
         * @brief 判断指定模块是否被允许
         * @param name 模块名称
         * @return true 指定模块被允许
         * @return false 指定模块未被允许
         */
        bool IsModuleEnabled(const std::string &name) const;

        // -------------------------------------------------------------------
        // Functions methods
        // -------------------------------------------------------------------

        /**
         * @brief 获取指定模块中的函数指针
         *
         * @tparam T 函数指针类型
         * @param name 模块名称
         * @param function_name 函数名称
         * @return T 返回函数指针，如果获取失败则返回nullptr
         */
        template <typename T>
        T GetFunction(const std::string &name, const std::string &function_name);

        /**
         * @brief 判断指定模块中的函数是否存在
         *
         * @param name 模块名称
         * @param function_name 函数名称
         * @return true 函数存在
         * @return false 函数不存在
         */
        bool HasFunction(const std::string &name, const std::string &function_name);

        /**
         * @brief 从指定模块中获取实例对象
         *
         * @tparam T 实例对象类型
         * @param name 模块名称
         * @param config 实例对象的配置参数
         * @param symbol_name 获取实例对象的符号名称
         * @return std::shared_ptr<T> 返回实例对象的智能指针，如果获取失败则返回nullptr
         */
        template <typename T>
        std::shared_ptr<T> GetInstance(const std::string &name, const json &config,
                                       const std::string &symbol_name);

        /**
         * @brief 从指定模块中获取实例对象
         *
         * @tparam T 实例对象类型
         * @param name 模块名称
         * @param config 实例对象的配置参数
         * @param instance_function_name 获取实例对象的函数名称
         * @return std::unique_ptr<T> 返回实例对象的智能指针，如果获取失败则返回nullptr
         */
        template <typename T>
        std::unique_ptr<T> GetUniqueInstance(const std::string &name, const json &config,
                                            const std::string &instance_function_name);

        /**
         * @brief 获取指定模块的任务实例指针
         *
         * @tparam T 任务类型
         * @param name 模块名称
         * @param config 配置信息
         * @param instance_function_name 实例化函数名称
         * @return std::shared_ptr<T> 任务实例指针
         * std::shared_ptr<BasicTask> task = GetInstancePointer<BasicTask>(name, config, "GetTaskInstance");
         * std::shared_ptr<Device> device = GetInstancePointer<Device>(name, config, "GetDeviceInstance");
         * std::shared_ptr<Plugin> plugin = GetInstancePointer<Plugin>(name, config, "GetPluginInstance");
         */
        template <typename T>
        std::shared_ptr<T> GetInstancePointer(const std::string &name, const json &config, const std::string &instance_function_name);

    public:
        /**
         * @brief 获取给定名称的句柄。
         *
         * @param name 句柄名称。
         * @return 对应名称的句柄指针，如果未找到则返回空指针。
         * @note 该函数不检查模块是否被允许。这个函数的使用其实是很危险的，不建议暴露到模组或者脚本中被随意调用。
         */
        void *GetHandle(const std::string &name) const;

        // ---------------------------------------------------------------------
        // Get Module Info
        // ---------------------------------------------------------------------

        /**
         * @brief 获取指定模块的版本号
         * @param name 模块名称
         * @return 模块版本号
         */
        std::string GetModuleVersion(const std::string &name);

        /**
         * @brief 获取指定模块的描述信息
         * @param name 模块名称
         * @return 模块描述信息
         */
        std::string GetModuleDescription(const std::string &name);

        /**
         * @brief 获取指定模块的作者信息
         * @param name 模块名称
         * @return 模块作者信息
         */
        std::string GetModuleAuthor(const std::string &name);

        /**
         * @brief 获取指定模块的许可证信息
         * @param name 模块名称
         * @return 模块许可证信息
         */
        std::string GetModuleLicense(const std::string &name);

        /**
         * @brief 获取给定模块名称的模块路径。
         *
         * @param name 模块名称。
         * @return 对应模块名称的模块路径。
         */
        std::string GetModulePath(const std::string &name);

        /**
         * @brief 获取给定模块名称的模块配置。
         *
         * @param name 模块名称。
         * @return 对应模块名称的模块配置。
         */
        json GetModuleConfig(const std::string &name);

        /**
         * @brief 获取所有存在的模块名称。
         *
         * @return 存在的模块名称的向量。
         */
        const std::vector<std::string> GetAllExistedModules() const;

    private:
#if ENABLE_FASTHASH
        emhash8::HashMap<std::string, std::shared_ptr<Mod>> modules_; // 模块哈希表
#else
        std::unordered_map<std::string, std::shared_ptr<Mod>> modules_; // 模块哈希表
#endif
        // Injected Thread Manager
        std::shared_ptr<Atom::Async::ThreadManager> m_ThreadManager;

        mutable std::shared_mutex m_SharedMutex;
    };

    template <typename T>
    T ModuleLoader::GetFunction(const std::string &name, const std::string &function_name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        auto handle_it = modules_.find(name);
        if (handle_it == modules_.end())
        {
            LOG_F(ERROR, "Failed to find module {}", name);
            return nullptr;
        }

        auto func_ptr = reinterpret_cast<T>(LOAD_FUNCTION(handle_it->second->handle, function_name.c_str()));

        if (!func_ptr)
        {
            LOG_F(ERROR, "Failed to get symbol {} from module {}: {}", function_name, name, dlerror());
            return nullptr;
        }

        return func_ptr;
    }

    template <typename T>
    std::shared_ptr<T> ModuleLoader::GetInstance(const std::string &name, const json &config,
                                                 const std::string &symbol_name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        if (!HasModule(name))
        {
            LOG_F(ERROR, "Failed to find module {}", name);
            return nullptr;
        }
        auto get_instance_func = GetFunction<std::shared_ptr<T> (*)(const json &)>(name, symbol_name);
        if (!get_instance_func)
        {
            LOG_F(ERROR, "Failed to get symbol {} from module {}: {}", symbol_name, name, dlerror());
            return nullptr;
        }

        return get_instance_func(config);
    }

    template <typename T>
    std::unique_ptr<T> ModuleLoader::GetUniqueInstance(const std::string &name, const json &config,
                                                       const std::string &instance_function_name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        if (!HasModule(name))
        {
            LOG_F(ERROR, "Failed to find module {}", name);
            return nullptr;
        }
        auto get_instance_func = GetFunction<std::unique_ptr<T> (*)(const json &)>(name, instance_function_name);
        if (!get_instance_func)
        {
            LOG_F(ERROR, "Failed to get symbol {} from module {}: {}", instance_function_name, name, dlerror());
            return nullptr;
        }

        return get_instance_func(config);
    }

    template <typename T>
    std::shared_ptr<T> ModuleLoader::GetInstancePointer(const std::string &name, const json &config, const std::string &instance_function_name)
    {
        return GetInstance<T>(name, config, instance_function_name);
    }
}