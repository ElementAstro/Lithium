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
#include <fstream>
#include <iostream>
#include <regex>
#include "macro.hpp"
#ifdef _WIN32
#include <windows.h>
#else
#include <link.h>
#endif
#include "atom/io/io.hpp"

namespace lithium {

ModuleLoader::ModuleLoader(const std::string& dir_name) {
    DLOG_F(INFO, "C++ module manager loaded successfully.");
}

ModuleLoader::~ModuleLoader() {
    if (!modules_.empty()) {
        if (!UnloadAllModules()) {
            LOG_F(ERROR, "Failed to unload all modules");
        }
        modules_.clear();
    }
}

std::shared_ptr<ModuleLoader> ModuleLoader::createShared() {
    return std::make_shared<ModuleLoader>("modules");
}

std::shared_ptr<ModuleLoader> ModuleLoader::createShared(
    const std::string& dir_name) {
    return std::make_shared<ModuleLoader>(dir_name);
}

bool ModuleLoader::LoadModule(const std::string& path,
                              const std::string& name) {
    std::unique_lock lock(m_SharedMutex);
    if (HasModule(name)) {
        LOG_F(ERROR, "Module {} already loaded", name);
        return false;
    }

    if (!atom::io::isFileExists(path)) {
        LOG_F(ERROR, "Module {} does not exist", name);
        return false;
    }

    auto mod_info = std::make_shared<ModuleInfo>();
    if (auto handle = LOAD_LIBRARY(path.c_str()); handle) {
        mod_info->handle = handle;
        mod_info->functions = loadModuleFunctions(name);
        modules_[name] = mod_info;
        DLOG_F(INFO, "Loaded module: {}", name);
        return true;
    } else {
        LOG_F(ERROR, "Failed to load library {}: {}", path, LOAD_ERROR());
    }
    return false;
}

std::vector<std::unique_ptr<FunctionInfo>> ModuleLoader::loadModuleFunctions(
    const std::string& name) {
    std::vector<std::unique_ptr<FunctionInfo>> funcs;
    std::shared_lock lock(m_SharedMutex);

    if (auto it = modules_.find(name); it != modules_.end()) {
        if (void* handle = it->second->handle) {
#ifdef _WIN32
            HMODULE hModule = static_cast<HMODULE>(handle);
            ULONG size;
            PIMAGE_EXPORT_DIRECTORY exports =
                (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToDataEx(
                    hModule, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size, NULL);

            if (exports) {
                DWORD* names =
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
                [](struct dl_phdr_info* info, ATOM_UNUSED size_t size, void* data) {
                    auto *funcs = static_cast<
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

bool ModuleLoader::UnloadModule(const std::string& name) {
    std::unique_lock lock(m_SharedMutex);
    if (HasModule(name)) {
        if (auto result = UNLOAD_LIBRARY(modules_[name]->handle); result == 0) {
            modules_.erase(name);
            return true;
        } else {
            LOG_F(ERROR, "Failed to unload module {}", name);
        }
    } else {
        LOG_F(ERROR, "Module {} is not loaded", name);
    }
    return false;
}

bool ModuleLoader::UnloadAllModules() {
    std::unique_lock lock(m_SharedMutex);
    for (auto& [name, module] : modules_) {
        if (UNLOAD_LIBRARY(module->handle) != 0) {
            LOG_F(ERROR, "Failed to unload module {}", name);
            return false;
        }
    }
    modules_.clear();
    return true;
}

bool ModuleLoader::CheckModuleExists(const std::string& name) const {
    if (void* handle = LOAD_LIBRARY(name.c_str()); handle) {
        UNLOAD_LIBRARY(handle);
        return true;
    }
    return false;
}

std::shared_ptr<ModuleInfo> ModuleLoader::GetModule(
    const std::string& name) const {
    std::shared_lock lock(m_SharedMutex);
    if (auto it = modules_.find(name); it != modules_.end()) {
        return it->second;
    }
    return nullptr;
}

void* ModuleLoader::GetHandle(const std::string& name) const {
    std::shared_lock lock(m_SharedMutex);
    if (auto it = modules_.find(name); it != modules_.end()) {
        return it->second->handle;
    }
    return nullptr;
}

bool ModuleLoader::HasModule(const std::string& name) const {
    std::shared_lock lock(m_SharedMutex);
    return modules_.contains(name);
}

bool ModuleLoader::EnableModule(const std::string& name) {
    std::unique_lock lock(m_SharedMutex);
    if (auto mod = GetModule(name); mod && !mod->m_enabled.load()) {
        mod->m_enabled.store(true);
        return true;
    }
    return false;
}

bool ModuleLoader::DisableModule(const std::string& name) {
    std::unique_lock lock(m_SharedMutex);
    if (auto mod = GetModule(name); mod && mod->m_enabled.load()) {
        mod->m_enabled.store(false);
        return true;
    }
    return false;
}

bool ModuleLoader::IsModuleEnabled(const std::string& name) const {
    std::shared_lock lock(m_SharedMutex);
    if (auto mod = GetModule(name); mod) {
        return mod->m_enabled.load();
    }
    return false;
}

std::string ModuleLoader::GetModuleVersion(const std::string& name) {
    if (auto get_version_func =
            GetFunction<std::string (*)()>(name, "GetVersion");
        get_version_func) {
        return get_version_func();
    }
    return "";
}

std::string ModuleLoader::GetModuleDescription(const std::string& name) {
    if (auto get_description_func =
            GetFunction<std::string (*)()>(name, "GetDescription");
        get_description_func) {
        return get_description_func();
    }
    return "";
}

std::string ModuleLoader::GetModuleAuthor(const std::string& name) {
    if (auto get_author_func =
            GetFunction<std::string (*)()>(name, "GetAuthor");
        get_author_func) {
        return get_author_func();
    }
    return "";
}

std::string ModuleLoader::GetModuleLicense(const std::string& name) {
    if (auto get_license_func =
            GetFunction<std::string (*)()>(name, "GetLicense");
        get_license_func) {
        return get_license_func();
    }
    return "";
}

std::string ModuleLoader::GetModulePath(const std::string& name) {
    std::shared_lock lock(m_SharedMutex);
    if (auto it = modules_.find(name); it != modules_.end()) {
        Dl_info dl_info;
        if (dladdr(it->second->handle, &dl_info) != 0) {
            return dl_info.dli_fname;
        }
    }
    return "";
}

json ModuleLoader::GetModuleConfig(const std::string& name) {
    if (auto get_config_func = GetFunction<json (*)()>(name, "GetConfig");
        get_config_func) {
        return get_config_func();
    }
    return {};
}

const std::vector<std::string> ModuleLoader::GetAllExistedModules() const {
    std::shared_lock lock(m_SharedMutex);
    std::vector<std::string> module_names;
    for (const auto& [name, _] : modules_) {
        module_names.push_back(name);
    }
    return module_names;
}

bool ModuleLoader::HasFunction(const std::string& name,
                               const std::string& function_name) {
    std::shared_lock lock(m_SharedMutex);
    if (auto it = modules_.find(name); it != modules_.end()) {
        return (LOAD_FUNCTION(it->second->handle, function_name.c_str()) !=
                nullptr);
    }
    LOG_F(ERROR, "Failed to find module {}", name);
    return false;
}

}  // namespace lithium
