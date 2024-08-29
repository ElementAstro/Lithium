#ifdef _WIN32

#include "pdb.hpp"

#include "atom/log/loguru.hpp"

namespace lithium {
class PdbParser::Impl {
public:
    Impl(const std::string& pdbFile)
        : pdbFilePath(pdbFile),
          hProcess(GetCurrentProcess()),
          symInitialized(FALSE),
          baseAddress(0) {}
    ~Impl() { unloadPdb(); }

    bool initialize() { return loadPdb(); }

    std::vector<SymbolInfo> getSymbols() const;
    std::vector<TypeInfo> getTypes() const;
    std::vector<VariableInfo> getGlobalVariables() const;
    std::vector<FunctionInfo> getFunctions() const;
    SymbolInfo findSymbolByName(const std::string& name) const;
    SymbolInfo findSymbolByAddress(DWORD64 address) const;

private:
    std::string pdbFilePath;
    HANDLE hProcess;
    BOOL symInitialized;
    DWORD64 baseAddress;

    bool loadPdb();
    void unloadPdb();
};

bool PdbParser::Impl::loadPdb() {
    symInitialized = SymInitialize(hProcess, nullptr, FALSE);
    if (!symInitialized) {
        LOG_F(ERROR, "Failed to initialize DbgHelp: {}", GetLastError());
        return false;
    }

    baseAddress = SymLoadModuleEx(hProcess, nullptr, pdbFilePath.c_str(),
                                  nullptr, 0, 0, nullptr, 0);
    if (baseAddress == 0) {
        LOG_F(ERROR, "Failed to load PDB file: {}", GetLastError());
        return false;
    }

    return true;
}

void PdbParser::Impl::unloadPdb() {
    if (baseAddress != 0) {
        SymUnloadModule64(hProcess, baseAddress);
        baseAddress = 0;
    }

    if (symInitialized) {
        SymCleanup(hProcess);
        symInitialized = FALSE;
    }
}

std::vector<SymbolInfo> PdbParser::Impl::getSymbols() const {
    std::vector<SymbolInfo> symbols;

    SymEnumSymbols(
        hProcess, baseAddress, nullptr,
        [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) -> BOOL {
            auto* pSymbols = static_cast<std::vector<SymbolInfo>*>(UserContext);
            SymbolInfo symbol;
            symbol.name = std::string(pSymInfo->Name, pSymInfo->NameLen);
            symbol.address = pSymInfo->Address;
            symbol.size = pSymInfo->Size;
            symbol.flags = pSymInfo->Flags;
            pSymbols->emplace_back(symbol);
            return TRUE;
        },
        &symbols);

    return symbols;
}

std::vector<TypeInfo> PdbParser::Impl::getTypes() const {
    std::vector<TypeInfo> types;

    SymEnumTypes(
        hProcess, baseAddress,
        [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) -> BOOL {
            auto* pTypes = static_cast<std::vector<TypeInfo>*>(UserContext);
            TypeInfo type;
            type.name = std::string(pSymInfo->Name, pSymInfo->NameLen);
            type.typeId = pSymInfo->TypeIndex;
            type.size = pSymInfo->Size;
            type.typeIndex = pSymInfo->TypeIndex;
            pTypes->emplace_back(type);
            return TRUE;
        },
        &types);

    return types;
}

std::vector<VariableInfo> PdbParser::Impl::getGlobalVariables() const {
    std::vector<VariableInfo> variables;

    SymEnumSymbols(
        hProcess, baseAddress, nullptr,
        [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) -> BOOL {
            auto* pVariables =
                static_cast<std::vector<VariableInfo>*>(UserContext);
            if (pSymInfo->Flags & SYMFLAG_GLOBAL) {
                VariableInfo var;
                var.name = std::string(pSymInfo->Name, pSymInfo->NameLen);
                var.address = pSymInfo->Address;
                var.size = pSymInfo->Size;
                pVariables->emplace_back(var);
            }
            return TRUE;
        },
        &variables);

    return variables;
}

std::vector<FunctionInfo> PdbParser::Impl::getFunctions() const {
    std::vector<FunctionInfo> functions;

    SymEnumSymbols(
        hProcess, baseAddress, nullptr,
        [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext) -> BOOL {
            auto* pFunctions =
                static_cast<std::vector<FunctionInfo>*>(UserContext);
            if (pSymInfo->Tag == SymTagFunction) {
                FunctionInfo func;
                func.name = std::string(pSymInfo->Name, pSymInfo->NameLen);
                func.address = pSymInfo->Address;
                func.size = pSymInfo->Size;
                func.typeIndex = pSymInfo->TypeIndex;
                pFunctions->emplace_back(func);
            }
            return TRUE;
        },
        &functions);

    return functions;
}

SymbolInfo PdbParser::Impl::findSymbolByName(const std::string& name) const {
    SYMBOL_INFO_PACKAGE sip;
    ZeroMemory(&sip, sizeof(sip));
    sip.si.MaxNameLen = MAX_SYM_NAME;
    sip.si.SizeOfStruct = sizeof(SYMBOL_INFO);

    if (SymFromName(hProcess, name.c_str(), &sip.si)) {
        SymbolInfo symbol;
        symbol.name = sip.si.Name;
        symbol.address = sip.si.Address;
        symbol.size = sip.si.Size;
        symbol.flags = sip.si.Flags;
        return symbol;
    }

    return {"", 0, 0, 0};
}

SymbolInfo PdbParser::Impl::findSymbolByAddress(DWORD64 address) const {
    SYMBOL_INFO_PACKAGE sip;
    ZeroMemory(&sip, sizeof(sip));
    sip.si.MaxNameLen = MAX_SYM_NAME;
    sip.si.SizeOfStruct = sizeof(SYMBOL_INFO);

    if (SymFromAddr(hProcess, address, nullptr, &sip.si)) {
        SymbolInfo symbol;
        symbol.name = sip.si.Name;
        symbol.address = sip.si.Address;
        symbol.size = sip.si.Size;
        symbol.flags = sip.si.Flags;
        return symbol;
    }

    return {"", 0, 0, 0};
}
}  // namespace lithium

#endif