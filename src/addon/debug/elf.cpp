#include "elf.hpp"

#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <unistd.h>
#include <cstring>

#include "atom/log/loguru.hpp"
#include "atom/macro.hpp"

namespace lithium {
// Define your structures
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
    uint32_t type{};
    uint64_t flags{};
    uint64_t addr{};
    uint64_t offset{};
    uint64_t size{};
    uint32_t link{};
    uint32_t info{};
    uint64_t addralign{};
    uint64_t entsize{};
} ATOM_ALIGNAS(128);

struct Symbol {
    std::string name;
    uint64_t value{};
    uint64_t size{};
    unsigned char bind{};
    unsigned char type{};
    uint16_t shndx{};
} ATOM_ALIGNAS(64);

struct DynamicEntry {
    uint64_t tag;
    union {
        uint64_t val;
        uint64_t ptr;
    } dUn;
} ATOM_ALIGNAS(16);

struct RelocationEntry {
    uint64_t offset;
    uint64_t info;
    int64_t addend;
} ATOM_ALIGNAS(32);

// Forward declaration of the implementation class
class ElfParser::Impl {
public:
    explicit Impl(const char* file);
    ~Impl();

    auto initialize() -> bool;
    void cleanup();
    auto parse() -> bool;
    [[nodiscard]] auto getElfHeader() const -> ElfHeader;
    [[nodiscard]] auto getProgramHeaders() const -> std::vector<ProgramHeader>;
    [[nodiscard]] auto getSectionHeaders() const -> std::vector<SectionHeader>;
    [[nodiscard]] auto getSymbolTable() const -> std::vector<Symbol>;
    [[nodiscard]] auto getDynamicEntries() const -> std::vector<DynamicEntry>;
    [[nodiscard]] auto getRelocationEntries() const -> std::vector<RelocationEntry>;

private:
    const char* filePath_;
    int fd_{};
    Elf* elf_{};
    GElf_Ehdr ehdr_;
};

// Implementation of the `Impl` class methods
ElfParser::Impl::Impl(const char* file) : filePath_(file) {}

ElfParser::Impl::~Impl() { cleanup(); }

auto ElfParser::Impl::initialize() -> bool {
    if (elf_version(EV_CURRENT) == EV_NONE) {
        LOG_F(ERROR, "ELF library initialization failed: {}", elf_errmsg(-1));
        return false;
    }

    fd_ = open(filePath_, O_RDONLY, 0);
    if (fd_ < 0) {
        LOG_F(ERROR, "Failed to open ELF file: {}", filePath_);
        return false;
    }

    elf_ = elf_begin(fd_, ELF_C_READ, nullptr);
    if (elf_ == nullptr) {
        LOG_F(ERROR, "elf_begin() failed: {}", elf_errmsg(-1));
        close(fd_);
        fd_ = -1;
        return false;
    }

    if (gelf_getehdr(elf_, &ehdr_) == nullptr) {
        LOG_F(ERROR, "gelf_getehdr() failed: {}", elf_errmsg(-1));
        elf_end(elf_);
        elf_ = nullptr;
        close(fd_);
        fd_ = -1;
        return false;
    }

    return true;
}

void ElfParser::Impl::cleanup() {
    if (elf_ != nullptr) {
        elf_end(elf_);
        elf_ = nullptr;
    }
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

auto ElfParser::Impl::parse() -> bool { return initialize(); }

auto ElfParser::Impl::getElfHeader() const -> ElfHeader {
    ElfHeader header{};
    header.type = ehdr_.e_type;
    header.machine = ehdr_.e_machine;
    header.version = ehdr_.e_version;
    header.entry = ehdr_.e_entry;
    header.phoff = ehdr_.e_phoff;
    header.shoff = ehdr_.e_shoff;
    header.flags = ehdr_.e_flags;
    header.ehsize = ehdr_.e_ehsize;
    header.phentsize = ehdr_.e_phentsize;
    header.phnum = ehdr_.e_phnum;
    header.shentsize = ehdr_.e_shentsize;
    header.shnum = ehdr_.e_shnum;
    header.shstrndx = ehdr_.e_shstrndx;
    return header;
}

auto ElfParser::Impl::getProgramHeaders() const -> std::vector<ProgramHeader> {
    std::vector<ProgramHeader> headers;
    for (size_t i = 0; i < ehdr_.e_phnum; ++i) {
        GElf_Phdr phdr;
        if (gelf_getphdr(elf_, i, &phdr) != &phdr) {
            LOG_F(ERROR, "gelf_getphdr() failed: {}", elf_errmsg(-1));
            continue;
        }
        ProgramHeader header;
        header.type = phdr.p_type;
        header.offset = phdr.p_offset;
        header.vaddr = phdr.p_vaddr;
        header.paddr = phdr.p_paddr;
        header.filesz = phdr.p_filesz;
        header.memsz = phdr.p_memsz;
        header.flags = phdr.p_flags;
        header.align = phdr.p_align;
        headers.push_back(header);
    }
    return headers;
}

auto ElfParser::Impl::getSectionHeaders() const -> std::vector<SectionHeader> {
    std::vector<SectionHeader> headers;
    for (size_t i = 0; i < ehdr_.e_shnum; ++i) {
        GElf_Shdr shdr;
        if (gelf_getshdr(elf_getscn(elf_, i), &shdr) != &shdr) {
            LOG_F(ERROR, "gelf_getshdr() failed: {}", elf_errmsg(-1));
            continue;
        }
        SectionHeader header;
        header.name = elf_strptr(elf_, ehdr_.e_shstrndx, shdr.sh_name);
        header.type = shdr.sh_type;
        header.flags = shdr.sh_flags;
        header.addr = shdr.sh_addr;
        header.offset = shdr.sh_offset;
        header.size = shdr.sh_size;
        header.link = shdr.sh_link;
        header.info = shdr.sh_info;
        header.addralign = shdr.sh_addralign;
        header.entsize = shdr.sh_entsize;
        headers.push_back(header);
    }
    return headers;
}

std::vector<Symbol> ElfParser::Impl::getSymbolTable() const {
    std::vector<Symbol> symbols;
    size_t shnum;
    elf_getshdrnum(elf_, &shnum);

    for (size_t i = 0; i < shnum; ++i) {
        Elf_Scn* scn = elf_getscn(elf_, i);
        GElf_Shdr shdr;
        gelf_getshdr(scn, &shdr);

        if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM) {
            Elf_Data* data = elf_getdata(scn, nullptr);
            size_t symbolCount = shdr.sh_size / shdr.sh_entsize;

            for (size_t j = 0; j < symbolCount; ++j) {
                GElf_Sym sym;
                gelf_getsym(data, j, &sym);
                Symbol symbol;
                symbol.name = elf_strptr(elf_, shdr.sh_link, sym.st_name);
                symbol.value = sym.st_value;
                symbol.size = sym.st_size;
                symbol.bind = GELF_ST_BIND(sym.st_info);
                symbol.type = GELF_ST_TYPE(sym.st_info);
                symbol.shndx = sym.st_shndx;
                symbols.push_back(symbol);
            }
        }
    }
    return symbols;
}

