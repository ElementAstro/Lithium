/*
 * loader.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: C++20 and Modules Loader

**************************************************/

#ifndef LITHIUM_ADDON_LOADER_HPP
#define LITHIUM_ADDON_LOADER_HPP

#include <cstdio>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "atom/function/ffi.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json_fwd.hpp"

#include "module.hpp"

using json = nlohmann::json;

#define MODULE_HANDLE void*

namespace lithium {
class ModuleLoader {
public:
    explicit ModuleLoader(std::string dirName);
    ~ModuleLoader();

    static auto createShared() -> std::shared_ptr<ModuleLoader>;
    static auto createShared(std::string dirName)
        -> std::shared_ptr<ModuleLoader>;

    auto loadModule(const std::string& path, const std::string& name) -> bool;
    auto unloadModule(const std::string& name) -> bool;
    auto unloadAllModules() -> bool;
    auto hasModule(const std::string& name) const -> bool;
    auto getModule(const std::string& name) const
        -> std::shared_ptr<ModuleInfo>;
    auto enableModule(const std::string& name) -> bool;
    auto disableModule(const std::string& name) -> bool;
    auto isModuleEnabled(const std::string& name) const -> bool;
    auto getAllExistedModules() const -> std::vector<std::string>;

    template <typename T>
    auto getFunction(const std::string& name,
                     const std::string& functionName) -> std::function<T>;

    template <typename T>
    auto getInstance(const std::string& name, const json& config,
                     const std::string& symbolName) -> std::shared_ptr<T>;

    template <typename T>
    auto getUniqueInstance(const std::string& name, const json& config,
                           const std::string& instanceFunctionName)
        -> std::unique_ptr<T>;

    template <typename T>
    auto getInstancePointer(const std::string& name, const json& config,
                            const std::string& instanceFunctionName)
        -> std::shared_ptr<T>;

    auto hasFunction(const std::string& name,
                     const std::string& functionName) -> bool;

private:
    std::unordered_map<std::string, std::shared_ptr<ModuleInfo>> modules_;
    mutable std::shared_mutex sharedMutex_;

    auto loadModuleFunctions(const std::string& name)
        -> std::vector<std::unique_ptr<FunctionInfo>>;
    auto getHandle(const std::string& name) const
        -> std::shared_ptr<atom::meta::DynamicLibrary>;
    auto checkModuleExists(const std::string& name) const -> bool;
};

template <typename T>
auto ModuleLoader::getFunction(const std::string& name,
                               const std::string& functionName)
    -> std::function<T> {
    std::shared_lock lock(sharedMutex_);
    auto it = modules_.find(name);
    if (it == modules_.end()) {
        LOG_F(ERROR, "Module {} not found", name);
        return nullptr;
    }

    try {
        return it->second->mLibrary->getFunction<T>(functionName);
    } catch (const FFIException& e) {
        LOG_F(ERROR, "Failed to load function {} from module {}: {}",
              functionName, name, e.what());
    }
    return nullptr;
}

template <typename T>
auto ModuleLoader::getInstance(const std::string& name, const json& config,
                               const std::string& symbolName)
    -> std::shared_ptr<T> {
    if (auto getInstanceFunc =
            getFunction<std::shared_ptr<T>(const json&)>(name, symbolName);
        getInstanceFunc) {
        return getInstanceFunc(config);
    }
    return nullptr;
}

template <typename T>
auto ModuleLoader::getUniqueInstance(
    const std::string& name, const json& config,
    const std::string& instanceFunctionName) -> std::unique_ptr<T> {
    if (auto getInstanceFunc = getFunction<std::unique_ptr<T>(const json&)>(
            name, instanceFunctionName);
        getInstanceFunc) {
        return getInstanceFunc(config);
    }
    return nullptr;
}

template <typename T>
auto ModuleLoader::getInstancePointer(
    const std::string& name, const json& config,
    const std::string& instanceFunctionName) -> std::shared_ptr<T> {
    return getInstance<T>(name, config, instanceFunctionName);
}

}  // namespace lithium

#endif  // LITHIUM_ADDON_LOADER_HPP
