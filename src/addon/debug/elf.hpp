#ifndef LITHIUM_ADDON_ELF_HPP
#define LITHIUM_ADDON_ELF_HPP

#ifdef __linux__

#include <concepts>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "macro.hpp"

namespace lithium {

/**
 * @brief Represents the ELF header structure.
 */
struct ElfHeader {
    uint16_t type;       ///< Object file type
    uint16_t machine;    ///< Architecture
    uint32_t version;    ///< Object file version
    uint64_t entry;      ///< Entry point virtual address
    uint64_t phoff;      ///< Program header table file offset
    uint64_t shoff;      ///< Section header table file offset
    uint32_t flags;      ///< Processor-specific flags
    uint16_t ehsize;     ///< ELF header size in bytes
    uint16_t phentsize;  ///< Program header table entry size
    uint16_t phnum;      ///< Program header table entry count
    uint16_t shentsize;  ///< Section header table entry size
    uint16_t shnum;      ///< Section header table entry count
    uint16_t shstrndx;   ///< Section header string table index
} ATOM_ALIGNAS(64);

/**
 * @brief Represents the program header structure.
 */
struct ProgramHeader {
    uint32_t type;    ///< Segment type
    uint64_t offset;  ///< Segment file offset
    uint64_t vaddr;   ///< Segment virtual address
    uint64_t paddr;   ///< Segment physical address
    uint64_t filesz;  ///< Segment size in file
    uint64_t memsz;   ///< Segment size in memory
    uint32_t flags;   ///< Segment flags
    uint64_t align;   ///< Segment alignment
} ATOM_ALIGNAS(64);

/**
 * @brief Represents the section header structure.
 */
struct SectionHeader {
    std::string name;    ///< Section name
    uint32_t type;       ///< Section type
    uint64_t flags;      ///< Section flags
    uint64_t addr;       ///< Section virtual address
    uint64_t offset;     ///< Section file offset
    uint64_t size;       ///< Section size in bytes
    uint32_t link;       ///< Link to another section
    uint32_t info;       ///< Additional section information
    uint64_t addralign;  ///< Section alignment
    uint64_t entsize;    ///< Entry size if section holds a table
} ATOM_ALIGNAS(128);

/**
 * @brief Represents a symbol in the ELF file.
 */
struct Symbol {
    std::string name;    ///< Symbol name
    uint64_t value;      ///< Symbol value
    uint64_t size;       ///< Symbol size
    unsigned char bind;  ///< Symbol binding
    unsigned char type;  ///< Symbol type
    uint16_t shndx;      ///< Section index
} ATOM_ALIGNAS(64);

/**
 * @brief Represents a dynamic entry in the ELF file.
 */
struct DynamicEntry {
    uint64_t tag;  ///< Dynamic entry tag
    union {
        uint64_t val;  ///< Integer value
        uint64_t ptr;  ///< Pointer value
    } d_un;            ///< Union for dynamic entry value
} ATOM_ALIGNAS(16);

/**
 * @brief Represents a relocation entry in the ELF file.
 */
struct RelocationEntry {
    uint64_t offset;  ///< Relocation offset
    uint64_t info;    ///< Relocation type and symbol index
    int64_t addend;   ///< Addend
} ATOM_ALIGNAS(32);

/**
 * @brief Parses and provides access to ELF file structures.
 */
class ElfParser {
public:
    /**
     * @brief Constructs an ElfParser with the given file.
     * @param file The path to the ELF file.
     */
    explicit ElfParser(std::string_view file);

    /**
     * @brief Destructor for ElfParser.
     */
    ~ElfParser();

    // Delete copy constructor and copy assignment operator
    ElfParser(const ElfParser&) = delete;
    ElfParser& operator=(const ElfParser&) = delete;

    // Default move constructor and move assignment operator
    ElfParser(ElfParser&&) = default;
    ElfParser& operator=(ElfParser&&) = default;

    /**
     * @brief Parses the ELF file.
     * @return True if parsing was successful, false otherwise.
     */
    [[nodiscard]] auto parse() -> bool;

    /**
     * @brief Gets the ELF header.
     * @return An optional containing the ELF header if available.
     */
    [[nodiscard]] auto getElfHeader() const -> std::optional<ElfHeader>;

    /**
     * @brief Gets the program headers.
     * @return A span of program headers.
     */
    [[nodiscard]] auto getProgramHeaders() const
        -> std::span<const ProgramHeader>;

    /**
     * @brief Gets the section headers.
     * @return A span of section headers.
     */
    [[nodiscard]] auto getSectionHeaders() const
        -> std::span<const SectionHeader>;

    /**
     * @brief Gets the symbol table.
     * @return A span of symbols.
     */
    [[nodiscard]] auto getSymbolTable() const -> std::span<const Symbol>;

    /**
     * @brief Gets the dynamic entries.
     * @return A span of dynamic entries.
     */
    [[nodiscard]] auto getDynamicEntries() const
        -> std::span<const DynamicEntry>;

    /**
     * @brief Gets the relocation entries.
     * @return A span of relocation entries.
     */
    [[nodiscard]] auto getRelocationEntries() const
        -> std::span<const RelocationEntry>;

    /**
     * @brief Finds a symbol using a predicate.
     * @tparam Predicate The type of the predicate.
     * @param pred The predicate to use for finding the symbol.
     * @return An optional containing the symbol if found.
     */
    template <std::invocable<const Symbol&> Predicate>
    [[nodiscard]] auto findSymbol(Predicate&& pred) const
        -> std::optional<Symbol>;

    /**
     * @brief Finds a symbol by name.
     * @param name The name of the symbol.
     * @return An optional containing the symbol if found.
     */
    [[nodiscard]] auto findSymbolByName(std::string_view name) const
        -> std::optional<Symbol>;

    /**
     * @brief Finds a symbol by address.
     * @param address The address of the symbol.
     * @return An optional containing the symbol if found.
     */
    [[nodiscard]] auto findSymbolByAddress(uint64_t address) const
        -> std::optional<Symbol>;

    /**
     * @brief Finds a section by name.
     * @param name The name of the section.
     * @return An optional containing the section header if found.
     */
    [[nodiscard]] auto findSection(std::string_view name) const
        -> std::optional<SectionHeader>;

    /**
     * @brief Gets the data of a section.
     * @param section The section header.
     * @return A vector containing the section data.
     */
    [[nodiscard]] auto getSectionData(const SectionHeader& section) const
        -> std::vector<uint8_t>;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;  ///< Pointer to the implementation.
};

template <std::invocable<const Symbol&> Predicate>
auto ElfParser::findSymbol(Predicate&& pred) const -> std::optional<Symbol> {
    auto symbols = getSymbolTable();
    if (auto iter =
            std::ranges::find_if(symbols, std::forward<Predicate>(pred));
        iter != symbols.end()) {
        return *iter;
    }
    return std::nullopt;
}

}  // namespace lithium

#endif  // __linux__

#endif  // LITHIUM_ADDON_ELF_HPP
