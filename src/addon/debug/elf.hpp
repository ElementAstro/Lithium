#ifndef LITHIUM_ADDON_ELF_HPP
#define LITHIUM_ADDON_ELF_HPP

#include <memory>
#include <vector>

namespace lithium {
struct ElfHeader;
struct ProgramHeader;
struct SectionHeader;
struct Symbol;
struct DynamicEntry;
struct RelocationEntry;

class ElfParser {
public:
    explicit ElfParser(const char* file);

    bool parse();
    ElfHeader getElfHeader() const;
    std::vector<ProgramHeader> getProgramHeaders() const;
    std::vector<SectionHeader> getSectionHeaders() const;
    std::vector<Symbol> getSymbolTable() const;
    std::vector<DynamicEntry> getDynamicEntries() const;
    std::vector<RelocationEntry> getRelocationEntries() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_ELF_HPP
