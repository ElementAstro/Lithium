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
#include <iostream>
#include <fstream>
#include <memory>
#include <typeinfo>
#include <cxxabi.h>
#include <regex>
#include <mutex>

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

namespace Lithium
{
    ModuleLoader::ModuleLoader(const std::string &dir_name = "modules")
    {
        DLOG_F(INFO, "C++ module manager loaded successfully.");
    }

    ModuleLoader::~ModuleLoader()
    {
        if (!modules_.empty())
        {
            if (!UnloadAllModules())
            {
                LOG_F(ERROR, "Failed to unload all modules");
            }
            modules_.clear();
        }
    }

    std::shared_ptr<ModuleLoader> ModuleLoader::createShared()
    {
        return std::make_shared<ModuleLoader>("modules");
    }
    
    std::shared_ptr<ModuleLoader> ModuleLoader::createShared(const std::string &dir_name = "modules")
    {
        return std::make_shared<ModuleLoader>(dir_name);
    }

    bool ModuleLoader::LoadModule(const std::string &path, const std::string &name)
    {
        std::unique_lock<std::shared_mutex> lock(m_SharedMutex);
        try
        {
            // Max : The mod's name should be unique, so we check if it already exists
            if (HasModule(name))
            {
                LOG_F(ERROR, "Module {} already loaded", name);
                return false;
            }
            if (!Atom::IO::isFileExists(path))
            {
                LOG_F(ERROR, "Module {} does not exist", name);
                return false;
            }
            // Create module info
            std::shared_ptr<ModuleInfo> mod_info = std::make_shared<ModuleInfo>();
            // Load the library file
            void *handle = LOAD_LIBRARY(path.c_str());
            if (!handle)
            {
                LOG_F(ERROR, "Failed to load library {}: {}", path, LOAD_ERROR());
                return false;
            }
            mod_info->handle = handle;
            // Load infomation from the library

            // Store the library handle in handles_ map with the module name as key
            // handles_[name] = handle;
            modules_[name] = mod_info;
            DLOG_F(INFO, "Loaded module : {}", name);
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to load library {}: {}", path, e.what());
            return false;
        }
    }

