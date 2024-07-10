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

#include <dlfcn.h>
#include <cstdio>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "module.hpp"

using json = nlohmann::json;

#define MODULE_HANDLE void*
#define LOAD_LIBRARY(p) dlopen(p, RTLD_LAZY | RTLD_GLOBAL)
#define UNLOAD_LIBRARY(p) dlclose(p)
#define LOAD_ERROR() dlerror()
#define LOAD_FUNCTION(handle, name) dlsym(handle, name)

namespace lithium {
class ModuleLoader {
public:
    explicit ModuleLoader(const std::string& dir_name);
    ~ModuleLoader();

    static std::shared_ptr<ModuleLoader> createShared();
    static std::shared_ptr<ModuleLoader> createShared(
        const std::string& dir_name);

    bool LoadModule(const std::string& path, const std::string& name);
    bool UnloadModule(const std::string& name);
    bool UnloadAllModules();
    bool HasModule(const std::string& name) const;
    std::shared_ptr<ModuleInfo> GetModule(const std::string& name) const;
    bool EnableModule(const std::string& name);
    bool DisableModule(const std::string& name);
    bool IsModuleEnabled(const std::string& name) const;
    std::string GetModuleVersion(const std::string& name);
    std::string GetModuleDescription(const std::string& name);
    std::string GetModuleAuthor(const std::string& name);
    std::string GetModuleLicense(const std::string& name);
    std::string GetModulePath(const std::string& name);
    json GetModuleConfig(const std::string& name);
    const std::vector<std::string> GetAllExistedModules() const;

    template <typename T>
    T GetFunction(const std::string& name, const std::string& function_name);

    template <typename T>
    std::shared_ptr<T> GetInstance(const std::string& name, const json& config,
                                   const std::string& symbol_name);

    template <typename T>
    std::unique_ptr<T> GetUniqueInstance(
        const std::string& name, const json& config,
        const std::string& instance_function_name);

    template <typename T>
    std::shared_ptr<T> GetInstancePointer(
        const std::string& name, const json& config,
        const std::string& instance_function_name);

private:
    std::unordered_map<std::string, std::shared_ptr<ModuleInfo>> modules_;
    mutable std::shared_mutex m_SharedMutex;

    std::vector<std::unique_ptr<FunctionInfo>> loadModuleFunctions(
        const std::string& name);
    void* GetHandle(const std::string& name) const;
    bool CheckModuleExists(const std::string& name) const;
    bool HasFunction(const std::string& name, const std::string& function_name);
};

template <typename T>
T ModuleLoader::GetFunction(const std::string& name,
                            const std::string& function_name) {
    std::shared_lock lock(m_SharedMutex);
    if (auto it = modules_.find(name); it != modules_.end()) {
        if (auto func_ptr = reinterpret_cast<T>(
                LOAD_FUNCTION(it->second->handle, function_name.c_str()));
            func_ptr) {
            return func_ptr;
        }
        LOG_F(ERROR, "Failed to get symbol {} from module {}: {}",
              function_name, name, LOAD_ERROR());
    } else {
        LOG_F(ERROR, "Failed to find module {}", name);
    }
    return nullptr;
}

template <typename T>
std::shared_ptr<T> ModuleLoader::GetInstance(const std::string& name,
                                             const json& config,
                                             const std::string& symbol_name) {
    if (auto get_instance_func =
            GetFunction<std::shared_ptr<T> (*)(const json&)>(name, symbol_name);
        get_instance_func) {
        return get_instance_func(config);
    }
    return nullptr;
}

template <typename T>
std::unique_ptr<T> ModuleLoader::GetUniqueInstance(
    const std::string& name, const json& config,
    const std::string& instance_function_name) {
    if (auto get_instance_func =
            GetFunction<std::unique_ptr<T> (*)(const json&)>(
                name, instance_function_name);
        get_instance_func) {
        return get_instance_func(config);
    }
    return nullptr;
}

template <typename T>
std::shared_ptr<T> ModuleLoader::GetInstancePointer(
    const std::string& name, const json& config,
    const std::string& instance_function_name) {
    return GetInstance<T>(name, config, instance_function_name);
}

}  // namespace lithium

#endif  // LITHIUM_ADDON_LOADER_HPP
