/*
 * module_loader.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: C++ and Modules Loader

**************************************************/

#include "loader.hpp"

#include <filesystem>

#include "macro.hpp"
#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <dbghelp.h>
// clang-format on
#else
#include <link.h>
#include <fstream>
#include <iostream>
#include <regex>
#endif
#include "atom/io/io.hpp"

namespace lithium {

ModuleLoader::ModuleLoader(std::string dirName) {
    DLOG_F(INFO, "C++ module manager loaded successfully.");
}

ModuleLoader::~ModuleLoader() {
    if (!modules_.empty()) {
        if (!unloadAllModules()) {
            LOG_F(ERROR, "Failed to unload all modules");
        }
        modules_.clear();
    }
}

auto ModuleLoader::createShared() -> std::shared_ptr<ModuleLoader> {
    return std::make_shared<ModuleLoader>("modules");
}

auto ModuleLoader::createShared(std::string dirName)
    -> std::shared_ptr<ModuleLoader> {
    return std::make_shared<ModuleLoader>(std::move(dirName));
}

auto ModuleLoader::loadModule(const std::string& path,
                              const std::string& name) -> bool {
    std::unique_lock lock(sharedMutex_);
    if (hasModule(name)) {
        LOG_F(ERROR, "Module {} already loaded", name);
        return false;
    }

    if (!std::filesystem::exists(path)) {
        LOG_F(ERROR, "Module {} does not exist", name);
        return false;
    }

    auto modInfo = std::make_shared<ModuleInfo>();
    if (auto* handle = LOAD_LIBRARY(path.c_str()); handle) {
        modInfo->handle = handle;
        modInfo->functions = loadModuleFunctions(name);
        modules_[name] = modInfo;
        DLOG_F(INFO, "Loaded module: {}", name);
        return true;
    }
    LOG_F(ERROR, "Failed to load library {}: {}", path, LOAD_ERROR());
    return false;
}

std::vector<std::unique_ptr<FunctionInfo>> ModuleLoader::loadModuleFunctions(
    const std::string& name) {
    std::vector<std::unique_ptr<FunctionInfo>> funcs;
    std::shared_lock lock(sharedMutex_);

    if (auto it = modules_.find(name); it != modules_.end()) {
        if (void* handle = it->second->handle) {
#ifdef _WIN32
            auto* hModule = static_cast<HMODULE>(handle);
            ULONG size;
            auto* exports =
                (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToDataEx(
                    hModule, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size, NULL);

            if (exports != nullptr) {
                auto* names =
                    (DWORD*)((BYTE*)hModule + exports->AddressOfNames);
                for (DWORD i = 0; i < exports->NumberOfNames; ++i) {
                    char* name = (char*)hModule + names[i];
                    auto func = std::make_unique<FunctionInfo>();
                    func->name = name;
                    func->address = (void*)GetProcAddress(hModule, name);
                    funcs.push_back(std::move(func));
                }
            }
#else
            dl_iterate_phdr(
                [](struct dl_phdr_info* info, size_t, void* data) {
                    auto* funcs = static_cast<
                        std::vector<std::unique_ptr<FunctionInfo>>*>(data);
                    for (int i = 0; i < info->dlpi_phnum; i++) {
                        const ElfW(Phdr)* phdr = &info->dlpi_phdr[i];
                        if (phdr->p_type == PT_DYNAMIC) {
                            ElfW(Dyn)* dyn =
                                (ElfW(Dyn)*)(info->dlpi_addr + phdr->p_vaddr);
                            ElfW(Sym)* symtab = nullptr;
                            char* strtab = nullptr;

                            for (; dyn->d_tag != DT_NULL; dyn++) {
                                if (dyn->d_tag == DT_SYMTAB) {
                                    symtab = (ElfW(Sym)*)(info->dlpi_addr +
                                                          dyn->d_un.d_ptr);
                                } else if (dyn->d_tag == DT_STRTAB) {
                                    strtab = (char*)(info->dlpi_addr +
                                                     dyn->d_un.d_ptr);
                                }
                            }

                            if (symtab && strtab) {
                                for (ElfW(Sym)* sym = symtab; sym->st_name != 0;
                                     sym++) {
                                    if (ELF32_ST_TYPE(sym->st_info) ==
                                        STT_FUNC) {
                                        auto func =
                                            std::make_unique<FunctionInfo>();
                                        func->name = &strtab[sym->st_name];
                                        func->address =
                                            (void*)(info->dlpi_addr +
                                                    sym->st_value);
                                        funcs->push_back(std::move(func));
                                    }
                                }
                            }
                        }
                    }
                    return 0;
                },
                &funcs);
#endif
        }
    }
    return funcs;
}

auto ModuleLoader::unloadModule(const std::string& name) -> bool {
    std::unique_lock lock(sharedMutex_);
    if (hasModule(name)) {
        if (auto result = UNLOAD_LIBRARY(modules_[name]->handle); result == 0) {
            modules_.erase(name);
            return true;
        }
        LOG_F(ERROR, "Failed to unload module {}", name);

    } else {
        LOG_F(ERROR, "Module {} is not loaded", name);
    }
    return false;
}

auto ModuleLoader::unloadAllModules() -> bool {
    std::unique_lock lock(sharedMutex_);
    for (auto& [name, module] : modules_) {
        if (UNLOAD_LIBRARY(module->handle) != 0) {
            LOG_F(ERROR, "Failed to unload module {}", name);
            return false;
        }
    }
    modules_.clear();
    return true;
}

auto ModuleLoader::checkModuleExists(const std::string& name) const -> bool {
    if (void* handle = LOAD_LIBRARY(name.c_str()); handle) {
        UNLOAD_LIBRARY(handle);
        return true;
    }
    return false;
}

auto ModuleLoader::getModule(const std::string& name) const
    -> std::shared_ptr<ModuleInfo> {
    std::shared_lock lock(sharedMutex_);
    if (auto it = modules_.find(name); it != modules_.end()) {
        return it->second;
    }
    return nullptr;
}

auto ModuleLoader::getHandle(const std::string& name) const -> void* {
    std::shared_lock lock(sharedMutex_);
    if (auto it = modules_.find(name); it != modules_.end()) {
        return it->second->handle;
    }
    return nullptr;
}

auto ModuleLoader::hasModule(const std::string& name) const -> bool {
    std::shared_lock lock(sharedMutex_);
    return modules_.contains(name);
}

auto ModuleLoader::enableModule(const std::string& name) -> bool {
    std::unique_lock lock(sharedMutex_);
    if (auto mod = getModule(name); mod && !mod->m_enabled.load()) {
        mod->m_enabled.store(true);
        return true;
    }
    return false;
}

auto ModuleLoader::disableModule(const std::string& name) -> bool {
    std::unique_lock lock(sharedMutex_);
    if (auto mod = getModule(name); mod && mod->m_enabled.load()) {
        mod->m_enabled.store(false);
        return true;
    }
    return false;
}

auto ModuleLoader::isModuleEnabled(const std::string& name) const -> bool {
    std::shared_lock lock(sharedMutex_);
    if (auto mod = getModule(name); mod) {
        return mod->m_enabled.load();
    }
    return false;
}

auto ModuleLoader::getModuleVersion(const std::string& name) -> std::string {
    if (auto getVersionFunc =
            getFunction<std::string (*)()>(name, "getVersion");
        getVersionFunc) {
        return getVersionFunc();
    }
    return "";
}

auto ModuleLoader::getModuleDescription(const std::string& name)
    -> std::string {
    if (auto getDescriptionFunc =
            getFunction<std::string (*)()>(name, "getDescription");
        getDescriptionFunc) {
        return getDescriptionFunc();
    }
    return "";
}

auto ModuleLoader::getModuleAuthor(const std::string& name) -> std::string {
    if (auto getAuthorFunc = getFunction<std::string (*)()>(name, "getAuthor");
        getAuthorFunc) {
        return getAuthorFunc();
    }
    return "";
}

auto ModuleLoader::getModuleLicense(const std::string& name) -> std::string {
    if (auto getLicenseFunc =
            getFunction<std::string (*)()>(name, "getLicense");
        getLicenseFunc) {
        return getLicenseFunc();
    }
    return "";
}

auto ModuleLoader::getModulePath(const std::string& name) -> std::string {
    std::shared_lock lock(sharedMutex_);
    if (auto it = modules_.find(name); it != modules_.end()) {
        Dl_info dlInfo;
        if (dladdr(it->second->handle, &dlInfo) != 0) {
            return dlInfo.dli_fname;
        }
    }
    return "";
}

auto ModuleLoader::getModuleConfig(const std::string& name) -> json {
    if (auto getConfigFunc = getFunction<json (*)()>(name, "getConfig");
        getConfigFunc) {
        return getConfigFunc();
    }
    return {};
}

auto ModuleLoader::getAllExistedModules() const -> std::vector<std::string> {
    std::shared_lock lock(sharedMutex_);
    std::vector<std::string> moduleNames;
    moduleNames.reserve(modules_.size());
    for (const auto& [name, _] : modules_) {
        moduleNames.push_back(name);
    }
    return moduleNames;
}

auto ModuleLoader::hasFunction(const std::string& name,
                               const std::string& functionName) -> bool {
    std::shared_lock lock(sharedMutex_);
    if (auto it = modules_.find(name); it != modules_.end()) {
        return (LOAD_FUNCTION(it->second->handle, functionName.c_str()) !=
                nullptr);
    }
    LOG_F(ERROR, "Failed to find module {}", name);
    return false;
}

}  // namespace lithium
