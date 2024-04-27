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

#include <cxxabi.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <typeinfo>

#include "atom/io/io.hpp"

#ifdef _WIN32
#include <Windows.h>
#include <psapi.h>
#else
#include <dlfcn.h>
#include <link.h>
#endif

namespace fs = std::filesystem;

#ifdef _WIN32
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

#define SET_CONFIG_VALUE(key) \
    config[dir.path().string()][#key] = module_config.value(#key, "");

namespace Lithium {
ModuleLoader::ModuleLoader(const std::string &dir_name = "modules") {
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
    const std::string &dir_name = "modules") {
    return std::make_shared<ModuleLoader>(dir_name);
}

bool ModuleLoader::LoadModule(const std::string &path,
                              const std::string &name) {
    // dead lock if use std::lock_guard
    // std::unique_lock lock(m_SharedMutex);
    try {
        // Max : The mod's name should be unique, so we check if it already
        // exists
        if (HasModule(name)) {
            LOG_F(ERROR, "Module {} already loaded", name);
            return false;
        }
        if (!Atom::IO::isFileExists(path)) {
            LOG_F(ERROR, "Module {} does not exist", name);
            return false;
        }
        // Create module info
        std::shared_ptr<ModuleInfo> mod_info = std::make_shared<ModuleInfo>();
        // Load the library file
        void *handle = LOAD_LIBRARY(path.c_str());
        if (!handle) {
            LOG_F(ERROR, "Failed to load library {}: {}", path, LOAD_ERROR());
            return false;
        }
        mod_info->handle = handle;
        mod_info->functions = loadModuleFunctions(name);
        // Load infomation from the library

        // Store the library handle in handles_ map with the module name as key
        // handles_[name] = handle;
        modules_[name] = mod_info;
        DLOG_F(INFO, "Loaded module : {}", name);
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to load library {}: {}", path, e.what());
        return false;
    }
}

std::vector<std::unique_ptr<FunctionInfo>> ModuleLoader::loadModuleFunctions(
    const std::string &name) {
    std::vector<std::unique_ptr<FunctionInfo>> funcs;
    auto it = modules_.find(name);
    if (it == modules_.end()) {
        LOG_F(ERROR, "Module {} not found", name);
        return funcs;
    }

#ifdef _WIN32
    HMODULE handle = reinterpret_cast<HMODULE>(it->second->handle);

    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);
    PIMAGE_NT_HEADERS ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(
        reinterpret_cast<char *>(handle) + dosHeader->e_lfanew);
    PIMAGE_EXPORT_DIRECTORY exportDir =
        reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(
            reinterpret_cast<char *>(handle) +
            ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
                .VirtualAddress);

    DWORD *nameTable = reinterpret_cast<DWORD *>(
        reinterpret_cast<char *>(handle) + exportDir->AddressOfNames);
    WORD *ordinalTable = reinterpret_cast<WORD *>(
        reinterpret_cast<char *>(handle) + exportDir->AddressOfNameOrdinals);
    DWORD *functionTable = reinterpret_cast<DWORD *>(
        reinterpret_cast<char *>(handle) + exportDir->AddressOfFunctions);

    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i) {
        std::unique_ptr<FunctionInfo> func = std::make_unique<FunctionInfo>();
        func->name = reinterpret_cast<const char *>(
            reinterpret_cast<char *>(handle) + nameTable[i]);
        func->address = reinterpret_cast<void *>(
            reinterpret_cast<char *>(handle) + functionTable[ordinalTable[i]]);
        // TODO funcs.push_back(std::construct_at(func.get()));
        DLOG_F(INFO, "Function: {}", func->name);

        // Trying to get function's parameters, but there are some issues with
        // it
        DWORD functionRva = functionTable[ordinalTable[i]];
        PIMAGE_FUNCTION_ENTRY functionEntry =
            reinterpret_cast<PIMAGE_FUNCTION_ENTRY>(
                reinterpret_cast<char *>(handle) + functionRva);
        ULONG_PTR dwFunctionLength =
            functionEntry->EndOfPrologue - functionEntry->StartingAddress;

        DWORD64 dwImageBase = reinterpret_cast<DWORD64>(handle);
        DWORD64 dwFunctionAddress =
            dwImageBase + functionEntry->StartingAddress;

        const int maxParameters = 16;
        BYTE parameters[maxParameters * sizeof(DWORD64)];
        DWORD cbParameterSize = sizeof(DWORD64) * maxParameters;
        DWORD cbBytesRead;
        if (ReadProcessMemory(GetCurrentProcess(),
                              reinterpret_cast<LPCVOID>(dwFunctionAddress),
                              parameters, cbParameterSize,
                              reinterpret_cast<SIZE_T *>(&cbBytesRead))) {
            BYTE *dwParameterPtr = parameters;
            DWORD64 dwParameter = *reinterpret_cast<DWORD64 *>(dwParameterPtr);
            for (int j = 0; j < maxParameters; ++j) {
                DWORD64 dwParameter = *dwParameterPtr;

                if (dwParameter == 0) {
                    break;
                }

                char parameterBuffer[256];
                sprintf_s(parameterBuffer, "%#010I64x", dwParameter);
                funcs[i].get()->parameters.push_back(parameterBuffer);

                ++dwParameterPtr;
            }
        }
    }
#else
    void *handle = it->second->handle;

    dl_iterate_phdr(
        [](struct dl_phdr_info *info, size_t size, void *data) {
            auto funcs =
                static_cast<std::vector<std::unique_ptr<FunctionInfo>> *>(data);
            if (info->dlpi_name && strcmp(info->dlpi_name, "") != 0) {
                DLOG_F(INFO, "Module: %s", info->dlpi_name);

                const ElfW(Addr) base_address = info->dlpi_addr;
                const ElfW(Phdr) *phdr = info->dlpi_phdr;
                const auto phnum = info->dlpi_phnum;

                for (int j = 0; j < phnum; ++j) {
                    if (phdr[j].p_type == PT_DYNAMIC) {
                        auto *dyn = reinterpret_cast<ElfW(Dyn) *>(
                            base_address + phdr[j].p_vaddr);
                        ElfW(Sym) *symtab = nullptr;
                        char *strtab = nullptr;

                        for (; dyn->d_tag != DT_NULL; ++dyn) {
                            if (dyn->d_tag == DT_SYMTAB) {
                                symtab = reinterpret_cast<ElfW(Sym) *>(
                                    base_address + dyn->d_un.d_ptr);
                            } else if (dyn->d_tag == DT_STRTAB) {
                                strtab = reinterpret_cast<char *>(
                                    base_address + dyn->d_un.d_ptr);
                            }
                        }

                        if (symtab && strtab) {
                            for (ElfW(Sym) *sym = symtab; sym->st_name != 0;
                                 ++sym) {
                                if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC) {
                                    const char *name = &strtab[sym->st_name];
                                    std::unique_ptr<FunctionInfo> func =
                                        std::make_unique<FunctionInfo>();
                                    func->name = name;
                                    func->address = reinterpret_cast<void *>(
                                        base_address + sym->st_value);
                                    funcs->push_back(std::move(func));
                                    DLOG_F(INFO, "Function: %s", name);
                                }
                            }
                        }
                    }
                }
            }
            return 0;
        },
        funcs);

