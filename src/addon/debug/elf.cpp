#ifdef __linux__

#include "elf.hpp"

#include <elf.h>
#include <algorithm>
#include <fstream>

#include "atom/error/exception.hpp"

namespace lithium {

class ElfParser::Impl {
public:
    explicit Impl(std::string_view file) : filePath_(file) {}

    auto parse() -> bool {
        std::ifstream file(filePath_, std::ios::binary);
        if (!file) {
            return false;
        }

        file.seekg(0, std::ios::end);
        fileSize_ = file.tellg();
        file.seekg(0, std::ios::beg);

        fileContent_.resize(fileSize_);
        file.read(reinterpret_cast<char*>(fileContent_.data()), fileSize_);

        return parseElfHeader() && parseProgramHeaders() &&
               parseSectionHeaders() && parseSymbolTable();
    }

    [[nodiscard]] auto getElfHeader() const -> std::optional<ElfHeader> {
        return elfHeader_;
    }

    [[nodiscard]] auto getProgramHeaders() const
        -> std::span<const ProgramHeader> {
        return programHeaders_;
    }

    [[nodiscard]] auto getSectionHeaders() const
        -> std::span<const SectionHeader> {
        return sectionHeaders_;
    }

    [[nodiscard]] auto getSymbolTable() const -> std::span<const Symbol> {
        return symbolTable_;
    }