    std::vector<std::unique_ptr<FunctionInfo>> ModuleLoader::loadModuleFunctions(const std::string &name)
    {
        std::vector<std::unique_ptr<FunctionInfo>> funcs;
        auto it = modules_.find(name);
        if (it == modules_.end())
        {
            LOG_F(ERROR, "Module {} not found", name);
            return funcs;
        }

#ifdef _WIN32
        HMODULE handle = reinterpret_cast<HMODULE>(it->second->handle);

        PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);
        PIMAGE_NT_HEADERS ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<char *>(handle) + dosHeader->e_lfanew);
        PIMAGE_EXPORT_DIRECTORY exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<char *>(handle) + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

        DWORD *nameTable = reinterpret_cast<DWORD *>(reinterpret_cast<char *>(handle) + exportDir->AddressOfNames);
        WORD *ordinalTable = reinterpret_cast<WORD *>(reinterpret_cast<char *>(handle) + exportDir->AddressOfNameOrdinals);
        DWORD *functionTable = reinterpret_cast<DWORD *>(reinterpret_cast<char *>(handle) + exportDir->AddressOfFunctions);

        for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
        {
            std::unique_ptr<FunctionInfo> func = std::make_unique<FunctionInfo>();
            func->name = reinterpret_cast<const char *>(reinterpret_cast<char *>(handle) + nameTable[i]);
            func->address = reinterpret_cast<void *>(reinterpret_cast<char *>(handle) + functionTable[ordinalTable[i]]);
            // TODO funcs.push_back(std::construct_at(func.get()));
            DLOG_F(INFO, "Function: {}", func->name);

            // Trying to get function's parameters, but there are some issues with it
            DWORD functionRva = functionTable[ordinalTable[i]];
            PIMAGE_FUNCTION_ENTRY functionEntry = reinterpret_cast<PIMAGE_FUNCTION_ENTRY>(reinterpret_cast<char *>(handle) + functionRva);
            ULONG_PTR dwFunctionLength = functionEntry->EndOfPrologue - functionEntry->StartingAddress;

            DWORD64 dwImageBase = reinterpret_cast<DWORD64>(handle);
            DWORD64 dwFunctionAddress = dwImageBase + functionEntry->StartingAddress;

            const int maxParameters = 16;
            BYTE parameters[maxParameters * sizeof(DWORD64)];
            DWORD cbParameterSize = sizeof(DWORD64) * maxParameters;
            DWORD cbBytesRead;
            if (ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<LPCVOID>(dwFunctionAddress), parameters, cbParameterSize, reinterpret_cast<SIZE_T *>(&cbBytesRead)))
            {
                BYTE *dwParameterPtr = parameters;
                DWORD64 dwParameter = *reinterpret_cast<DWORD64 *>(dwParameterPtr);
                for (int j = 0; j < maxParameters; ++j)
                {
                    DWORD64 dwParameter = *dwParameterPtr;

                    if (dwParameter == 0)
                    {
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

        dl_iterate_phdr([](struct dl_phdr_info *info, size_t size, void *data)
                        {
        std::vector<FunctionInfo>* funcs = reinterpret_cast<std::vector<FunctionInfo>*>(data);

        if (info->dlpi_name && strcmp(info->dlpi_name, "") != 0) {
            DLOG_F(INFO, "Module: {}", info->dlpi_name);

            Dl_info dl_info;
            memset(&dl_info, 0, sizeof(Dl_info));
            dladdr(reinterpret_cast<const void*>(info->dlpi_addr), &dl_info);

            ElfW(Ehdr)* elfHeader = reinterpret_cast<ElfW(Ehdr)*>(info->dlpi_addr);
            int numOfSymbols = elfHeader->e_shnum;
            ElfW(Shdr)* symbolSection = nullptr;
            ElfW(Sym)* symbolTable = nullptr;
            char* strTable = nullptr;
            int strTableSize = 0;

/*
TODO: fix  this when we have a way to get the correct module base address from lldb
            for (int i = 0; i < numOfSymbols; ++i) {
                ElfW(Shdr)* sectionHeader = reinterpret_cast<ElfW(Shdr)*>(info->dlpi_addr + elfHeader->e_shoff + i * elfHeader->e_shentsize);
                if (sectionHeader->sh_type == SHT_DYNSYM) {
                    symbolSection = sectionHeader;
                    symbolTable = reinterpret_cast<ElfW(Sym)*>(info->dlpi_addr + symbolSection->sh_offset);
                    strTable = reinterpret_cast<char*>(info->dlpi_addr + reinterpret_cast<ElfW(Shdr)*>(info->dlpi_phdr + info->dlpi_phnum)->sh_offset);
                    strTableSize = reinterpret_cast<ElfW(Shdr)*>(info->dlpi_phdr + info->dlpi_phnum)->sh_size;
                    break;
                }

                // Get function parameters using libdw
                Dwfl* dwfl = dwfl_begin(nullptr);
                Dwfl_Module* module = dwfl_module_info(dwfl, info->dlpi_addr);
                GElf_Sym sym;
                GElf_Addr sym_address;
                Dwarf_Die* die = nullptr;

                if (gelf_getsym(symtab, i, &sym)) {
                    sym_address = sym.st_value;
                    die = dwarf_offdie(elf, sym.st_value);
                }

                if (die != nullptr) {
                    Dwarf_Die params;
                    Dwarf_Die param;
                    if (dwarf_child(die, &params) == DW_DLV_OK) {
                        while (dwarf_siblingof(die, &param) == DW_DLV_OK) {
                            char* paramName = nullptr;
                            dwarf_diename(param, &paramName);

                            if (paramName != nullptr) {
                                funcs[i].parameters.push_back(paramName);
                            }

                            dwarf_nextcu(die, &die, nullptr);
                        }
                    }
                }
            }
*/
            

            if (symbolTable && symbolSection) {
                int numOfSymbols = symbolSection->sh_size / sizeof(ElfW(Sym));
                for (int i = 0; i < numOfSymbols; ++i) {
                    ElfW(Sym)* symbol = &symbolTable[i];
                    char* symName = &strTable[symbol->st_name];

                    if (ELF64_ST_TYPE(symbol->st_info) == STT_FUNC && symName[0] != '\0') {
                        std::unique_ptr<FunctionInfo> func = std::make_unique<FunctionInfo>();
                        func->name = symName;
                        func->address = reinterpret_cast<void*>(info->dlpi_addr + symbol->st_value);
                        //funcs->push_back(std::move(func));
                        LOG_F(INFO, "Function: {}", symName);
                    }
                }
            }
        }

        return 0; },
                        &funcs);

        dlclose(handle);
#endif

        return funcs;
    }

    bool ModuleLoader::UnloadModule(const std::string &name)
    {
        std::unique_lock<std::shared_mutex> lock(m_SharedMutex);
        try
        {
            // Check if the module is loaded and has a valid handle
            if (!HasModule(name))
            {
                LOG_F(ERROR, "Module {} is not loaded", name);
                return false;
            };
            // Unload the library and remove its handle from handles_ map
            int result = UNLOAD_LIBRARY(GetModule(name)->handle);
            if (result != 0)
            {
                LOG_F(ERROR, "Failed to unload module {}", name);
                return false;
            }
            modules_.erase(name);
            return true;
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "{}", e.what());
            return false;
        }
    }

    bool ModuleLoader::UnloadAllModules()
    {
        std::unique_lock<std::shared_mutex> lock(m_SharedMutex);
        for (auto entry : modules_)
        {
            int result = UNLOAD_LIBRARY(entry.second->handle);
            if (result != 0)
            {
                LOG_F(ERROR, "Failed to unload module {}", entry.first);
                return false;
            }
        }
        modules_.clear();
        return true;
    }

    bool ModuleLoader::CheckModuleExists(const std::string &name) const
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        // Max : Directly check if the library exists seems to be a litle bit slow. May we use filesystem instead?
        void *handle = LOAD_LIBRARY(name.c_str());
        if (handle == nullptr)
        {
            LOG_F(ERROR, "Module {} does not exist.", name);
            return false;
        }
        DLOG_F(INFO, "Module {} is existing.", name);
        UNLOAD_LIBRARY(handle);
        return true;
    }

    std::shared_ptr<ModuleInfo> ModuleLoader::GetModule(const std::string &name) const
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        auto it = modules_.find(name);
        if (it == modules_.end())
        {
            return nullptr;
        }
        return it->second;
    }

    void *ModuleLoader::GetHandle(const std::string &name) const
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        auto it = modules_.find(name);
        if (it == modules_.end())
        {
            return nullptr;
        }
        return it->second->handle;
    }

    bool ModuleLoader::HasModule(const std::string &name) const
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        return modules_.count(name) > 0;
    }

    bool ModuleLoader::EnableModule(const std::string &name)
    {
        std::unique_lock<std::shared_mutex> lock(m_SharedMutex);
        // Check if the module is loaded
        if (!HasModule(name))
        {
            LOG_F(ERROR, "Module {} is not loaded", name);
            return false;
        }
        std::shared_ptr<ModuleInfo> mod = GetModule(name);
        if (!mod->m_enabled.load())
        {
            mod->m_enabled.store(true);
            std::string disabled_file = mod->m_path;
            std::string enabled_file = disabled_file.substr(0, disabled_file.size() - 8);
            if (CheckModuleExists(enabled_file))
            {
                if (UnloadModule(enabled_file))
                {
                    std::rename(disabled_file.c_str(), enabled_file.c_str());
                    return true;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                LOG_F(ERROR, "Enabled file not found for module {}", name);
                return false;
            }
        }
        return true;
    }

    bool ModuleLoader::DisableModule(const std::string &name)
    {
        std::unique_lock<std::shared_mutex> lock(m_SharedMutex);
        // Check if the module is loaded
        if (!HasModule(name))
        {
            LOG_F(ERROR, "Module {} is not loaded", name);
            return false;
        }
        std::shared_ptr<ModuleInfo> mod = GetModule(name);
        if (mod->m_enabled.load())
        {
            mod->m_enabled.store(false);
            std::string module_path = GetModulePath(name);
            if (module_path.empty())
            {
                LOG_F(ERROR, "Module path not found for module {}", name);
                return false;
            }
            std::string disabled_file = module_path + ".disabled";
            if (std::rename(module_path.c_str(), disabled_file.c_str()) == 0)
            {
                modules_.erase(name);
                return true;
            }
            else
            {
                LOG_F(ERROR, "Failed to disable module {}", name);
                return false;
            }
        }
        return true;
    }

    bool ModuleLoader::IsModuleEnabled(const std::string &name) const
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        if (!HasModule(name))
        {
            LOG_F(ERROR, "Module {} is not loaded", name);
            return false;
        }
        if (GetModule(name)->m_enabled.load())
        {
            return true;
        }
        return false;
    }

    std::string ModuleLoader::GetModuleVersion(const std::string &name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        if (HasModule(name))
        {
            return GetFunction<std::string (*)()>(name, "GetVersion")();
        }
        return "";
    }

    std::string ModuleLoader::GetModuleDescription(const std::string &name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        if (HasModule(name))
        {
            return GetFunction<std::string (*)()>(name, "GetDescription")();
        }
        return "";
    }

    std::string ModuleLoader::GetModuleAuthor(const std::string &name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        if (HasModule(name))
        {
            return GetFunction<std::string (*)()>(name, "GetAuthor")();
        }
        return "";
    }

    std::string ModuleLoader::GetModuleLicense(const std::string &name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        if (HasModule(name))
        {
            return GetFunction<std::string (*)()>(name, "GetLicense")();
        }
        return "";
    }

    std::string ModuleLoader::GetModulePath(const std::string &name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        auto it = modules_.find(name);
        if (it != modules_.end())
        {
            Dl_info dl_info;
            if (dladdr(it->second->handle, &dl_info) != 0)
            {
                return dl_info.dli_fname;
            }
        }
        return "";
    }

    json ModuleLoader::GetModuleConfig(const std::string &name)
    {
        if (HasModule(name))
        {
            return GetFunction<json (*)()>(name, "GetConfig")();
        }
        return {};
    }

    const std::vector<std::string> ModuleLoader::GetAllExistedModules() const
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        std::vector<std::string> modules_name;
        if (modules_.empty())
        {
            return modules_name;
        }
        for (auto module_ : modules_)
        {
            modules_name.push_back(module_.first);
        }
        return modules_name;
    }

    bool ModuleLoader::HasFunction(const std::string &name, const std::string &function_name)
    {
        std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
        auto handle_it = modules_.find(name);
        if (handle_it == modules_.end())
        {
            LOG_F(ERROR, "Failed to find module {}", name);
            return false;
        }
        return (LOAD_FUNCTION(handle_it->second->handle, function_name.c_str()) != nullptr);
    }
}