#ifdef _WIN32

#include "pdb.hpp"
#include <DbgHelp.h>
#include <Windows.h>
#include <algorithm>
#include <ranges>
#include <stdexcept>

#pragma comment(lib, "dbghelp.lib")

namespace lithium {

class PdbParser::Impl {
public:
    explicit Impl(std::string_view pdbFile)
        : pdbFilePath(pdbFile), hProcess(GetCurrentProcess()), baseAddress(0) {}

    ~Impl() { cleanup(); }

    bool initialize() {
        if (!SymInitialize(hProcess, nullptr, FALSE)) {
            throw std::runtime_error("Failed to initialize DbgHelp");
        }

        baseAddress = SymLoadModuleEx(hProcess, nullptr, pdbFilePath.c_str(),
                                      nullptr, 0, 0, nullptr, 0);
        if (baseAddress == 0) {
            SymCleanup(hProcess);
            throw std::runtime_error("Failed to load PDB file");
        }

        return true;
    }

    std::span<const SymbolInfo> getSymbols() {
        if (symbols.empty()) {
            SymEnumSymbols(
                hProcess, baseAddress, nullptr,
                [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize,
                   PVOID UserContext) -> BOOL {
                    auto* pSymbols =
                        static_cast<std::vector<SymbolInfo>*>(UserContext);
                    pSymbols->push_back(SymbolInfo{
                        .name = std::string(pSymInfo->Name, pSymInfo->NameLen),
                        .address = pSymInfo->Address,
                        .size = pSymInfo->Size,
                        .flags = pSymInfo->Flags});
                    return TRUE;
                },
                &symbols);
        }
        return symbols;
    }

    std::span<const TypeInfo> getTypes() {
        if (types.empty()) {
            SymEnumTypes(
                hProcess, baseAddress,
                [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize,
                   PVOID UserContext) -> BOOL {
                    auto* pTypes =
                        static_cast<std::vector<TypeInfo>*>(UserContext);
                    pTypes->push_back(TypeInfo{
                        .name = std::string(pSymInfo->Name, pSymInfo->NameLen),
                        .typeId = pSymInfo->TypeIndex,
                        .size = pSymInfo->Size,
                        .typeIndex = pSymInfo->TypeIndex});
                    return TRUE;
                },
                &types);
        }
        return types;
    }

    std::span<const VariableInfo> getGlobalVariables() {
        if (globalVariables.empty()) {
            SymEnumSymbols(
                hProcess, baseAddress, nullptr,
                [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize,
                   PVOID UserContext) -> BOOL {
                    auto* pVariables =
                        static_cast<std::vector<VariableInfo>*>(UserContext);
                    if (pSymInfo->Flags & SYMFLAG_VALUEPRESENT) {
                        pVariables->push_back(VariableInfo{
                            .name =
                                std::string(pSymInfo->Name, pSymInfo->NameLen),
                            .address = pSymInfo->Address,
                            .size = pSymInfo->Size,
                            .type = getTypeInfo(pSymInfo->TypeIndex)});
                    }
                    return TRUE;
                },
                &globalVariables);
        }
        return globalVariables;
    }

    std::span<const FunctionInfo> getFunctions() {
        if (functions.empty()) {
            SymEnumSymbols(
                hProcess, baseAddress, nullptr,
                [](PSYMBOL_INFO pSymInfo, ULONG SymbolSize,
                   PVOID UserContext) -> BOOL {
                    auto* pFunctions =
                        static_cast<std::vector<FunctionInfo>*>(UserContext);
                    if (pSymInfo->Tag == SymTagFunction) {
                        pFunctions->push_back(FunctionInfo{
                            .name =
                                std::string(pSymInfo->Name, pSymInfo->NameLen),
                            .address = pSymInfo->Address,
                            .size = pSymInfo->Size,
                            .typeIndex = pSymInfo->TypeIndex,
                            .parameters =
                                getFunctionParameters(pSymInfo->TypeIndex),
                            .returnType = getTypeInfo(pSymInfo->TypeIndex)});
                    }
                    return TRUE;
                },
                &functions);
        }
        return functions;
    }

    std::optional<SymbolInfo> findSymbolByName(std::string_view name) const {
        SYMBOL_INFO_PACKAGE sip = {};
        sip.si.SizeOfStruct = sizeof(SYMBOL_INFO);
        sip.si.MaxNameLen = MAX_SYM_NAME;

        if (SymFromName(hProcess, name.data(), &sip.si)) {
            return SymbolInfo{.name = std::string(sip.si.Name, sip.si.NameLen),
                              .address = sip.si.Address,
                              .size = sip.si.Size,
                              .flags = sip.si.Flags};
        }
        return std::nullopt;
    }

