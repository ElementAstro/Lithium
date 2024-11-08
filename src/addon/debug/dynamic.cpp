#include "dynamic.hpp"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#ifdef __linux__
#include <elf.h>
#elif defined(__APPLE__)
// Apple-specific includes can go here if needed
#else
#include <tchar.h>
#include <windows.h>
#include <string>
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/type/json.hpp"

namespace lithium::addon {
using json = nlohmann::json;

class DynamicLibraryParser::Impl {
public:
    explicit Impl(std::string executable) : executable_(std::move(executable)) {
        LOG_F(INFO, "Initialized DynamicLibraryParser for executable: {}",
              executable_);
    }

    void setJsonOutput(bool json_output) {
        json_output_ = json_output;
        LOG_F(INFO, "Set JSON output to: {}", json_output_ ? "true" : "false");
    }

    void setOutputFilename(const std::string &filename) {
        output_filename_ = filename;
        LOG_F(INFO, "Set output filename to: {}", output_filename_);
    }

    void parse() {
        LOG_SCOPE_FUNCTION(INFO);
        try {
#ifdef __linux__
            readDynamicLibraries();
#endif
            executePlatformCommand();
            if (json_output_) {
                handleJsonOutput();
            }
            LOG_F(INFO, "Parse process completed successfully.");
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Exception caught during parsing: {}", e.what());
            throw;
        }
    }

private:
    std::string executable_;
    bool json_output_{};
    std::string output_filename_;
    std::vector<std::string> libraries_;
    std::string command_output_;

    void readDynamicLibraries() {
        LOG_SCOPE_FUNCTION(INFO);
        std::ifstream file(executable_, std::ios::binary);
        if (!file) {
            LOG_F(ERROR, "Failed to open file: {}", executable_);
            THROW_FAIL_TO_OPEN_FILE("Failed to open file: " + executable_);
        }

        // Read ELF header
        Elf64_Ehdr elfHeader;
        file.read(reinterpret_cast<char *>(&elfHeader), sizeof(elfHeader));
        if (std::memcmp(elfHeader.e_ident, ELFMAG, SELFMAG) != 0) {
            LOG_F(ERROR, "Not a valid ELF file: {}", executable_);
            THROW_RUNTIME_ERROR("Not a valid ELF file: " + executable_);
        }

        // Read section headers
        file.seekg(static_cast<std::ifstream::off_type>(elfHeader.e_shoff),
                   std::ios::beg);
        std::vector<Elf64_Shdr> sectionHeaders(elfHeader.e_shnum);
        file.read(reinterpret_cast<char *>(sectionHeaders.data()),
                  static_cast<std::streamsize>(elfHeader.e_shnum *
                                               sizeof(Elf64_Shdr)));

        // Find the dynamic section
        for (const auto &section : sectionHeaders) {
            if (section.sh_type == SHT_DYNAMIC) {
                file.seekg(
                    static_cast<std::ifstream::off_type>(section.sh_offset),
                    std::ios::beg);
                std::vector<Elf64_Dyn> dynamic_entries(section.sh_size /
                                                       sizeof(Elf64_Dyn));
                file.read(reinterpret_cast<char *>(dynamic_entries.data()),
                          static_cast<std::streamsize>(section.sh_size));

                // Read dynamic string table
                Elf64_Shdr strtabHeader = sectionHeaders[section.sh_link];
                std::vector<char> strtab(strtabHeader.sh_size);
                file.seekg(static_cast<std::ifstream::off_type>(
                               strtabHeader.sh_offset),
                           std::ios::beg);
                file.read(strtab.data(),
                          static_cast<std::streamsize>(strtabHeader.sh_size));

                // Collect needed libraries
                LOG_F(INFO, "Needed libraries from ELF:");
                for (const auto &entry : dynamic_entries) {
                    if (entry.d_tag == DT_NEEDED) {
                        std::string lib(&strtab[entry.d_un.d_val]);
                        libraries_.emplace_back(lib);
                        LOG_F(INFO, " - {}", lib);
                    }
                }
                break;
            }
        }

        if (libraries_.empty()) {
            LOG_F(WARNING, "No dynamic libraries found in ELF file.");
        }
    }

    void executePlatformCommand() {
        LOG_SCOPE_FUNCTION(INFO);
        std::string command;

#ifdef __APPLE__
        command = "otool -L ";
#elif __linux__
        command = "ldd ";
#elif defined(_WIN32)
        command = "dumpbin /dependents ";
#else
#error "Unsupported OS"
#endif

        command += executable_;
        LOG_F(INFO, "Running command: {}", command);

        auto [output, status] = atom::system::executeCommandWithStatus(command);

        command_output_ = output;
        LOG_F(INFO, "Command output: \n{}", command_output_);
    }

    void handleJsonOutput() {
        LOG_SCOPE_FUNCTION(INFO);
        std::string jsonContent = getDynamicLibrariesAsJson();
        if (!output_filename_.empty()) {
            writeOutputToFile(jsonContent);
        } else {
            LOG_F(INFO, "JSON output:\n{}", jsonContent);
        }
    }

    std::string getDynamicLibrariesAsJson() const {
        LOG_SCOPE_FUNCTION(INFO);
        json jsonOutput;
        jsonOutput["executable"] = executable_;
        jsonOutput["libraries"] = libraries_;
        jsonOutput["command_output"] = command_output_;
        return jsonOutput.dump(4);
    }

    void writeOutputToFile(const std::string &content) const {
        LOG_SCOPE_FUNCTION(INFO);
        std::ofstream outFile(output_filename_);
        if (outFile) {
            outFile << content;
            outFile.close();
            LOG_F(INFO, "Output successfully written to {}", output_filename_);
        } else {
            LOG_F(ERROR, "Failed to write to file: {}", output_filename_);
            throw std::runtime_error("Failed to write to file: " +
                                     output_filename_);
        }
    }
};

DynamicLibraryParser::DynamicLibraryParser(const std::string &executable)
    : impl_(std::make_unique<Impl>(executable)) {}

DynamicLibraryParser::~DynamicLibraryParser() = default;

void DynamicLibraryParser::setJsonOutput(bool json_output) {
    impl_->setJsonOutput(json_output);
}

void DynamicLibraryParser::setOutputFilename(const std::string &filename) {
    impl_->setOutputFilename(filename);
}

void DynamicLibraryParser::parse() { impl_->parse(); }

}  // namespace lithium::addon
