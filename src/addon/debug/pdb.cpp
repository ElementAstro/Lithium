#ifdef _WIN32

#include "pdb.hpp"

// clang-format off
#include <windows.h>
#include <dbgHelp.h>
#ifdef _MSC_VER
#include <cvconst.h>
#endif
// clang-format on

#include <stdexcept>

#ifdef _MSC_VER
#pragma comment(lib, "dbghelp.lib")
#endif

namespace lithium {

class PdbParser::Impl {
public:
    explicit Impl(std::string_view pdbFile)
        : pdbFilePath(pdbFile), hProcess(GetCurrentProcess()), baseAddress(0) {}

    ~Impl() { cleanup(); }

    Impl(Impl&&) = default;
    Impl& operator=(Impl&&) = default;

    auto initialize() -> bool {
        if (static_cast<bool>(SymInitialize(hProcess, nullptr, FALSE)) ==
            false) {
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

    auto getSymbols() -> std::span<const SymbolInfo> {
        if (symbols.empty()) {
            SymEnumSymbols(
                hProcess, baseAddress, nullptr,
                [](PSYMBOL_INFO pSymInfo, [[maybe_unused]] ULONG SymbolSize,
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

    auto getTypes() -> std::span<const TypeInfo> {
        if (types.empty()) {
            SymEnumTypes(
                hProcess, baseAddress,
                [](PSYMBOL_INFO pSymInfo, [[maybe_unused]] ULONG SymbolSize,
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

    auto getGlobalVariables() -> std::span<const VariableInfo> {
        if (globalVariables.empty()) {
            SymEnumSymbols(
                hProcess, baseAddress, nullptr,
                [](PSYMBOL_INFO pSymInfo, [[maybe_unused]] ULONG SymbolSize,
                   PVOID UserContext) -> BOOL {
                    auto* pVariables =
                        static_cast<std::vector<VariableInfo>*>(UserContext);
                    if (static_cast<bool>(pSymInfo->Flags &
                                          SYMFLAG_VALUEPRESENT)) {
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

    auto getFunctions() -> std::span<const FunctionInfo> {
        if (functions.empty()) {
            SymEnumSymbols(
                hProcess, baseAddress, nullptr,
                [](PSYMBOL_INFO pSymInfo, [[maybe_unused]] ULONG SymbolSize,
                   PVOID UserContext) -> BOOL {
                    auto* pFunctions =
                        static_cast<std::vector<FunctionInfo>*>(UserContext);
                // TODO: Implement this for MinGW
#ifdef _MSC_VER
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
#endif
                    return TRUE;
                },
                &functions);
        }
        return functions;
    }

    [[nodiscard]] auto findSymbolByName(std::string_view name) const
        -> std::optional<SymbolInfo> {
        SYMBOL_INFO_PACKAGE sip = {};
        sip.si.SizeOfStruct = sizeof(SYMBOL_INFO);
        sip.si.MaxNameLen = MAX_SYM_NAME;

        if (static_cast<bool>(SymFromName(hProcess, name.data(), &sip.si))) {
            return SymbolInfo{.name = std::string(sip.si.Name, sip.si.NameLen),
                              .address = sip.si.Address,
                              .size = sip.si.Size,
                              .flags = sip.si.Flags};
        }
        return std::nullopt;
    }

    [[nodiscard]] auto findSymbolByAddress(uint64_t address) const
        -> std::optional<SymbolInfo> {
        SYMBOL_INFO_PACKAGE sip = {};
        sip.si.SizeOfStruct = sizeof(SYMBOL_INFO);
        sip.si.MaxNameLen = MAX_SYM_NAME;
        DWORD64 displacement = 0;

        if (static_cast<bool>(
                SymFromAddr(hProcess, address, &displacement, &sip.si))) {
            return SymbolInfo{.name = std::string(sip.si.Name, sip.si.NameLen),
                              .address = sip.si.Address,
                              .size = sip.si.Size,
                              .flags = sip.si.Flags};
        }
        return std::nullopt;
    }

    [[nodiscard]] auto findTypeByName(std::string_view name) const
        -> std::optional<TypeInfo> {
        for (const auto& type : types) {
            if (type.name == name) {
                return type;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] auto findFunctionByName(std::string_view name) const
        -> std::optional<FunctionInfo> {
        for (const auto& func : functions) {
            if (func.name == name) {
                return func;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] auto getSourceLinesForAddress(uint64_t address) const
        -> std::vector<SourceLineInfo> {
        IMAGEHLP_LINE64 line = {};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD displacement = 0;
        std::vector<SourceLineInfo> sourceLines;

        if (static_cast<bool>(SymGetLineFromAddr64(hProcess, address,
                                                   &displacement, &line))) {
            sourceLines.push_back(SourceLineInfo{.fileName = line.FileName,
                                                 .lineNumber = line.LineNumber,
                                                 .address = line.Address});
        }

        return sourceLines;
    }

    [[nodiscard]] static auto demangleName(std::string_view name)
        -> std::string {
        std::array<char, MAX_SYM_NAME> undecorated = {};
        if (static_cast<bool>(
                UnDecorateSymbolName(name.data(), undecorated.data(),
                                     MAX_SYM_NAME, UNDNAME_COMPLETE))) {
            return std::string(undecorated.data());
        }
        return std::string(name);
    }

private:
    std::string pdbFilePath;
    HANDLE hProcess;
    DWORD64 baseAddress = 0;
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

    static auto getTypeInfo([[maybe_unused]] DWORD typeIndex)
        -> std::optional<TypeInfo> {
        // This is a placeholder. In a real implementation, you would use
        // SymGetTypeInfo to get detailed type information.
        return std::nullopt;
    }

    static auto getFunctionParameters([[maybe_unused]] DWORD typeIndex)
        -> std::vector<VariableInfo> {
        // This is a placeholder. In a real implementation, you would use
        // SymGetTypeInfo to get function parameter information.
        return {};
    }
};

// PdbParser method implementations
PdbParser::PdbParser(std::string_view pdbFile)
    : pImpl_(std::make_unique<Impl>(pdbFile)) {}

PdbParser::~PdbParser() = default;

auto PdbParser::initialize() -> bool { return pImpl_->initialize(); }

auto PdbParser::getSymbols() const -> std::span<const SymbolInfo> {
    return pImpl_->getSymbols();
}

auto PdbParser::getTypes() const -> std::span<const TypeInfo> {
    return pImpl_->getTypes();
}

auto PdbParser::getGlobalVariables() const -> std::span<const VariableInfo> {
    return pImpl_->getGlobalVariables();
}

auto PdbParser::getFunctions() const -> std::span<const FunctionInfo> {
    return pImpl_->getFunctions();
}

[[nodiscard]] auto PdbParser::findSymbolByName(std::string_view name) const
    -> std::optional<SymbolInfo> {
    return pImpl_->findSymbolByName(name);
}

[[nodiscard]] auto PdbParser::findSymbolByAddress(uint64_t address) const
    -> std::optional<SymbolInfo> {
    return pImpl_->findSymbolByAddress(address);
}

[[nodiscard]] auto PdbParser::findTypeByName(std::string_view name) const
    -> std::optional<TypeInfo> {
    return pImpl_->findTypeByName(name);
}

[[nodiscard]] auto PdbParser::findFunctionByName(std::string_view name) const
    -> std::optional<FunctionInfo> {
    return pImpl_->findFunctionByName(name);
}

[[nodiscard]] auto PdbParser::getSourceLinesForAddress(uint64_t address) const
    -> std::vector<SourceLineInfo> {
    return pImpl_->getSourceLinesForAddress(address);
}

[[nodiscard]] auto PdbParser::demangleName(std::string_view name) const
    -> std::string {
    return pImpl_->demangleName(name);
}
}  // namespace lithium

#endif  // _WIN32