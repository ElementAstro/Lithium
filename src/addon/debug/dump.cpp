#include "dump.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "atom/log/loguru.hpp"

constexpr size_t ELF_IDENT_SIZE = 16;
constexpr size_t NUM_REGISTERS = 27;
constexpr size_t NUM_GENERAL_REGISTERS = 24;
constexpr uint32_t SHT_NOTE = 7;
constexpr uint32_t SHT_PROGBITS = 1;
constexpr uint32_t PT_LOAD = 1;
constexpr size_t ALIGN_64 = 64;
constexpr size_t ALIGN_16 = 16;
constexpr size_t ALIGN_128 = 128;

namespace lithium::addon {
class CoreDumpAnalyzer::Impl {
public:
    auto readFile(const std::string& filename) -> bool {
        LOG_F(INFO, "Reading file: {}", filename);
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            LOG_F(ERROR, "Unable to open file: {}", filename);
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        data_.resize(fileSize);
        file.read(reinterpret_cast<char*>(data_.data()),
                  static_cast<std::streamsize>(fileSize));

        if (!file) {
            LOG_F(ERROR, "Error reading file: {}", filename);
            return false;
        }

        if (fileSize < sizeof(ElfHeader)) {
            LOG_F(ERROR, "File too small to be a valid ELF format: {}",
                  filename);
            return false;
        }

        std::memcpy(&header_, data_.data(), sizeof(ElfHeader));
        LOG_F(INFO, "Successfully read file: {}", filename);
        return true;
    }

    [[nodiscard]] auto getElfHeaderInfo() const -> std::string {
        LOG_F(INFO, "Getting ELF header info");
        std::ostringstream oss;
        oss << "ELF Header:\n";
        oss << "  Type: " << header_.e_type << "\n";
        oss << "  Machine: " << header_.e_machine << "\n";
        oss << "  Version: " << header_.e_version << "\n";
        oss << "  Entry point address: 0x" << std::hex << header_.e_entry
            << "\n";
        oss << "  Start of program headers: " << header_.e_phoff
            << " (bytes into file)\n";
        oss << "  Start of section headers: " << header_.e_shoff
            << " (bytes into file)\n";
        oss << "  Flags: 0x" << std::hex << header_.e_flags << "\n";
        oss << "  Size of this header: " << header_.e_ehsize << " (bytes)\n";
        oss << "  Size of program headers: " << header_.e_phentsize
            << " (bytes)\n";
        oss << "  Number of program headers: " << header_.e_phnum << "\n";
        oss << "  Size of section headers: " << header_.e_shentsize
            << " (bytes)\n";
        oss << "  Number of section headers: " << header_.e_shnum << "\n";
        oss << "  Section header string table index: " << header_.e_shstrndx
            << "\n";
        return oss.str();
    }

    [[nodiscard]] auto getProgramHeadersInfo() const -> std::string {
        LOG_F(INFO, "Getting program headers info");
        std::ostringstream oss;
        oss << "Program Headers:\n";
        for (const auto& programHeader : programHeaders_) {
            oss << "  Type: " << programHeader.p_type << "\n";
            oss << "  Offset: 0x" << std::hex << programHeader.p_offset << "\n";
            oss << "  Virtual address: 0x" << std::hex << programHeader.p_vaddr
                << "\n";
            oss << "  Physical address: 0x" << std::hex << programHeader.p_paddr
                << "\n";
            oss << "  File size: " << programHeader.p_filesz << "\n";
            oss << "  Memory size: " << programHeader.p_memsz << "\n";
            oss << "  Flags: 0x" << std::hex << programHeader.p_flags << "\n";
            oss << "  Align: " << programHeader.p_align << "\n";
        }
        return oss.str();
    }

    [[nodiscard]] auto getSectionHeadersInfo() const -> std::string {
        LOG_F(INFO, "Getting section headers info");
        std::ostringstream oss;
        oss << "Section Headers:\n";
        for (const auto& sectionHeader : sectionHeaders_) {
            oss << "  Name: " << sectionHeader.sh_name << "\n";
            oss << "  Type: " << sectionHeader.sh_type << "\n";
            oss << "  Flags: 0x" << std::hex << sectionHeader.sh_flags << "\n";
            oss << "  Address: 0x" << std::hex << sectionHeader.sh_addr << "\n";
            oss << "  Offset: 0x" << std::hex << sectionHeader.sh_offset
                << "\n";
            oss << "  Size: " << sectionHeader.sh_size << "\n";
            oss << "  Link: " << sectionHeader.sh_link << "\n";
            oss << "  Info: " << sectionHeader.sh_info << "\n";
            oss << "  Address align: " << sectionHeader.sh_addralign << "\n";
            oss << "  Entry size: " << sectionHeader.sh_entsize << "\n";
        }
        return oss.str();
    }

