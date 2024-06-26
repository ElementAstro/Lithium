/*
 * module_loader.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: C++ and Modules Loader

**************************************************/

#ifndef LITHIUM_ADDON_LOADER_HPP
#define LITHIUM_ADDON_LOADER_HPP

#include <atomic>
#include <cstdio>
#include <functional>
#include <shared_mutex>
#include <vector>

#include "module.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

// Max: There we use dlfcn-win to load shared library on windows.So we don't
// need to use Windows Native API.
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
#include <dirent.h>
#include <dlfcn.h>
#include <unistd.h>
#define MODULE_HANDLE void *
#define LOAD_LIBRARY(p) dlopen(p, RTLD_LAZY | RTLD_GLOBAL)
#define LOAD_SHARED_LIBRARY(file, size) dlopen(nullptr, RTLD_LAZY | RTLD_GLOBAL)
#define UNLOAD_LIBRARY(p) dlclose(p)
#define LOAD_ERROR() dlerror()
#define LOAD_FUNCTION(handle, name) dlsym(handle, name)
/*
#endif
*/

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "error/error_code.hpp"

using json = nlohmann::json;

namespace lithium {
/**
 * @brief 模块加载器类。
 *
 * @details 模块加载器类，用于加载模块，提供模块的注册和卸载功能。
 */
class ModuleLoader {
public:
    /**
     * @brief 使用给定的目录名称和线程管理器参数构造 ModuleLoader 对象。
     *
     * @param dir_name 模块所在的目录名称。
     * @param threadManager 线程管理器的共享指针。
     */
    explicit ModuleLoader(const std::string &dir_name);

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
     * @brief 使用给定的目录名称和线程管理器参数创建一个共享的 ModuleLoader
     * 指针对象。
     *
     * @param dir_name 模块所在的目录名称。
     * @param threadManager 线程管理器的共享指针。
     * @return 新创建的共享 ModuleLoader 指针对象。
     */
    static std::shared_ptr<ModuleLoader> createShared(
        const std::string &dir_name);

    // -------------------------------------------------------------------
    // Module methods
    // -------------------------------------------------------------------

    /**
     * @brief   Loads a dynamic module from the given path.
     *
     * This function loads a dynamic module from the given path. If the loading
     * is successful, it returns true and saves the handle to the module in the
     * modules_ map. If the loading fails, it returns false and logs an error
     * message.
     *
     * @param[in]   path    The path of the dynamic module to load.
     * @param[in]   name    The name of the dynamic module.
     * @return      true if the loading is successful, false otherwise.
     */
    bool LoadModule(const std::string &path, const std::string &name);

    /**
     * @brief   Loads all functions from the given module.
     *
     * This function loads all functions from the given module. If the loading
     * is successful, it returns a vector of FunctionInfo objects. If the
     * loading fails, it returns an empty vector and logs an error message.
     *
     * @param[in]   name    The name of the dynamic module.
     * @return      A vector of FunctionInfo objects.
     */
    std::vector<std::unique_ptr<FunctionInfo>> loadModuleFunctions(
        const std::string &name);

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
     * @return std::shared_ptr<ModuleInfo> 模块指针
     */
    std::shared_ptr<ModuleInfo> GetModule(const std::string &name) const;

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
     * @return std::shared_ptr<T>
     * 返回实例对象的智能指针，如果获取失败则返回nullptr
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
     * @return std::unique_ptr<T>
     * 返回实例对象的智能指针，如果获取失败则返回nullptr
     */
    template <typename T>
    std::unique_ptr<T> GetUniqueInstance(
        const std::string &name, const json &config,
        const std::string &instance_function_name);

    /**
     * @brief 获取指定模块的任务实例指针
     *
     * @tparam T 任务类型
     * @param name 模块名称
     * @param config 配置信息
     * @param instance_function_name 实例化函数名称
     * @return std::shared_ptr<T> 任务实例指针
     * std::shared_ptr<BasicTask> task = GetInstancePointer<BasicTask>(name,
     * config, "GetTaskInstance"); std::shared_ptr<Device> device =
     * GetInstancePointer<Device>(name, config, "GetDeviceInstance");
     * std::shared_ptr<Plugin> plugin = GetInstancePointer<Plugin>(name, config,
     * "GetPluginInstance");
     */
    template <typename T>
    std::shared_ptr<T> GetInstancePointer(
        const std::string &name, const json &config,
        const std::string &instance_function_name);

public:
    /**
     * @brief 获取给定名称的句柄。
     *
     * @param name 句柄名称。
     * @return 对应名称的句柄指针，如果未找到则返回空指针。
     * @note
     * 该函数不检查模块是否被允许。这个函数的使用其实是很危险的，不建议暴露到模组或者脚本中被随意调用。
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
    emhash8::HashMap<std::string, std::shared_ptr<ModuleInfo>>
        modules_;  // 模块哈希表
#else
    std::unordered_map<std::string, std::shared_ptr<ModuleInfo>>
        modules_;  // 模块哈希表
#endif

    mutable std::shared_mutex m_SharedMutex;
};
}  // namespace lithium

#include "loader.inl"

#endif