auto ElfParser::Impl::getDynamicEntries() const -> std::vector<DynamicEntry> {
    std::vector<DynamicEntry> entries;
    size_t shnum;
    elf_getshdrnum(elf_, &shnum);

    for (size_t i = 0; i < shnum; ++i) {
        Elf_Scn* scn = elf_getscn(elf_, i);
        GElf_Shdr shdr;
        gelf_getshdr(scn, &shdr);

        if (shdr.sh_type == SHT_DYNAMIC) {
            Elf_Data* data = elf_getdata(scn, nullptr);
            size_t entryCount = shdr.sh_size / shdr.sh_entsize;

            for (size_t j = 0; j < entryCount; ++j) {
                GElf_Dyn dyn;
                gelf_getdyn(data, j, &dyn);
                DynamicEntry entry;
                entry.tag = dyn.d_tag;
                entry.dUn.val = dyn.d_un.d_val;
                entries.push_back(entry);
            }
        }
    }
    return entries;
}

std::vector<RelocationEntry> ElfParser::Impl::getRelocationEntries() const {
    std::vector<RelocationEntry> entries;
    size_t shnum;
    elf_getshdrnum(elf_, &shnum);

    for (size_t i = 0; i < shnum; ++i) {
        Elf_Scn* scn = elf_getscn(elf_, i);
        GElf_Shdr shdr;
        gelf_getshdr(scn, &shdr);

        if (shdr.sh_type == SHT_RELA || shdr.sh_type == SHT_REL) {
            Elf_Data* data = elf_getdata(scn, nullptr);
            size_t entryCount = shdr.sh_size / shdr.sh_entsize;

            for (size_t j = 0; j < entryCount; ++j) {
                RelocationEntry entry;
                if (shdr.sh_type == SHT_RELA) {
                    GElf_Rela rela;
                    gelf_getrela(data, j, &rela);
                    entry.offset = rela.r_offset;
                    entry.info = rela.r_info;
                    entry.addend = rela.r_addend;
                } else {
                    GElf_Rel rel;
                    gelf_getrel(data, j, &rel);
                    entry.offset = rel.r_offset;
                    entry.info = rel.r_info;
                    entry.addend = 0;  // REL doesn't have addend
                }
                entries.push_back(entry);
            }
        }
    }
    return entries;
}

// Define the public `ElfParser` class methods
ElfParser::ElfParser(const char* file)
    : pImpl(std::make_unique<ElfParser::Impl>(file)) {}

bool ElfParser::parse() { return pImpl->parse(); }

ElfHeader ElfParser::getElfHeader() const { return pImpl->getElfHeader(); }

std::vector<ProgramHeader> ElfParser::getProgramHeaders() const {
    return pImpl->getProgramHeaders();
}

std::vector<SectionHeader> ElfParser::getSectionHeaders() const {
    return pImpl->getSectionHeaders();
}

std::vector<Symbol> ElfParser::getSymbolTable() const {
    return pImpl->getSymbolTable();
}

std::vector<DynamicEntry> ElfParser::getDynamicEntries() const {
    return pImpl->getDynamicEntries();
}

std::vector<RelocationEntry> ElfParser::getRelocationEntries() const {
    return pImpl->getRelocationEntries();
}

}  // namespace lithium
