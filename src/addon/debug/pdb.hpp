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
    PdbParser(const PdbParser&) = delete;
    PdbParser& operator=(const PdbParser&) = delete;
    PdbParser(PdbParser&&) = default;
    PdbParser& operator=(PdbParser&&) = default;

    [[nodiscard]] auto initialize() -> bool;
    [[nodiscard]] auto getSymbols() const -> std::span<const SymbolInfo>;
    [[nodiscard]] auto getTypes() const -> std::span<const TypeInfo>;
    [[nodiscard]] auto getGlobalVariables() const
        -> std::span<const VariableInfo>;
    [[nodiscard]] auto getFunctions() const -> std::span<const FunctionInfo>;

    template <std::invocable<const SymbolInfo&> Predicate>
    [[nodiscard]] auto findSymbol(Predicate&& pred) const
        -> std::optional<SymbolInfo>;

    [[nodiscard]] auto findSymbolByName(std::string_view name) const
        -> std::optional<SymbolInfo>;
    [[nodiscard]] auto findSymbolByAddress(uint64_t address) const
        -> std::optional<SymbolInfo>;

    [[nodiscard]] auto findTypeByName(std::string_view name) const
        -> std::optional<TypeInfo>;
    [[nodiscard]] auto findFunctionByName(std::string_view name) const
        -> std::optional<FunctionInfo>;
    [[nodiscard]] auto getSourceLinesForAddress(uint64_t address) const
        -> std::vector<SourceLineInfo>;

    [[nodiscard]] auto demangleName(std::string_view name) const -> std::string;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;  // Renamed to p_impl
};

template <std::invocable<const SymbolInfo&> Predicate>
auto PdbParser::findSymbol(Predicate&& pred) const
    -> std::optional<SymbolInfo> {
    auto symbols = getSymbols();
    if (auto iter =
            std::ranges::find_if(symbols, std::forward<Predicate>(pred));
        iter != symbols.end()) {
        return *iter;
    }
    return std::nullopt;
}

}  // namespace lithium

#endif  // LITHIUM_ADDON_PDB_HPP
