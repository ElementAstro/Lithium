/*
 * loader.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: C++20 and Modules Loader

**************************************************/

#include "loader.hpp"

#include <memory>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on
#else
#include <dlfcn.h>
#include <link.h>
#include <fstream>
#endif

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/to_string.hpp"

namespace lithium {
ModuleLoader::ModuleLoader(std::string dirName) {
    DLOG_F(INFO, "Module manager {} initialized.", dirName);
}

ModuleLoader::~ModuleLoader() {
    DLOG_F(INFO, "Module manager destroyed.");
    unloadAllModules();
}

auto ModuleLoader::createShared() -> std::shared_ptr<ModuleLoader> {
    DLOG_F(INFO, "Creating shared ModuleLoader with default directory.");
    return std::make_shared<ModuleLoader>("modules");
}

auto ModuleLoader::createShared(std::string dirName)
    -> std::shared_ptr<ModuleLoader> {
    DLOG_F(INFO, "Creating shared ModuleLoader with directory: {}", dirName);
    return std::make_shared<ModuleLoader>(std::move(dirName));
}

auto ModuleLoader::loadModule(const std::string& path,
                              const std::string& name) -> bool {
    DLOG_F(INFO, "Loading module: {} from path: {}", name, path);
    std::unique_lock lock(sharedMutex_);
    if (hasModule(name)) {
        LOG_F(ERROR, "Module {} already loaded", name);
        return false;
    }
    if (!atom::io::isFileExists(path)) {
        LOG_F(ERROR, "Module {} not found at path: {}", name, path);
        return false;
    }
    auto modInfo = std::make_shared<ModuleInfo>();
    try {
        modInfo->mLibrary = std::make_shared<atom::meta::DynamicLibrary>(path);
    } catch (const FFIException& ex) {
        LOG_F(ERROR, "Failed to load module {}: {}", name, ex.what());
        return false;
    }
    modules_[name] = modInfo;
    LOG_F(INFO, "Module {} loaded successfully", name);
    return true;
}

auto ModuleLoader::unloadModule(const std::string& name) -> bool {
    DLOG_F(INFO, "Unloading module: {}", name);
    std::unique_lock lock(sharedMutex_);
    auto it = modules_.find(name);
    if (it != modules_.end()) {
        it->second->mLibrary.reset();
        modules_.erase(it);
        LOG_F(INFO, "Module {} unloaded successfully", name);
        return true;
    }
    LOG_F(ERROR, "Module {} not loaded", name);
    return false;
}

auto ModuleLoader::unloadAllModules() -> bool {
    DLOG_F(INFO, "Unloading all modules.");
    std::unique_lock lock(sharedMutex_);
    modules_.clear();
    LOG_F(INFO, "All modules unloaded");
    return true;
}

auto ModuleLoader::hasModule(const std::string& name) const -> bool {
    DLOG_F(INFO, "Checking if module {} is loaded.", name);
    std::shared_lock lock(sharedMutex_);
    bool result = modules_.find(name) != modules_.end();
    DLOG_F(INFO, "Module {} is {}loaded.", name, result ? "" : "not ");
    return result;
}

auto ModuleLoader::getModule(const std::string& name) const
    -> std::shared_ptr<ModuleInfo> {
    DLOG_F(INFO, "Getting module: {}", name);
    std::shared_lock lock(sharedMutex_);
    auto it = modules_.find(name);
    if (it != modules_.end()) {
        DLOG_F(INFO, "Module {} found.", name);
        return it->second;
    }
    DLOG_F(INFO, "Module {} not found.", name);
    return nullptr;
}

auto ModuleLoader::enableModule(const std::string& name) -> bool {
    DLOG_F(INFO, "Enabling module: {}", name);
    std::unique_lock lock(sharedMutex_);
    auto mod = getModule(name);
    if (mod && !mod->m_enabled.load()) {
        mod->m_enabled.store(true);
        LOG_F(INFO, "Module {} enabled.", name);
        return true;
    }
    LOG_F(WARNING, "Module {} is already enabled or not found.", name);
    return false;
}

auto ModuleLoader::disableModule(const std::string& name) -> bool {
    DLOG_F(INFO, "Disabling module: {}", name);
    std::unique_lock lock(sharedMutex_);
    auto mod = getModule(name);
    if (mod && mod->m_enabled.load()) {
        mod->m_enabled.store(false);
        LOG_F(INFO, "Module {} disabled.", name);
        return true;
    }
    LOG_F(WARNING, "Module {} is already disabled or not found.", name);
    return false;
}

auto ModuleLoader::isModuleEnabled(const std::string& name) const -> bool {
    DLOG_F(INFO, "Checking if module {} is enabled.", name);
    std::shared_lock lock(sharedMutex_);
    auto mod = getModule(name);
    bool result = mod && mod->m_enabled.load();
    DLOG_F(INFO, "Module {} is {}enabled.", name, result ? "" : "not ");
    return result;
}

auto ModuleLoader::getAllExistedModules() const -> std::vector<std::string> {
    DLOG_F(INFO, "Getting all loaded modules.");
    std::shared_lock lock(sharedMutex_);
    std::vector<std::string> moduleNames;
    moduleNames.reserve(modules_.size());
    for (const auto& [name, _] : modules_) {
        moduleNames.push_back(name);
    }
    DLOG_F(INFO, "Loaded modules: {}", atom::utils::toString(moduleNames));
    return moduleNames;
}

auto ModuleLoader::hasFunction(const std::string& name,
                               const std::string& functionName) -> bool {
    DLOG_F(INFO, "Checking if module {} has function: {}", name, functionName);
    std::shared_lock lock(sharedMutex_);
    auto it = modules_.find(name);
    bool result =
        it != modules_.end() && it->second->mLibrary->hasFunction(functionName);
    DLOG_F(INFO, "Module {} {} function: {}", name,
           result ? "has" : "does not have", functionName);
    return result;
}

}  // namespace lithium