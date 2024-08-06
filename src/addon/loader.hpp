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
    explicit ModuleLoader(std::string dirName);
    ~ModuleLoader();

    static std::shared_ptr<ModuleLoader> createShared();
    static std::shared_ptr<ModuleLoader> createShared(std::string dirName);

    bool loadModule(const std::string& path, const std::string& name);
    bool unloadModule(const std::string& name);
    bool unloadAllModules();
    bool hasModule(const std::string& name) const;
    std::shared_ptr<ModuleInfo> getModule(const std::string& name) const;
    bool enableModule(const std::string& name);
    bool disableModule(const std::string& name);
    bool isModuleEnabled(const std::string& name) const;
    std::string getModuleVersion(const std::string& name);
    std::string getModuleDescription(const std::string& name);
    std::string getModuleAuthor(const std::string& name);
    std::string getModuleLicense(const std::string& name);
    std::string getModulePath(const std::string& name);
    json getModuleConfig(const std::string& name);
    std::vector<std::string> getAllExistedModules() const;

    template <typename T>
    T getFunction(const std::string& name, const std::string& functionName);

    template <typename T>
    std::shared_ptr<T> getInstance(const std::string& name, const json& config,
                                   const std::string& symbolName);

    template <typename T>
    std::unique_ptr<T> getUniqueInstance(
        const std::string& name, const json& config,
        const std::string& instanceFunctionName);

    template <typename T>
    std::shared_ptr<T> getInstancePointer(
        const std::string& name, const json& config,
        const std::string& instanceFunctionName);

    bool hasFunction(const std::string& name, const std::string& functionName);

private:
    std::unordered_map<std::string, std::shared_ptr<ModuleInfo>> modules_;
    mutable std::shared_mutex sharedMutex_;

    std::vector<std::unique_ptr<FunctionInfo>> loadModuleFunctions(
        const std::string& name);
    void* getHandle(const std::string& name) const;
    bool checkModuleExists(const std::string& name) const;
};

template <typename T>
T ModuleLoader::getFunction(const std::string& name,
                            const std::string& functionName) {
    std::shared_lock lock(sharedMutex_);
    if (auto it = modules_.find(name); it != modules_.end()) {
        if (auto funcPtr = reinterpret_cast<T>(
                LOAD_FUNCTION(it->second->handle, functionName.c_str()));
            funcPtr) {
            return funcPtr;
        }
        LOG_F(ERROR, "Failed to get symbol {} from module {}: {}", functionName,
              name, LOAD_ERROR());
    } else {
        LOG_F(ERROR, "Failed to find module {}", name);
    }
    return nullptr;
}

template <typename T>
auto ModuleLoader::getInstance(const std::string& name, const json& config,
                               const std::string& symbolName)
    -> std::shared_ptr<T> {
    if (auto getInstanceFunc =
            getFunction<std::shared_ptr<T> (*)(const json&)>(name, symbolName);
        getInstanceFunc) {
        return getInstanceFunc(config);
    }
    if (auto getInstanceFunc =
            getFunction<std::shared_ptr<T> (*)()>(name, symbolName);
        getInstanceFunc) {
        return getInstanceFunc();
    }

    return nullptr;
}

template <typename T>
std::unique_ptr<T> ModuleLoader::getUniqueInstance(
    const std::string& name, const json& config,
    const std::string& instanceFunctionName) {
    if (auto getInstanceFunc = getFunction<std::unique_ptr<T> (*)(const json&)>(
            name, instanceFunctionName);
        getInstanceFunc) {
        return getInstanceFunc(config);
    }
    return nullptr;
}

template <typename T>
std::shared_ptr<T> ModuleLoader::getInstancePointer(
    const std::string& name, const json& config,
    const std::string& instanceFunctionName) {
    return getInstance<T>(name, config, instanceFunctionName);
}

}  // namespace lithium

#endif  // LITHIUM_ADDON_LOADER_HPP