    std::optional<SymbolInfo> findSymbolByAddress(uint64_t address) const {
        SYMBOL_INFO_PACKAGE sip = {};
        sip.si.SizeOfStruct = sizeof(SYMBOL_INFO);
        sip.si.MaxNameLen = MAX_SYM_NAME;
        DWORD64 displacement = 0;

        if (SymFromAddr(hProcess, address, &displacement, &sip.si)) {
            return SymbolInfo{.name = std::string(sip.si.Name, sip.si.NameLen),
                              .address = sip.si.Address,
                              .size = sip.si.Size,
                              .flags = sip.si.Flags};
        }
        return std::nullopt;
    }

    std::optional<TypeInfo> findTypeByName(std::string_view name) const {
        for (const auto& type : types) {
            if (type.name == name) {
                return type;
            }
        }
        return std::nullopt;
    }

    std::optional<FunctionInfo> findFunctionByName(
        std::string_view name) const {
        for (const auto& func : functions) {
            if (func.name == name) {
                return func;
            }
        }
        return std::nullopt;
    }

    std::vector<SourceLineInfo> getSourceLinesForAddress(
        uint64_t address) const {
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement = 0;
        std::vector<SourceLineInfo> sourceLines;

        if (SymGetLineFromAddr64(hProcess, address, &displacement, &line)) {
            sourceLines.push_back(SourceLineInfo{.fileName = line.FileName,
                                                 .lineNumber = line.LineNumber,
                                                 .address = line.Address});
        }

        return sourceLines;
    }

    std::string demangleName(std::string_view name) const {
        char undecorated[MAX_SYM_NAME] = {};
        if (UnDecorateSymbolName(name.data(), undecorated, MAX_SYM_NAME,
                                 UNDNAME_COMPLETE)) {
            return undecorated;
        }
        return std::string(name);
    }

private:
    std::string pdbFilePath;
    HANDLE hProcess;
    DWORD64 baseAddress;
    std::vector<SymbolInfo> symbols;
    std::vector<TypeInfo> types;
    std::vector<VariableInfo> globalVariables;
    std::vector<FunctionInfo> functions;

    void cleanup() {
        if (baseAddress != 0) {
            SymUnloadModule64(hProcess, baseAddress);
        }
        SymCleanup(hProcess);
    }

    static std::optional<TypeInfo> getTypeInfo(DWORD typeIndex) {
        // This is a placeholder. In a real implementation, you would use
        // SymGetTypeInfo to get detailed type information.
        return std::nullopt;
    }

    static std::vector<VariableInfo> getFunctionParameters(DWORD typeIndex) {
        // This is a placeholder. In a real implementation, you would use
        // SymGetTypeInfo to get function parameter information.
        return {};
    }
};

// PdbParser method implementations
PdbParser::PdbParser(std::string_view pdbFile)
    : pImpl(std::make_unique<Impl>(pdbFile)) {}

PdbParser::~PdbParser() = default;

bool PdbParser::initialize() { return pImpl->initialize(); }

std::span<const SymbolInfo> PdbParser::getSymbols() const {
    return pImpl->getSymbols();
}

std::span<const TypeInfo> PdbParser::getTypes() const {
    return pImpl->getTypes();
}

std::span<const VariableInfo> PdbParser::getGlobalVariables() const {
    return pImpl->getGlobalVariables();
}

std::span<const FunctionInfo> PdbParser::getFunctions() const {
    return pImpl->getFunctions();
}

std::optional<SymbolInfo> PdbParser::findSymbolByName(
    std::string_view name) const {
    return pImpl->findSymbolByName(name);
}

std::optional<SymbolInfo> PdbParser::findSymbolByAddress(
    uint64_t address) const {
    return pImpl->findSymbolByAddress(address);
}

std::optional<TypeInfo> PdbParser::findTypeByName(std::string_view name) const {
    return pImpl->findTypeByName(name);
}

std::optional<FunctionInfo> PdbParser::findFunctionByName(
    std::string_view name) const {
    return pImpl->findFunctionByName(name);
}

std::vector<SourceLineInfo> PdbParser::getSourceLinesForAddress(
    uint64_t address) const {
    return pImpl->getSourceLinesForAddress(address);
}

std::string PdbParser::demangleName(std::string_view name) const {
    return pImpl->demangleName(name);
}

template <std::invocable<const SymbolInfo&> Predicate>
std::optional<SymbolInfo> PdbParser::findSymbol(Predicate&& pred) const {
    auto symbols = getSymbols();
    auto it = std::ranges::find_if(symbols, std::forward<Predicate>(pred));
    if (it != symbols.end()) {
        return *it;
    }
    return std::nullopt;
}

}  // namespace lithium

#endif  // _WIN32