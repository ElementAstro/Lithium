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

#include <cstdint>
#include <filesystem>
#include <memory>

#include "function/ffi.hpp"
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

namespace internal {
auto replaceFilename(const std::string& path,
                     const std::string& newFilename) -> std::string {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return newFilename;
    }
    return path.substr(0, pos + 1) + newFilename;
}
}  // namespace internal

ModuleLoader::ModuleLoader(std::string dirName) {
    DLOG_F(INFO, "Module manager {} loaded successfully.", dirName);
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
    LOG_F(INFO, "Loading module: {} from {}", name, path);
    auto modInfo = std::make_shared<ModuleInfo>();
    modInfo->mLibrary = std::make_shared<atom::meta::DynamicLibrary>(path);
    modules_[name] = modInfo;

    // TODO: Fix me
    // modInfo->functions = loadModuleFunctions(name);
    auto moduleDumpPath = internal::replaceFilename(path, "module_dump.json");
    if (!atom::io::isFileExists(moduleDumpPath)) {
        std::ofstream out(moduleDumpPath);
        json dump;
        for (auto& func : modInfo->functions) {
            json j;
            j["name"] = func->name;
            j["address"] = (uintptr_t)func->address;
            j["parameters"] = func->parameters;
            dump.push_back(j);
        }
        dump.dump(4);
        LOG_F(INFO, "Dumped module functions to {}", moduleDumpPath);
    } else {
        LOG_F(WARNING, "Module dump file already exists, skipping");
    }
    DLOG_F(INFO, "Loaded module: {}", name);
    return true;
}

auto ModuleLoader::loadModuleFunctions(const std::string& name)
    -> std::vector<std::unique_ptr<FunctionInfo>> {
    std::vector<std::unique_ptr<FunctionInfo>> funcs;

    // std::shared_lock lock(sharedMutex_);
    auto it = modules_.find(name);
    if (it == modules_.end()) {
        LOG_F(ERROR, "Module not found: {}", name);
        return funcs;
    }

    void* handle = it->second->mLibrary->getHandle();
    if (handle == nullptr) {
        LOG_F(ERROR, "Failed to get handle for module: {}", name);
        return funcs;
    }

#ifdef _WIN32
    loadFunctionsWindows(handle, funcs);
#else
    loadFunctionsUnix(funcs);
#endif

    return funcs;
}