    [[nodiscard]] auto getNoteSectionInfo() const -> std::string {
        LOG_F(INFO, "Getting note section info");
        std::ostringstream oss;
        oss << "Note Sections:\n";
        for (const auto& section : sectionHeaders_) {
            if (section.sh_type == SHT_NOTE) {
                size_t offset = section.sh_offset;
                while (offset < section.sh_offset + section.sh_size) {
                    NoteSection note{};
                    std::memcpy(&note, data_.data() + offset,
                                sizeof(NoteSection));
                    offset += sizeof(NoteSection);

                    std::string name(
                        reinterpret_cast<const char*>(data_.data() + offset),
                        note.n_namesz - 1);
                    offset += note.n_namesz;

                    oss << "  Note: " << name << ", Type: 0x" << std::hex
                        << note.n_type << ", Size: " << note.n_descsz
                        << " bytes\n";

                    if (name == "CORE" && note.n_type == 1) {
                        oss << getThreadInfo(offset);
                    } else if (name == "CORE" && note.n_type == 4) {
                        oss << getFileInfo(offset);
                    }

                    offset += note.n_descsz;
                }
            }
        }
        return oss.str();
    }

    [[nodiscard]] auto getThreadInfo(size_t offset) const -> std::string {
        LOG_F(INFO, "Getting thread info at offset: {}", offset);
        std::ostringstream oss;
        ThreadInfo thread{};
        std::memcpy(&thread.tid, data_.data() + offset, sizeof(uint64_t));
        std::memcpy(thread.registers.data(),
                    data_.data() + offset + sizeof(uint64_t),
                    sizeof(uint64_t) * NUM_REGISTERS);

        oss << "  Thread ID: " << thread.tid << "\n";
        oss << "  Registers:\n";
        const std::array<const char*, NUM_GENERAL_REGISTERS> REG_NAMES = {
            "RAX", "RBX",    "RCX", "RDX", "RSI", "RDI", "RBP", "RSP",
            "R8",  "R9",     "R10", "R11", "R12", "R13", "R14", "R15",
            "RIP", "EFLAGS", "CS",  "SS",  "DS",  "ES",  "FS",  "GS"};
        for (size_t i = 0; i < NUM_GENERAL_REGISTERS; ++i) {
            oss << "    " << REG_NAMES[i] << ": 0x" << std::hex
                << thread.registers[i] << "\n";
        }
        return oss.str();
    }

    [[nodiscard]] auto getFileInfo(size_t offset) const -> std::string {
        LOG_F(INFO, "Getting file info at offset: {}", offset);
        std::ostringstream oss;
        uint64_t count =
            *reinterpret_cast<const uint64_t*>(data_.data() + offset);
        offset += sizeof(uint64_t);

        oss << "  Open File Descriptors:\n";
        for (uint64_t i = 0; i < count; ++i) {
            int fileDescriptor =
                *reinterpret_cast<const int*>(data_.data() + offset);
            offset += sizeof(int);
            uint64_t nameSize =
                *reinterpret_cast<const uint64_t*>(data_.data() + offset);
            offset += sizeof(uint64_t);
            std::string filename(
                reinterpret_cast<const char*>(data_.data() + offset), nameSize);
            offset += nameSize;

            oss << "    File Descriptor " << fileDescriptor << ": " << filename
                << "\n";
        }
        return oss.str();
    }

    [[nodiscard]] auto getMemoryMapsInfo() const -> std::string {
        LOG_F(INFO, "Getting memory maps info");
        std::ostringstream oss;
        oss << "Memory Maps:\n";
        for (const auto& programHeader : programHeaders_) {
            if (programHeader.p_type == PT_LOAD) {
                oss << "  Mapping: 0x" << std::hex << programHeader.p_vaddr
                    << " - 0x" << std::hex
                    << (programHeader.p_vaddr + programHeader.p_memsz)
                    << " (Size: 0x" << std::hex << programHeader.p_memsz
                    << " bytes)\n";
            }
        }
        return oss.str();
    }

