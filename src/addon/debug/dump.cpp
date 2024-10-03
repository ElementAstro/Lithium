#include "dump.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

constexpr size_t ELF_IDENT_SIZE = 16;
constexpr size_t NUM_REGISTERS = 27;
constexpr size_t NUM_GENERAL_REGISTERS = 24;
constexpr uint32_t SHT_NOTE = 7;
constexpr uint32_t SHT_PROGBITS = 1;
constexpr uint32_t SHT_SYMTAB = 2;
constexpr uint32_t SHT_STRTAB = 3;
constexpr uint32_t PT_LOAD = 1;
constexpr size_t ALIGN_64 = 64;
constexpr size_t ALIGN_16 = 16;
constexpr size_t ALIGN_128 = 128;

namespace lithium::addon {
class CoreDumpAnalyzer::Impl {
public:
    std::vector<uint8_t> data_;
    struct alignas(ALIGN_64) ElfHeader {
        std::array<uint8_t, ELF_IDENT_SIZE> eIdent;
        uint16_t eType;
        uint16_t eMachine;
        uint32_t eVersion;
        uint64_t eEntry;
        uint64_t ePhoff;
        uint64_t eShoff;
        uint32_t eFlags;
        uint16_t eEhsize;
        uint16_t ePhentsize;
        uint16_t ePhnum;
        uint16_t eShentsize;
        uint16_t eShnum;
        uint16_t eShstrndx;
    };
    ElfHeader header_{};

    struct alignas(ALIGN_64) ProgramHeader {
        uint32_t pType;
        uint32_t pFlags;
        uint64_t pOffset;
        uint64_t pVaddr;
        uint64_t pPaddr;
        uint64_t pFilesz;
        uint64_t pMemsz;
        uint64_t pAlign;
    };

    struct alignas(ALIGN_64) SectionHeader {
        uint32_t shName;
        uint32_t shType;
        uint64_t shFlags;
        uint64_t shAddr;
        uint64_t shOffset;
        uint64_t shSize;
        uint32_t shLink;
        uint32_t shInfo;
        uint64_t shAddralign;
        uint64_t shEntsize;
    };

    struct alignas(ALIGN_16) NoteSection {
        uint32_t nNamesz;
        uint32_t nDescsz;
        uint32_t nType;
    };

    struct alignas(ALIGN_128) ThreadInfo {
        uint64_t tid;
        std::array<uint64_t, NUM_REGISTERS> registers;  // For x86_64
    };

    std::vector<ProgramHeader> programHeaders;
    std::vector<SectionHeader> sectionHeaders;
    std::map<std::string, std::string> sharedLibraries;
    std::vector<ThreadInfo> threads;
    std::map<int, std::string> signalHandlers;
    std::vector<std::pair<uint64_t, uint64_t>> memoryMaps;
    std::vector<int> openFileDescriptors;

    bool readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Unable to open file: " << filename << std::endl;
            return false;
        }

        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        data_.resize(fileSize);
        file.read(reinterpret_cast<char*>(data_.data()),
                  static_cast<std::streamsize>(fileSize));

        if (!file) {
            std::cerr << "Error reading file" << std::endl;
            return false;
        }

        if (fileSize < sizeof(ElfHeader)) {
            std::cerr << "File too small to be a valid ELF format" << std::endl;
            return false;
        }

        std::memcpy(&header_, data_.data(), sizeof(ElfHeader));

