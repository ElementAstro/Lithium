#include "utils.hpp"

#include <vector>

#ifdef _WIN32
#include <Windows.h>
#include <psapi.h>
#else
#include <dlfcn.h>
#include <link.h>
#endif

struct FunctionInfo
{
    std::string name;
    void *address;
    std::vector<std::string> parameters;
};

struct LibraryInfo
{
    std::string name;
    std::string path;
    std::vector<FunctionInfo> functions;
};

std::vector<LibraryInfo> EnumerateLibraries()
{
    std::vector<LibraryInfo> libraries;

#ifdef _WIN32
    // Windows implementation
    HMODULE hMods[1024];
    DWORD cbNeeded;

    if (EnumProcessModules(GetCurrentProcess(), hMods, sizeof(hMods), &cbNeeded))
    {
        for (DWORD i = 0; i < (cbNeeded / sizeof(HMODULE)); ++i)
        {
            char szModName[MAX_PATH];

            if (GetModuleFileNameA(hMods[i], szModName, sizeof(szModName) / sizeof(char)))
            {
                LibraryInfo libInfo;
                libInfo.name = szModName;
                libInfo.path = szModName;
                libraries.push_back(libInfo);
            }
        }
    }
#else
    // Linux implementation
    dl_iterate_phdr([](struct dl_phdr_info *info, size_t size, void *data)
                    {
        std::vector<LibraryInfo>* libs = reinterpret_cast<std::vector<LibraryInfo>*>(data);

        if (info->dlpi_name && strcmp(info->dlpi_name, "") != 0) {
            LibraryInfo libInfo;
            libInfo.name = info->dlpi_name;
            libs->push_back(libInfo);
        }

        return 0; },
                    &libraries);
#endif

    return libraries;
}

std::vector<FunctionInfo> EnumExportedFunctions(const char *moduleName)
{
    std::vector<FunctionInfo> funcs;

#ifdef _WIN32
    HMODULE handle = LoadLibraryA(moduleName);
    if (!handle)
    {
        std::cerr << "Failed to load module: " << GetLastError() << std::endl;
        return funcs;
    }

    PIMAGE_DOS_HEADER dosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(handle);
    PIMAGE_NT_HEADERS ntHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<char *>(handle) + dosHeader->e_lfanew);
    PIMAGE_EXPORT_DIRECTORY exportDir = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(reinterpret_cast<char *>(handle) + ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    DWORD *nameTable = reinterpret_cast<DWORD *>(reinterpret_cast<char *>(handle) + exportDir->AddressOfNames);
    WORD *ordinalTable = reinterpret_cast<WORD *>(reinterpret_cast<char *>(handle) + exportDir->AddressOfNameOrdinals);
    DWORD *functionTable = reinterpret_cast<DWORD *>(reinterpret_cast<char *>(handle) + exportDir->AddressOfFunctions);

    for (DWORD i = 0; i < exportDir->NumberOfNames; ++i)
    {
        FunctionInfo func;
        func.name = reinterpret_cast<const char *>(reinterpret_cast<char *>(handle) + nameTable[i]);
        func.address = reinterpret_cast<void *>(reinterpret_cast<char *>(handle) + functionTable[ordinalTable[i]]);
        funcs.push_back(func);

        std::cout << "Function: " << func.name << " (" << func.address << ")" << std::endl;

        // Get function parameters
        DWORD functionRva = functionTable[ordinalTable[i]];
        PIMAGE_FUNCTION_ENTRY functionEntry = reinterpret_cast<PIMAGE_FUNCTION_ENTRY>(reinterpret_cast<char *>(handle) + functionRva);
        ULONG_PTR dwFunctionLength = functionEntry->EndOfPrologue - functionEntry->StartingAddress;

        DWORD64 dwImageBase = reinterpret_cast<DWORD64>(handle);
        DWORD64 dwFunctionAddress = dwImageBase + functionEntry->StartingAddress;

        const int maxParameters = 16;                     // Maximum number of parameters to retrieve
        BYTE parameters[maxParameters * sizeof(DWORD64)]; // Use BYTE array instead of DWORD array
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
                funcs[i].parameters.push_back(parameterBuffer);

                ++dwParameterPtr;
            }
        }
    }

    FreeLibrary(handle);
