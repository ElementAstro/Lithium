#ifndef LITHIUM_ADDON_PDB_HPP
#define LITHIUM_ADDON_PDB_HPP

#ifdef _WIN32
#include <DbgHelp.h>
#include <Windows.h>
#include <memory>
#include <string>
#include <vector>

namespace lithium {
struct SymbolInfo {
    std::string name;
    DWORD64 address;
    DWORD size;
    DWORD flags;
};

struct TypeInfo {
    std::string name;
    DWORD typeId;
    DWORD size;
    DWORD typeIndex;
};

struct VariableInfo {
    std::string name;
    DWORD64 address;
    DWORD size;
};

struct FunctionInfo {
    std::string name;
    DWORD64 address;
    DWORD size;
    DWORD typeIndex;
};

class PdbParser {
public:
    explicit PdbParser(const std::string& pdbFile);
    ~PdbParser();

    bool initialize();
    std::vector<SymbolInfo> getSymbols() const;
    std::vector<TypeInfo> getTypes() const;
    std::vector<VariableInfo> getGlobalVariables() const;
    std::vector<FunctionInfo> getFunctions() const;
    SymbolInfo findSymbolByName(const std::string& name) const;
    SymbolInfo findSymbolByAddress(DWORD64 address) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;  // Pointer to implementation
};
}  // namespace lithium

#endif

#endif