#endif

    return funcs;
}

bool ModuleLoader::UnloadModule(const std::string &name) {
    // Check if the module is loaded and has a valid handle
    // Max: Check before loading to avaid dead lock
    if (!HasModule(name)) {
        LOG_F(ERROR, "Module {} is not loaded", name);
        return false;
    };
    // TODO: Fix this, maybe use a lock
    // std::unique_lock lock(m_SharedMutex);
    try {
        // Unload the library and remove its handle from handles_ map
        int result = UNLOAD_LIBRARY(GetModule(name)->handle);
        if (result != 0) {
            LOG_F(ERROR, "Failed to unload module {}", name);
            return false;
        }
        modules_.erase(name);
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "{}", e.what());
        return false;
    }
}

bool ModuleLoader::UnloadAllModules() {
    std::unique_lock lock(m_SharedMutex);
    for (auto entry : modules_) {
        int result = UNLOAD_LIBRARY(entry.second->handle);
        if (result != 0) {
            LOG_F(ERROR, "Failed to unload module {}", entry.first);
            return false;
        }
    }
    modules_.clear();
    return true;
}

bool ModuleLoader::CheckModuleExists(const std::string &name) const {
    std::shared_lock lock(m_SharedMutex);
    // Max : Directly check if the library exists seems to be a litle bit slow.
    // May we use filesystem instead?
    void *handle = LOAD_LIBRARY(name.c_str());
    if (handle == nullptr) {
        LOG_F(ERROR, "Module {} does not exist.", name);
        return false;
    }
    DLOG_F(INFO, "Module {} is existing.", name);
    UNLOAD_LIBRARY(handle);
    return true;
}

std::shared_ptr<ModuleInfo> ModuleLoader::GetModule(
    const std::string &name) const {
    std::shared_lock lock(m_SharedMutex);
    auto it = modules_.find(name);
    if (it == modules_.end()) {
        return nullptr;
    }
    return it->second;
}

void *ModuleLoader::GetHandle(const std::string &name) const {
    std::shared_lock lock(m_SharedMutex);
    auto it = modules_.find(name);
    if (it == modules_.end()) {
        return nullptr;
    }
    return it->second->handle;
}

