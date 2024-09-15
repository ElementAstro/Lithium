// elf.hpp
#ifndef LITHIUM_ADDON_ELF_HPP
#define LITHIUM_ADDON_ELF_HPP

#include <concepts>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "macro.hpp"

namespace lithium {

struct ElfHeader {
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t phoff;
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;
    uint16_t phnum;
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
} ATOM_ALIGNAS(64);

struct ProgramHeader {
    uint32_t type;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesz;
    uint64_t memsz;
    uint32_t flags;
    uint64_t align;
} ATOM_ALIGNAS(64);

struct SectionHeader {
    std::string name;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addralign;
    uint64_t entsize;
} ATOM_ALIGNAS(128);

struct Symbol {
    std::string name;
    uint64_t value;
    uint64_t size;
    unsigned char bind;
    unsigned char type;
    uint16_t shndx;
} ATOM_ALIGNAS(64);

struct DynamicEntry {
    uint64_t tag;
    union {
        uint64_t val;
        uint64_t ptr;
    } d_un;
} ATOM_ALIGNAS(16);

struct RelocationEntry {
    uint64_t offset;
    uint64_t info;
    int64_t addend;
} ATOM_ALIGNAS(32);

class ElfParser {
public:
    explicit ElfParser(std::string_view file);
    ~ElfParser();

    [[nodiscard]] bool parse();
    [[nodiscard]] auto getElfHeader() const -> std::optional<ElfHeader>;
    [[nodiscard]] std::span<const ProgramHeader> getProgramHeaders() const;
    [[nodiscard]] auto getSectionHeaders() const
        -> std::span<const SectionHeader>;
    [[nodiscard]] auto getSymbolTable() const -> std::span<const Symbol>;
    [[nodiscard]] std::span<const DynamicEntry> getDynamicEntries() const;
    [[nodiscard]] std::span<const RelocationEntry> getRelocationEntries() const;

    template <std::invocable<const Symbol&> Predicate>
    [[nodiscard]] std::optional<Symbol> findSymbol(Predicate&& pred) const;

    [[nodiscard]] std::optional<Symbol> findSymbolByName(
        std::string_view name) const;
    [[nodiscard]] std::optional<Symbol> findSymbolByAddress(
        uint64_t address) const;

    [[nodiscard]] std::optional<SectionHeader> findSection(
        std::string_view name) const;
    [[nodiscard]] std::vector<uint8_t> getSectionData(
        const SectionHeader& section) const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

template <std::invocable<const Symbol&> Predicate>
std::optional<Symbol> ElfParser::findSymbol(Predicate&& pred) const {
    auto symbols = getSymbolTable();
    auto it = std::ranges::find_if(symbols, std::forward<Predicate>(pred));
    if (it != symbols.end()) {
        return *it;
    }
    return std::nullopt;
}
}  // namespace lithium

#endif  // LITHIUM_ADDON_ELF_HPP