#else
    dlerror();
    void *handle = dlopen(moduleName, RTLD_NOW);
    if (!handle)
    {
        std::cerr << "Failed to load module: " << dlerror() << std::endl;
        return funcs;
    }

    dl_iterate_phdr([](struct dl_phdr_info *info, size_t size, void *data)
                    {
        std::vector<FunctionInfo>* funcs = reinterpret_cast<std::vector<FunctionInfo>*>(data);

        if (info->dlpi_name && strcmp(info->dlpi_name, "") != 0) {
            std::cout << "Module: " << info->dlpi_name << std::endl;

            Dl_info dl_info;
            memset(&dl_info, 0, sizeof(Dl_info));
            dladdr(info->dlpi_addr, &dl_info);

            ElfW(Ehdr)* elfHeader = reinterpret_cast<ElfW(Ehdr)*>(info->dlpi_addr);
            int numOfSymbols = elfHeader->e_shnum;
            ElfW(Shdr)* symbolSection = nullptr;
            ElfW(Sym)* symbolTable = nullptr;
            char* strTable = nullptr;
            int strTableSize = 0;

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

            if (symbolTable && symbolSection) {
                int numOfSymbols = symbolSection->sh_size / sizeof(ElfW(Sym));
                for (int i = 0; i < numOfSymbols; ++i) {
                    ElfW(Sym)* symbol = &symbolTable[i];
                    char* symName = &strTable[symbol->st_name];

                    if (ELF64_ST_TYPE(symbol->st_info) == STT_FUNC && symName[0] != '\0') {
                        FunctionInfo func;
                        func.name = symName;
                        func.address = reinterpret_cast<void*>(info->dlpi_addr + symbol->st_value);
                        funcs->push_back(func);

                        std::cout << "Function: " << symName << " (" << func.address << ")" << std::endl;
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

LibraryInfo GetLibraryInfo(const std::string &moduleName, const std::string &modulePath)
{
    LibraryInfo libInfo;
    libInfo.name = moduleName;

#ifdef _WIN32
    // Windows implementation
    std::string fullPath = modulePath + moduleName;
    HMODULE handle = LoadLibraryA(fullPath.c_str());
    if (!handle)
    {
        std::cerr << "Failed to load module: " << GetLastError() << std::endl;
        return libInfo;
    }

    char szModName[MAX_PATH];
    if (GetModuleFileNameA(handle, szModName, sizeof(szModName) / sizeof(char)))
    {
        libInfo.path = szModName;
    }

    FreeLibrary(handle);
#else
    // Linux implementation
    dlerror();
    void *handle = dlopen(modulePath.c_str(), RTLD_NOW);
    if (!handle)
    {
        std::cerr << "Failed to load module: " << dlerror() << std::endl;
        return libInfo;
    }

    Dl_info dl_info;
    memset(&dl_info, 0, sizeof(Dl_info));
    dladdr(reinterpret_cast<void *>(handle), &dl_info);

    libInfo.path = dl_info.dli_fname;

    dlclose(handle);
#endif

    libInfo.functions = EnumExportedFunctions(moduleName.c_str());

    return libInfo;
}

int main()
{
    std::vector<LibraryInfo> libraries = EnumerateLibraries();

    for (const LibraryInfo &libInfo : libraries)
    {
        std::cout << "Library: " << libInfo.name << std::endl;
        std::cout << "Path: " << libInfo.path << std::endl;

        std::vector<FunctionInfo> funcs = libInfo.functions;
        std::cout << "Total functions: " << funcs.size() << std::endl;

        for (const FunctionInfo &func : funcs)
        {
            std::cout << "Function: " << func.name << " (" << func.address << ")" << std::endl;
            std::cout << "Parameters: ";
            for (const std::string &param : func.parameters)
            {
                std::cout << param << " ";
            }
            std::cout << std::endl;
        }

        std::cout << std::endl;
    }

    auto it = GetLibraryInfo("libadd_numbers.so", "./");
    for (const FunctionInfo &func : it.functions)
    {
        std::cout << "Function: " << func.name << " (" << func.address << ")" << std::endl;
        std::cout << "Parameters: ";
        for (const std::string &param : func.parameters)
        {
            std::cout << param << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
