#ifdef __linux__

#include "elf.hpp"

#include <elf.h>
#include <algorithm>
#include <fstream>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace lithium {

class ElfParser::Impl {
public:
    explicit Impl(std::string_view file) : filePath_(file) {
        LOG_F(INFO, "ElfParser::Impl created for file: {}", file);
    }

    auto parse() -> bool {
        LOG_F(INFO, "Parsing ELF file: {}", filePath_);
        std::ifstream file(filePath_, std::ios::binary);
        if (!file) {
            LOG_F(ERROR, "Failed to open file: {}", filePath_);
            return false;
        }

        file.seekg(0, std::ios::end);
        fileSize_ = file.tellg();
        file.seekg(0, std::ios::beg);

        fileContent_.resize(fileSize_);
        file.read(reinterpret_cast<char*>(fileContent_.data()), fileSize_);

        bool result = parseElfHeader() && parseProgramHeaders() &&
                      parseSectionHeaders() && parseSymbolTable();
        if (result) {
            LOG_F(INFO, "Successfully parsed ELF file: {}", filePath_);
        } else {
            LOG_F(ERROR, "Failed to parse ELF file: {}", filePath_);
        }
        return result;
    }

    [[nodiscard]] auto getElfHeader() const -> std::optional<ElfHeader> {
        LOG_F(INFO, "Getting ELF header");
        return elfHeader_;
    }

    [[nodiscard]] auto getProgramHeaders() const
        -> std::span<const ProgramHeader> {
        LOG_F(INFO, "Getting program headers");
        return programHeaders_;
    }

    [[nodiscard]] auto getSectionHeaders() const
        -> std::span<const SectionHeader> {
        LOG_F(INFO, "Getting section headers");
        return sectionHeaders_;
    }

    [[nodiscard]] auto getSymbolTable() const -> std::span<const Symbol> {
        LOG_F(INFO, "Getting symbol table");
        return symbolTable_;
    }

    [[nodiscard]] auto findSymbolByName(std::string_view name) const
        -> std::optional<Symbol> {
        LOG_F(INFO, "Finding symbol by name: {}", name);
        auto it = std::ranges::find_if(
            symbolTable_,
            [name](const auto& symbol) { return symbol.name == name; });
        if (it != symbolTable_.end()) {
            LOG_F(INFO, "Found symbol: {}", name);
            return *it;
        }
        LOG_F(WARNING, "Symbol not found: {}", name);
        return std::nullopt;
    }

    [[nodiscard]] auto findSymbolByAddress(uint64_t address) const
        -> std::optional<Symbol> {
        LOG_F(INFO, "Finding symbol by address: {}", address);
        auto it = std::ranges::find_if(
            symbolTable_,
            [address](const auto& symbol) { return symbol.value == address; });
        if (it != symbolTable_.end()) {
            LOG_F(INFO, "Found symbol at address: {}", address);
            return *it;
        }
        LOG_F(WARNING, "Symbol not found at address: {}", address);
        return std::nullopt;
    }

    [[nodiscard]] auto findSection(std::string_view name) const
        -> std::optional<SectionHeader> {
        LOG_F(INFO, "Finding section by name: {}", name);
        auto it = std::ranges::find_if(
            sectionHeaders_,
            [name](const auto& section) { return section.name == name; });
        if (it != sectionHeaders_.end()) {
            LOG_F(INFO, "Found section: {}", name);
            return *it;
        }
        LOG_F(WARNING, "Section not found: {}", name);
        return std::nullopt;
    }

    [[nodiscard]] auto getSectionData(const SectionHeader& section) const
        -> std::vector<uint8_t> {
        LOG_F(INFO, "Getting data for section: {}", section.name);
        if (section.offset + section.size > fileSize_) {
            LOG_F(ERROR, "Section data out of bounds: {}", section.name);
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
        LOG_F(INFO, "Parsing ELF header");
        if (fileSize_ < sizeof(Elf64_Ehdr)) {
            LOG_F(ERROR, "File size too small for ELF header");
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

        LOG_F(INFO, "Parsed ELF header successfully");
        return true;
    }

    auto parseProgramHeaders() -> bool {
        LOG_F(INFO, "Parsing program headers");
        if (!elfHeader_) {
            LOG_F(ERROR, "ELF header not parsed");
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

        LOG_F(INFO, "Parsed program headers successfully");
        return true;
    }

    auto parseSectionHeaders() -> bool {
        LOG_F(INFO, "Parsing section headers");
        if (!elfHeader_) {
            LOG_F(ERROR, "ELF header not parsed");
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

        LOG_F(INFO, "Parsed section headers successfully");
        return true;
    }

    auto parseSymbolTable() -> bool {
        LOG_F(INFO, "Parsing symbol table");
        auto symtabSection = std::ranges::find_if(
            sectionHeaders_,
            [](const auto& section) { return section.type == SHT_SYMTAB; });

        if (symtabSection == sectionHeaders_.end()) {
            LOG_F(WARNING, "No symbol table found");
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

        LOG_F(INFO, "Parsed symbol table successfully");
        return true;
    }
};

// ElfParser method implementations
ElfParser::ElfParser(std::string_view file)
    : pImpl_(std::make_unique<Impl>(file)) {
    LOG_F(INFO, "ElfParser created for file: {}", file);
}

ElfParser::~ElfParser() = default;

auto ElfParser::parse() -> bool {
    LOG_F(INFO, "ElfParser::parse called");
    return pImpl_->parse();
}

auto ElfParser::getElfHeader() const -> std::optional<ElfHeader> {
    LOG_F(INFO, "ElfParser::getElfHeader called");
    return pImpl_->getElfHeader();
}

auto ElfParser::getProgramHeaders() const -> std::span<const ProgramHeader> {
    LOG_F(INFO, "ElfParser::getProgramHeaders called");
    return pImpl_->getProgramHeaders();
}

auto ElfParser::getSectionHeaders() const -> std::span<const SectionHeader> {
    LOG_F(INFO, "ElfParser::getSectionHeaders called");
    return pImpl_->getSectionHeaders();
}

auto ElfParser::getSymbolTable() const -> std::span<const Symbol> {
    LOG_F(INFO, "ElfParser::getSymbolTable called");
    return pImpl_->getSymbolTable();
}

auto ElfParser::findSymbolByName(std::string_view name) const
    -> std::optional<Symbol> {
    LOG_F(INFO, "ElfParser::findSymbolByName called with name: {}", name);
    return pImpl_->findSymbolByName(name);
}

auto ElfParser::findSymbolByAddress(uint64_t address) const
    -> std::optional<Symbol> {
    LOG_F(INFO, "ElfParser::findSymbolByAddress called with address: {}",
          address);
    return pImpl_->findSymbolByAddress(address);
}

auto ElfParser::findSection(std::string_view name) const
    -> std::optional<SectionHeader> {
    LOG_F(INFO, "ElfParser::findSection called with name: {}", name);
    return pImpl_->findSection(name);
}

auto ElfParser::getSectionData(const SectionHeader& section) const
    -> std::vector<uint8_t> {
    LOG_F(INFO, "ElfParser::getSectionData called for section: {}",
          section.name);
    return pImpl_->getSectionData(section);
}

}  // namespace lithium

#endif  // __linux__