bool ModuleLoader::HasModule(const std::string &name) const {
    std::shared_lock lock(m_SharedMutex);
    return modules_.count(name) > 0;
}

bool ModuleLoader::EnableModule(const std::string &name) {
    std::unique_lock lock(m_SharedMutex);
    // Check if the module is loaded
    if (!HasModule(name)) {
        LOG_F(ERROR, "Module {} is not loaded", name);
        return false;
    }
    std::shared_ptr<ModuleInfo> mod = GetModule(name);
    if (!mod->m_enabled.load()) {
        mod->m_enabled.store(true);
        std::string disabled_file = mod->m_path;
        std::string enabled_file =
            disabled_file.substr(0, disabled_file.size() - 8);
        if (CheckModuleExists(enabled_file)) {
            if (UnloadModule(enabled_file)) {
                std::rename(disabled_file.c_str(), enabled_file.c_str());
                return true;
            } else {
                return false;
            }
        } else {
            LOG_F(ERROR, "Enabled file not found for module {}", name);
            return false;
        }
    }
    return true;
}

bool ModuleLoader::DisableModule(const std::string &name) {
    std::unique_lock lock(m_SharedMutex);
    // Check if the module is loaded
    if (!HasModule(name)) {
        LOG_F(ERROR, "Module {} is not loaded", name);
        return false;
    }
    std::shared_ptr<ModuleInfo> mod = GetModule(name);
    if (mod->m_enabled.load()) {
        mod->m_enabled.store(false);
        std::string module_path = GetModulePath(name);
        if (module_path.empty()) {
            LOG_F(ERROR, "Module path not found for module {}", name);
            return false;
        }
        std::string disabled_file = module_path + ".disabled";
        if (std::rename(module_path.c_str(), disabled_file.c_str()) == 0) {
            modules_.erase(name);
            return true;
        } else {
            LOG_F(ERROR, "Failed to disable module {}", name);
            return false;
        }
    }
    return true;
}

bool ModuleLoader::IsModuleEnabled(const std::string &name) const {
    std::shared_lock lock(m_SharedMutex);
    if (!HasModule(name)) {
        LOG_F(ERROR, "Module {} is not loaded", name);
        return false;
    }
    if (GetModule(name)->m_enabled.load()) {
        return true;
    }
    return false;
}

std::string ModuleLoader::GetModuleVersion(const std::string &name) {
    std::shared_lock lock(m_SharedMutex);
    if (HasModule(name)) {
        return GetFunction<std::string (*)()>(name, "GetVersion")();
    }
    return "";
}

std::string ModuleLoader::GetModuleDescription(const std::string &name) {
    std::shared_lock lock(m_SharedMutex);
    if (HasModule(name)) {
        return GetFunction<std::string (*)()>(name, "GetDescription")();
    }
    return "";
}

std::string ModuleLoader::GetModuleAuthor(const std::string &name) {
    std::shared_lock lock(m_SharedMutex);
    if (HasModule(name)) {
        return GetFunction<std::string (*)()>(name, "GetAuthor")();
    }
    return "";
}

std::string ModuleLoader::GetModuleLicense(const std::string &name) {
    std::shared_lock lock(m_SharedMutex);
    if (HasModule(name)) {
        return GetFunction<std::string (*)()>(name, "GetLicense")();
    }
    return "";
}

std::string ModuleLoader::GetModulePath(const std::string &name) {
    std::shared_lock lock(m_SharedMutex);
    auto it = modules_.find(name);
    if (it != modules_.end()) {
        Dl_info dl_info;
        if (dladdr(it->second->handle, &dl_info) != 0) {
            return dl_info.dli_fname;
        }
    }
    return "";
}

json ModuleLoader::GetModuleConfig(const std::string &name) {
    if (HasModule(name)) {
        return GetFunction<json (*)()>(name, "GetConfig")();
    }
    return {};
}

const std::vector<std::string> ModuleLoader::GetAllExistedModules() const {
    std::shared_lock lock(m_SharedMutex);
    std::vector<std::string> modules_name;
    if (modules_.empty()) {
        return modules_name;
    }
    for (auto module_ : modules_) {
        modules_name.push_back(module_.first);
    }
    return modules_name;
}

bool ModuleLoader::HasFunction(const std::string &name,
                               const std::string &function_name) {
    std::shared_lock lock(m_SharedMutex);
    auto handle_it = modules_.find(name);
    if (handle_it == modules_.end()) {
        LOG_F(ERROR, "Failed to find module {}", name);
        return false;
    }
    return (LOAD_FUNCTION(handle_it->second->handle, function_name.c_str()) !=
            nullptr);
}
}  // namespace Lithium