    [[nodiscard]] auto findSymbolByName(std::string_view name) const
        -> std::optional<Symbol> {
        auto it = std::ranges::find_if(
            symbolTable_,
            [name](const auto& symbol) { return symbol.name == name; });
        if (it != symbolTable_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    [[nodiscard]] auto findSymbolByAddress(uint64_t address) const
        -> std::optional<Symbol> {
        auto it = std::ranges::find_if(
            symbolTable_,
            [address](const auto& symbol) { return symbol.value == address; });
        if (it != symbolTable_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    [[nodiscard]] auto findSection(std::string_view name) const
        -> std::optional<SectionHeader> {
        auto it = std::ranges::find_if(
            sectionHeaders_,
            [name](const auto& section) { return section.name == name; });
        if (it != sectionHeaders_.end()) {
            return *it;
        }
        return std::nullopt;
    }

    [[nodiscard]] auto getSectionData(const SectionHeader& section) const
        -> std::vector<uint8_t> {
        if (section.offset + section.size > fileSize_) {
            THROW_OUT_OF_RANGE("Section data out of bounds");
        }
        return {fileContent_.begin() + section.offset,
                fileContent_.begin() + section.offset + section.size};
    }

private:
    std::string filePath_;
    std::vector<uint8_t> fileContent_;
    size_t fileSize_{};

    std::optional<ElfHeader> elfHeader_;
    std::vector<ProgramHeader> programHeaders_;
    std::vector<SectionHeader> sectionHeaders_;
    std::vector<Symbol> symbolTable_;

    auto parseElfHeader() -> bool {
        if (fileSize_ < sizeof(Elf64_Ehdr)) {
            return false;
        }

        const auto* ehdr =
            reinterpret_cast<const Elf64_Ehdr*>(fileContent_.data());
        elfHeader_ = ElfHeader{.type = ehdr->e_type,
                               .machine = ehdr->e_machine,
                               .version = ehdr->e_version,
                               .entry = ehdr->e_entry,
                               .phoff = ehdr->e_phoff,
                               .shoff = ehdr->e_shoff,
                               .flags = ehdr->e_flags,
                               .ehsize = ehdr->e_ehsize,
                               .phentsize = ehdr->e_phentsize,
                               .phnum = ehdr->e_phnum,
                               .shentsize = ehdr->e_shentsize,
                               .shnum = ehdr->e_shnum,
                               .shstrndx = ehdr->e_shstrndx};

        return true;
    }

    auto parseProgramHeaders() -> bool {
        if (!elfHeader_) {
            return false;
        }

        const auto* phdr = reinterpret_cast<const Elf64_Phdr*>(
            fileContent_.data() + elfHeader_->phoff);
        for (uint16_t i = 0; i < elfHeader_->phnum; ++i) {
            programHeaders_.push_back(ProgramHeader{.type = phdr[i].p_type,
                                                    .offset = phdr[i].p_offset,
                                                    .vaddr = phdr[i].p_vaddr,
                                                    .paddr = phdr[i].p_paddr,
                                                    .filesz = phdr[i].p_filesz,
                                                    .memsz = phdr[i].p_memsz,
                                                    .flags = phdr[i].p_flags,
                                                    .align = phdr[i].p_align});
        }

        return true;
    }

    auto parseSectionHeaders() -> bool {
        if (!elfHeader_) {
            return false;
        }

        const auto* shdr = reinterpret_cast<const Elf64_Shdr*>(
            fileContent_.data() + elfHeader_->shoff);
        const auto* strtab = reinterpret_cast<const char*>(
            fileContent_.data() + shdr[elfHeader_->shstrndx].sh_offset);

        for (uint16_t i = 0; i < elfHeader_->shnum; ++i) {
            sectionHeaders_.push_back(
                SectionHeader{.name = std::string(strtab + shdr[i].sh_name),
                              .type = shdr[i].sh_type,
                              .flags = shdr[i].sh_flags,
                              .addr = shdr[i].sh_addr,
                              .offset = shdr[i].sh_offset,
                              .size = shdr[i].sh_size,
                              .link = shdr[i].sh_link,
                              .info = shdr[i].sh_info,
                              .addralign = shdr[i].sh_addralign,
                              .entsize = shdr[i].sh_entsize});
        }

        return true;
    }

    auto parseSymbolTable() -> bool {
        auto symtabSection = std::ranges::find_if(
            sectionHeaders_,
            [](const auto& section) { return section.type == SHT_SYMTAB; });

        if (symtabSection == sectionHeaders_.end()) {
            return true;  // No symbol table, but not an error
        }

        const auto* symtab = reinterpret_cast<const Elf64_Sym*>(
            fileContent_.data() + symtabSection->offset);
        size_t numSymbols = symtabSection->size / sizeof(Elf64_Sym);

        const auto* strtab = reinterpret_cast<const char*>(
            fileContent_.data() + sectionHeaders_[symtabSection->link].offset);

        for (size_t i = 0; i < numSymbols; ++i) {
            symbolTable_.push_back(
                Symbol{.name = std::string(strtab + symtab[i].st_name),
                       .value = symtab[i].st_value,
                       .size = symtab[i].st_size,
                       .bind = ELF64_ST_BIND(symtab[i].st_info),
                       .type = ELF64_ST_TYPE(symtab[i].st_info),
                       .shndx = symtab[i].st_shndx});
        }

        return true;
    }
};

// ElfParser method implementations
ElfParser::ElfParser(std::string_view file)
    : pImpl_(std::make_unique<Impl>(file)) {}

ElfParser::~ElfParser() = default;

auto ElfParser::parse() -> bool { return pImpl_->parse(); }

auto ElfParser::getElfHeader() const -> std::optional<ElfHeader> {
    return pImpl_->getElfHeader();
}

auto ElfParser::getProgramHeaders() const -> std::span<const ProgramHeader> {
    return pImpl_->getProgramHeaders();
}

auto ElfParser::getSectionHeaders() const -> std::span<const SectionHeader> {
    return pImpl_->getSectionHeaders();
}

auto ElfParser::getSymbolTable() const -> std::span<const Symbol> {
    return pImpl_->getSymbolTable();
}

auto ElfParser::findSymbolByName(std::string_view name) const
    -> std::optional<Symbol> {
    return pImpl_->findSymbolByName(name);
}

auto ElfParser::findSymbolByAddress(uint64_t address) const
    -> std::optional<Symbol> {
    return pImpl_->findSymbolByAddress(address);
}

auto ElfParser::findSection(std::string_view name) const
    -> std::optional<SectionHeader> {
    return pImpl_->findSection(name);
}

auto ElfParser::getSectionData(const SectionHeader& section) const
    -> std::vector<uint8_t> {
    return pImpl_->getSectionData(section);
}
}  // namespace lithium

#endif  // __linux__