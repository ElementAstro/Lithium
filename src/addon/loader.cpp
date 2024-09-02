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

#include <cstdint>
#include <filesystem>
#include <memory>

#include "function/ffi.hpp"

#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#include <link.h>
#include <fstream>
#endif

#include "atom/io/io.hpp"

namespace lithium {

namespace internal {
auto replaceFilename(const std::string& path,
                     const std::string& newFilename) -> std::string {
    auto pos = path.find_last_of("/\\");
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
    try {
        if (!modules_.empty()) {
            if (!unloadAllModules()) {
                LOG_F(ERROR, "Failed to unload all modules");
            }
            modules_.clear();
        }
    } catch (const std::exception& ex) {
        LOG_F(ERROR, "Exception during module unloading: {}", ex.what());
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
    try {
        modInfo->mLibrary = std::make_shared<atom::meta::DynamicLibrary>(path);
    } catch (const std::exception& ex) {
        LOG_F(ERROR, "Failed to load module {}: {}", name, ex.what());
        return false;
    }

    modules_[name] = modInfo;

    auto moduleDumpPath = internal::replaceFilename(path, "module_dump.json");
    if (!atom::io::isFileExists(moduleDumpPath)) {
        std::ofstream out(moduleDumpPath);
        json dump;
        for (auto& func : modInfo->functions) {
            json j;
            j["name"] = func->name;
            j["address"] = reinterpret_cast<uintptr_t>(func->address);
            j["parameters"] = func->parameters;
            dump.push_back(j);
        }
        out << dump.dump(4);
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
    loadFunctionsUnix(handle, funcs);
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
        LOG_F(ERROR, "No export directory found in module.");
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
            LOG_F(INFO, "Loaded function: {}", func->name);
        } else {
            LOG_F(ERROR, "Failed to load function: {}", funcName);
        }
    }
}
#else
void ModuleLoader::loadFunctionsUnix(
    void*  /*handle*/, std::vector<std::unique_ptr<FunctionInfo>>& funcs) {
    // UNIX-specific implementation using dl_iterate_phdr
    dl_iterate_phdr(
        [](struct dl_phdr_info* info, size_t, void* data) {
            auto* funcs = static_cast<std::vector<std::unique_ptr<FunctionInfo>>*>(data);

            for (int i = 0; i < info->dlpi_phnum; ++i) {
                const ElfW(Phdr)* phdr = &info->dlpi_phdr[i];
                if (phdr->p_type != PT_DYNAMIC) {
                    continue;
                }

                auto* dyn = reinterpret_cast<ElfW(Dyn)*>(info->dlpi_addr + phdr->p_vaddr);
                ElfW(Sym)* symtab = nullptr;
                char* strtab = nullptr;
                size_t numSymbols = 0;
                size_t symEntrySize = 0;

                for (; dyn->d_tag != DT_NULL; ++dyn) {
                    switch (dyn->d_tag) {
                        case DT_SYMTAB:
                            symtab = reinterpret_cast<ElfW(Sym)*>(info->dlpi_addr + dyn->d_un.d_ptr);
                            break;
                        case DT_STRTAB:
                            strtab = reinterpret_cast<char*>(info->dlpi_addr + dyn->d_un.d_ptr);
                            break;
                        case DT_HASH:
                            if (dyn->d_un.d_ptr) {
                                auto* hash = reinterpret_cast<uint32_t*>(info->dlpi_addr + dyn->d_un.d_ptr);
                                numSymbols = hash[1];
                            }
                            break;
                        case DT_SYMENT:
                            symEntrySize = dyn->d_un.d_val;
                            break;
                    }
                }

                if (!symtab || !strtab || numSymbols == 0 || symEntrySize == 0) {
                    continue;
                }

                for (size_t j = 0; j < numSymbols; ++j) {
                    auto* sym = reinterpret_cast<ElfW(Sym)*>(reinterpret_cast<uint8_t*>(symtab) + j * symEntrySize);

                    if (ELF32_ST_TYPE(sym->st_info) == STT_FUNC && sym->st_name != 0) {
                        auto func = std::make_unique<FunctionInfo>();
                        func->name = &strtab[sym->st_name];
                        func->address = reinterpret_cast<void*>(info->dlpi_addr + sym->st_value);
                        funcs->push_back(std::move(func));
                    }
                }
            }
            return 0;
        },
        &funcs);
}
#endif

auto ModuleLoader::unloadModule(const std::string& name) -> bool {
    std::unique_lock lock(sharedMutex_);
    if (auto it = modules_.find(name); it != modules_.end()) {
        modules_.erase(it);
        LOG_F(INFO, "Unloaded module: {}", name);
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
    std::shared_lock lock(sharedMutex_);
    return modules_.contains(name);
}

auto ModuleLoader::enableModule(const std::string& name) -> bool {
    std::unique_lock lock(sharedMutex_);
    if (auto mod = getModule(name); mod && !mod->m_enabled.load()) {
        mod->m_enabled.store(true);
        LOG_F(INFO, "Module {} enabled", name);
        return true;
    }
    LOG_F(ERROR, "Failed to enable module {}", name);
    return false;
}

auto ModuleLoader::disableModule(const std::string& name) -> bool {
    std::unique_lock lock(sharedMutex_);
    if (auto mod = getModule(name); mod && mod->m_enabled.load()) {
        mod->m_enabled.store(false);
        LOG_F(INFO, "Module {} disabled", name);
        return true;
    }
    LOG_F(ERROR, "Failed to disable module {}", name);
    return false;
}

auto ModuleLoader::isModuleEnabled(const std::string& name) const -> bool {
    std::shared_lock lock(sharedMutex_);
    if (auto mod = getModule(name); mod) {
        return mod->m_enabled.load();
    }
    return false;
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