#ifdef _WIN32
void ModuleLoader::loadFunctionsWindows(
    void* handle, std::vector<std::unique_ptr<FunctionInfo>>& funcs) {
    HMODULE hModule = static_cast<HMODULE>(handle);
    ULONG size;
    auto* exports = (PIMAGE_EXPORT_DIRECTORY)ImageDirectoryEntryToDataEx(
        hModule, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &size, NULL);

    if (exports == nullptr) {
        logError("No export directory found in module.");
        return;
    }

    auto* names = (DWORD*)((BYTE*)hModule + exports->AddressOfNames);
    for (DWORD i = 0; i < exports->NumberOfNames; ++i) {
        char* funcName = (char*)hModule + names[i];
        void* funcAddress = (void*)GetProcAddress(hModule, funcName);
        if (funcAddress) {
            auto func = std::make_unique<FunctionInfo>();
            func->name = funcName;
            func->address = funcAddress;
            funcs.push_back(std::move(func));
            logInfo("Loaded function: " + func->name);
        } else {
            logError("Failed to load function: " + std::string(funcName));
        }
    }
}
#else
// TODO: Max: 现在加载函数信息还是有问题，主要集中在段错误的问题
void ModuleLoader::loadFunctionsUnix(
    std::vector<std::unique_ptr<FunctionInfo>>& funcs) {
    dl_iterate_phdr(
        [](struct dl_phdr_info* info, size_t, void* data) {
            auto* funcs =
                static_cast<std::vector<std::unique_ptr<FunctionInfo>>*>(data);

            for (int i = 0; i < info->dlpi_phnum; ++i) {
                const ElfW(Phdr)* phdr = &info->dlpi_phdr[i];
                if (phdr->p_type != PT_DYNAMIC) {
                    continue;
                }

                auto* dyn = reinterpret_cast<ElfW(Dyn)*>(info->dlpi_addr +
                                                         phdr->p_vaddr);
                ElfW(Sym)* symtab = nullptr;
                char* strtab = nullptr;
                size_t numSymbols = 0;
                size_t symEntrySize = 0;
                uint32_t* gnuHash = nullptr;

                // Parse the dynamic section to locate symbol and string tables
                for (; dyn->d_tag != DT_NULL; ++dyn) {
                    switch (dyn->d_tag) {
                        case DT_SYMTAB:
                            symtab = reinterpret_cast<ElfW(Sym)*>(
                                info->dlpi_addr + dyn->d_un.d_ptr);
                            break;
                        case DT_STRTAB:
                            strtab = reinterpret_cast<char*>(info->dlpi_addr +
                                                             dyn->d_un.d_ptr);
                            break;
                        case DT_HASH:
                            if (dyn->d_un.d_ptr) {
                                auto* hash = reinterpret_cast<uint32_t*>(
                                    info->dlpi_addr + dyn->d_un.d_ptr);
                                if (hash) {
                                    numSymbols =
                                        hash[1];  // The second entry is
                                                  // the number of symbols
                                }
                            }
                            break;
                        case DT_GNU_HASH:
                            if (dyn->d_un.d_ptr) {
                                gnuHash = reinterpret_cast<uint32_t*>(
                                    info->dlpi_addr + dyn->d_un.d_ptr);
                            }
                            break;
                        case DT_SYMENT:
                            symEntrySize = dyn->d_un.d_val;
                            break;
                    }
                }

                // Process GNU hash if available
                /*
                if (gnuHash) {
                    const uint32_t nbuckets = gnuHash[0];
                    const uint32_t* buckets = gnuHash + 4 + gnuHash[2];
                    const uint32_t* chains = buckets + nbuckets;

                    // Calculate the number of symbols based on the GNU hash
                    // table
                    for (size_t b = 0; b < nbuckets; ++b) {
                        if (buckets[b] != 0) {
                            uint32_t chainIndex = buckets[b];
                            while (!(chains[chainIndex] & 1)) {
                                ++chainIndex;
                            }
                            numSymbols =
                                std::max(numSymbols,
                                         static_cast<size_t>(chainIndex + 1));
                        }
                    }
                }
                */
                

                // If hash-based methods fail, estimate from symbol table size
                if (!numSymbols && symEntrySize && symtab) {
                    // Assume the table ends with DT_NULL or similar entry
                    for (ElfW(Sym)* sym = symtab;
                         sym->st_name != 0 ||
                         ELF32_ST_TYPE(sym->st_info) != STT_NOTYPE;
                         ++sym) {
                        ++numSymbols;
                    }
                }

                // Ensure valid symbol table and string table pointers
                if (!symtab || !strtab || numSymbols == 0 ||
                    symEntrySize == 0) {
                    continue;
                }

                // Iterate over the symbol table
                for (size_t j = 0; j < numSymbols; ++j) {
                    auto* sym = reinterpret_cast<ElfW(Sym)*>(
                        reinterpret_cast<uint8_t*>(symtab) + j * symEntrySize);

                    // Check for valid function symbols
                    if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC &&
                        sym->st_name != 0) {
                        auto func = std::make_unique<FunctionInfo>();
                        func->name = &strtab[sym->st_name];
                        func->address = reinterpret_cast<void*>(
                            info->dlpi_addr + sym->st_value);
                        funcs->push_back(std::move(func));
                    }
                }
            }
            return 0;  // Continue iteration
        },
        &funcs);
}
#endif

auto ModuleLoader::unloadModule(const std::string& name) -> bool {
    std::unique_lock lock(sharedMutex_);
    if (hasModule(name)) {
        modules_.erase(name);
        return true;
    }
    LOG_F(ERROR, "Module {} is not loaded", name);
    return false;
}

auto ModuleLoader::unloadAllModules() -> bool {
    std::unique_lock lock(sharedMutex_);
    for (auto& [name, module] : modules_) {
        if (!unloadModule(name)) {
            LOG_F(ERROR, "Failed to unload module {}", name);
        }
        LOG_F(INFO, "Unloaded module {}", name);
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

auto ModuleLoader::getHandle(const std::string& name) const
    -> std::shared_ptr<atom::meta::DynamicLibrary> {
    std::shared_lock lock(sharedMutex_);
    if (auto it = modules_.find(name); it != modules_.end()) {
        return it->second->mLibrary;
    }
    return nullptr;
}

auto ModuleLoader::hasModule(const std::string& name) const -> bool {
    // std::shared_lock lock(sharedMutex_);
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
    if (auto getVersionFunc = getFunction<std::string()>(name, "getVersion");
        getVersionFunc) {
        return getVersionFunc();
    }
    return "";
}

auto ModuleLoader::getModuleDescription(const std::string& name)
    -> std::string {
    if (auto getDescriptionFunc =
            getFunction<std::string()>(name, "getDescription");
        getDescriptionFunc) {
        return getDescriptionFunc();
    }
    return "";
}

auto ModuleLoader::getModuleAuthor(const std::string& name) -> std::string {
    if (auto getAuthorFunc = getFunction<std::string()>(name, "getAuthor");
        getAuthorFunc) {
        return getAuthorFunc();
    }
    return "";
}

auto ModuleLoader::getModuleLicense(const std::string& name) -> std::string {
    if (auto getLicenseFunc = getFunction<std::string()>(name, "getLicense");
        getLicenseFunc) {
        return getLicenseFunc();
    }
    return "";
}

auto ModuleLoader::getModuleConfig(const std::string& name) -> json {
    if (auto getConfigFunc = getFunction<json()>(name, "getConfig");
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
        return it->second->mLibrary->hasFunction(functionName);
    }
    LOG_F(ERROR, "Failed to find module {}", name);
    return false;
}

}  // namespace lithium