        return true;
    }

    std::string getElfHeaderInfo() const {
        std::ostringstream oss;
        oss << "ELF Header:\n";
        oss << "  Type: " << header_.eType << "\n";
        oss << "  Machine: " << header_.eMachine << "\n";
        oss << "  Version: " << header_.eVersion << "\n";
        oss << "  Entry point address: 0x" << std::hex << header_.eEntry
            << "\n";
        oss << "  Start of program headers: " << header_.ePhoff
            << " (bytes into file)\n";
        oss << "  Start of section headers: " << header_.eShoff
            << " (bytes into file)\n";
        oss << "  Flags: 0x" << std::hex << header_.eFlags << "\n";
        oss << "  Size of this header: " << header_.eEhsize << " (bytes)\n";
        oss << "  Size of program headers: " << header_.ePhentsize
            << " (bytes)\n";
        oss << "  Number of program headers: " << header_.ePhnum << "\n";
        oss << "  Size of section headers: " << header_.eShentsize
            << " (bytes)\n";
        oss << "  Number of section headers: " << header_.eShnum << "\n";
        oss << "  Section header string table index: " << header_.eShstrndx
            << "\n";
        return oss.str();
    }

    std::string getProgramHeadersInfo() const {
        std::ostringstream oss;
        oss << "Program Headers:\n";
        for (const auto& programHeader : programHeaders) {
            oss << "  Type: " << programHeader.pType << "\n";
            oss << "  Offset: 0x" << std::hex << programHeader.pOffset << "\n";
            oss << "  Virtual address: 0x" << std::hex << programHeader.pVaddr
                << "\n";
            oss << "  Physical address: 0x" << std::hex << programHeader.pPaddr
                << "\n";
            oss << "  File size: " << programHeader.pFilesz << "\n";
            oss << "  Memory size: " << programHeader.pMemsz << "\n";
            oss << "  Flags: 0x" << std::hex << programHeader.pFlags << "\n";
            oss << "  Align: " << programHeader.pAlign << "\n";
        }
        return oss.str();
    }

    std::string getSectionHeadersInfo() const {
        std::ostringstream oss;
        oss << "Section Headers:\n";
        for (const auto& sectionHeader : sectionHeaders) {
            oss << "  Name: " << sectionHeader.shName << "\n";
            oss << "  Type: " << sectionHeader.shType << "\n";
            oss << "  Flags: 0x" << std::hex << sectionHeader.shFlags << "\n";
            oss << "  Address: 0x" << std::hex << sectionHeader.shAddr << "\n";
            oss << "  Offset: 0x" << std::hex << sectionHeader.shOffset << "\n";
            oss << "  Size: " << sectionHeader.shSize << "\n";
            oss << "  Link: " << sectionHeader.shLink << "\n";
            oss << "  Info: " << sectionHeader.shInfo << "\n";
            oss << "  Address align: " << sectionHeader.shAddralign << "\n";
            oss << "  Entry size: " << sectionHeader.shEntsize << "\n";
        }
        return oss.str();
    }

    std::string getNoteSectionInfo() const {
        std::ostringstream oss;
        oss << "Note Sections:\n";
        for (const auto& section : sectionHeaders) {
            if (section.shType == SHT_NOTE) {
                size_t offset = section.shOffset;
                while (offset < section.shOffset + section.shSize) {
                    NoteSection note{};
                    std::memcpy(&note, data_.data() + offset,
                                sizeof(NoteSection));
                    offset += sizeof(NoteSection);

                    std::string name(
                        reinterpret_cast<const char*>(data_.data() + offset),
                        note.nNamesz - 1);
                    offset += note.nNamesz;

                    oss << "  Note: " << name << ", Type: 0x" << std::hex
                        << note.nType << ", Size: " << note.nDescsz
                        << " bytes\n";

                    if (name == "CORE" && note.nType == 1) {
                        oss << getThreadInfo(offset);
                    } else if (name == "CORE" && note.nType == 4) {
                        oss << getFileInfo(offset);
                    }

                    offset += note.nDescsz;
                }
            }
        }
        return oss.str();
    }

    std::string getThreadInfo(size_t offset) const {
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

    std::string getFileInfo(size_t offset) const {
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

    std::string getMemoryMapsInfo() const {
        std::ostringstream oss;
        oss << "Memory Maps:\n";
        for (const auto& programHeader : programHeaders) {
            if (programHeader.pType == PT_LOAD) {
                oss << "  Mapping: 0x" << std::hex << programHeader.pVaddr
                    << " - 0x" << std::hex
                    << (programHeader.pVaddr + programHeader.pMemsz)
                    << " (Size: 0x" << std::hex << programHeader.pMemsz
                    << " bytes)\n";
            }
        }
        return oss.str();
    }

    std::string getSignalHandlersInfo() const {
        std::ostringstream oss;
        oss << "Signal Handlers:\n";
        for (const auto& section : sectionHeaders) {
            if (section.shType == SHT_NOTE &&
                section.shSize >= sizeof(uint64_t) * 2) {
                uint64_t signalNum = *reinterpret_cast<const uint64_t*>(
                    data_.data() + section.shOffset);
                uint64_t handlerAddr = *reinterpret_cast<const uint64_t*>(
                    data_.data() + section.shOffset + sizeof(uint64_t));
                oss << "  Signal " << signalNum << ": Handler Address 0x"
                    << std::hex << handlerAddr << "\n";
            }
        }
        return oss.str();
    }

    std::string getHeapUsageInfo() const {
        std::ostringstream oss;
        oss << "Heap Usage:\n";
        auto heapSection =
            std::find_if(sectionHeaders.begin(), sectionHeaders.end(),
                         [](const SectionHeader& sectionHeader) {
                             return sectionHeader.shType == SHT_PROGBITS &&
                                    ((sectionHeader.shFlags & 0x1U) != 0U);
                         });

        if (heapSection != sectionHeaders.end()) {
            oss << "  Heap Region: 0x" << std::hex << heapSection->shAddr
                << " - 0x" << std::hex
                << (heapSection->shAddr + heapSection->shSize) << " (Size: 0x"
                << std::hex << heapSection->shSize << " bytes)\n";
        } else {
            oss << "  No explicit heap region found\n";
        }
        return oss.str();
    }

    void analyze() {
        if (data_.empty()) {
            std::cout << "No data to analyze" << std::endl;
            return;
        }

        if (std::memcmp(header_.eIdent.data(),
                        "\x7F"
                        "ELF",
                        4) != 0) {
            std::cerr << "Not a valid ELF file" << std::endl;
            return;
        }

        std::cout << "File size: " << data_.size() << " bytes" << std::endl;
        std::cout << getElfHeaderInfo();
        std::cout << getProgramHeadersInfo();
        std::cout << getSectionHeadersInfo();
        std::cout << getNoteSectionInfo();
        std::cout << getMemoryMapsInfo();
        std::cout << getSignalHandlersInfo();
        std::cout << getHeapUsageInfo();
    }
};

CoreDumpAnalyzer::CoreDumpAnalyzer() : pImpl_(std::make_unique<Impl>()) {}
CoreDumpAnalyzer::~CoreDumpAnalyzer() = default;

bool CoreDumpAnalyzer::readFile(const std::string& filename) {
    return pImpl_->readFile(filename);
}

void CoreDumpAnalyzer::analyze() { pImpl_->analyze(); }
}  // namespace lithium::addon