    [[nodiscard]] auto getSignalHandlersInfo() const -> std::string {
        LOG_F(INFO, "Getting signal handlers info");
        std::ostringstream oss;
        oss << "Signal Handlers:\n";
        for (const auto& section : sectionHeaders_) {
            if (section.sh_type == SHT_NOTE &&
                section.sh_size >= sizeof(uint64_t) * 2) {
                uint64_t signalNum = *reinterpret_cast<const uint64_t*>(
                    data_.data() + section.sh_offset);
                uint64_t handlerAddr = *reinterpret_cast<const uint64_t*>(
                    data_.data() + section.sh_offset + sizeof(uint64_t));
                oss << "  Signal " << signalNum << ": Handler Address 0x"
                    << std::hex << handlerAddr << "\n";
            }
        }
        return oss.str();
    }

    [[nodiscard]] auto getHeapUsageInfo() const -> std::string {
        LOG_F(INFO, "Getting heap usage info");
        std::ostringstream oss;
        oss << "Heap Usage:\n";
        auto heapSection =
            std::find_if(sectionHeaders_.begin(), sectionHeaders_.end(),
                         [](const SectionHeader& sectionHeader) {
                             return sectionHeader.sh_type == SHT_PROGBITS &&
                                    ((sectionHeader.sh_flags & 0x1U) != 0U);
                         });

        if (heapSection != sectionHeaders_.end()) {
            oss << "  Heap Region: 0x" << std::hex << heapSection->sh_addr
                << " - 0x" << std::hex
                << (heapSection->sh_addr + heapSection->sh_size) << " (Size: 0x"
                << std::hex << heapSection->sh_size << " bytes)\n";
        } else {
            oss << "  No explicit heap region found\n";
        }
        return oss.str();
    }

    void analyze() {
        LOG_F(INFO, "Analyzing core dump");
        if (data_.empty()) {
            LOG_F(WARNING, "No data to analyze");
            return;
        }

        if (std::memcmp(header_.e_ident.data(),
                        "\x7F"
                        "ELF",
                        4) != 0) {
            LOG_F(ERROR, "Not a valid ELF file");
            return;
        }

        LOG_F(INFO, "File size: {} bytes", data_.size());
        LOG_F(INFO, "ELF header size: {} bytes", sizeof(ElfHeader));
        LOG_F(INFO, "Analysis complete");
    }

private:
    std::vector<uint8_t> data_;
    struct alignas(ALIGN_64) ElfHeader {
        std::array<uint8_t, ELF_IDENT_SIZE> e_ident;
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_version;
        uint64_t e_entry;
        uint64_t e_phoff;
        uint64_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
    };
    ElfHeader header_{};

    struct alignas(ALIGN_64) ProgramHeader {
        uint32_t p_type;
        uint32_t p_flags;
        uint64_t p_offset;
        uint64_t p_vaddr;
        uint64_t p_paddr;
        uint64_t p_filesz;
        uint64_t p_memsz;
        uint64_t p_align;
    };

    struct alignas(ALIGN_64) SectionHeader {
        uint32_t sh_name;
        uint32_t sh_type;
        uint64_t sh_flags;
        uint64_t sh_addr;
        uint64_t sh_offset;
        uint64_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint64_t sh_addralign;
        uint64_t sh_entsize;
    };

    struct alignas(ALIGN_16) NoteSection {
        uint32_t n_namesz;
        uint32_t n_descsz;
        uint32_t n_type;
    };

    struct alignas(ALIGN_128) ThreadInfo {
        uint64_t tid;
        std::array<uint64_t, NUM_REGISTERS> registers;  // For x86_64
    };

    std::vector<ProgramHeader> programHeaders_;
    std::vector<SectionHeader> sectionHeaders_;
    std::map<std::string, std::string> sharedLibraries_;
    std::vector<ThreadInfo> threads_;
    std::map<int, std::string> signalHandlers_;
    std::vector<std::pair<uint64_t, uint64_t>> memoryMaps_;
    std::vector<int> openFileDescriptors_;
};

CoreDumpAnalyzer::CoreDumpAnalyzer() : pImpl_(std::make_unique<Impl>()) {
    LOG_F(INFO, "CoreDumpAnalyzer created");
}

CoreDumpAnalyzer::~CoreDumpAnalyzer() {
    LOG_F(INFO, "CoreDumpAnalyzer destroyed");
}

auto CoreDumpAnalyzer::readFile(const std::string& filename) -> bool {
    LOG_F(INFO, "CoreDumpAnalyzer::readFile called with filename: {}",
          filename);
    return pImpl_->readFile(filename);
}

void CoreDumpAnalyzer::analyze() {
    LOG_F(INFO, "CoreDumpAnalyzer::analyze called");
    pImpl_->analyze();
}

}  // namespace lithium::addon