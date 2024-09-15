#ifndef LITHIUM_ADDON_PDB_HPP
#define LITHIUM_ADDON_PDB_HPP

#include <concepts>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "macro.hpp"

namespace lithium {

struct SymbolInfo {
    std::string name;
    uint64_t address;
    uint32_t size;
    uint32_t flags;
} ATOM_ALIGNAS(64);

struct TypeInfo {
    std::string name;
    uint32_t typeId;
    uint32_t size;
    uint32_t typeIndex;
} ATOM_ALIGNAS(64);

struct VariableInfo {
    std::string name;
    uint64_t address;
    uint32_t size;
    std::optional<TypeInfo> type;
} ATOM_ALIGNAS(128);

struct FunctionInfo {
    std::string name;
    uint64_t address;
    uint32_t size;
    uint32_t typeIndex;
    std::vector<VariableInfo> parameters;
    std::optional<TypeInfo> returnType;
} ATOM_ALIGNAS(128);

struct SourceLineInfo {
    std::string fileName;
    uint32_t lineNumber;
    uint64_t address;
} ATOM_ALIGNAS(64);

class PdbParser {
public:
    explicit PdbParser(std::string_view pdbFile);
    ~PdbParser();

    [[nodiscard]] bool initialize();
    [[nodiscard]] std::span<const SymbolInfo> getSymbols() const;
    [[nodiscard]] std::span<const TypeInfo> getTypes() const;
    [[nodiscard]] std::span<const VariableInfo> getGlobalVariables() const;
    [[nodiscard]] std::span<const FunctionInfo> getFunctions() const;

    template <std::invocable<const SymbolInfo&> Predicate>
    [[nodiscard]] std::optional<SymbolInfo> findSymbol(Predicate&& pred) const;

    [[nodiscard]] std::optional<SymbolInfo> findSymbolByName(
        std::string_view name) const;
    [[nodiscard]] std::optional<SymbolInfo> findSymbolByAddress(
        uint64_t address) const;

    [[nodiscard]] std::optional<TypeInfo> findTypeByName(
        std::string_view name) const;
    [[nodiscard]] std::optional<FunctionInfo> findFunctionByName(
        std::string_view name) const;
    [[nodiscard]] std::vector<SourceLineInfo> getSourceLinesForAddress(
        uint64_t address) const;

    [[nodiscard]] std::string demangleName(std::string_view name) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_PDB_HPP
