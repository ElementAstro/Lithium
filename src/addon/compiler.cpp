#include "compiler.hpp"
#include "compile_command_generator.hpp"
#include "compiler_output_parser.hpp"
#include "toolchain.hpp"

#include "utils/constant.hpp"

#include <fstream>
#include <ios>
#include <memory>
#include <unordered_map>

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/to_string.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace lithium {

class CompilerImpl {
public:
    CompilerImpl();
    auto compileToSharedLibrary(std::string_view code,
                                std::string_view moduleName,
                                std::string_view functionName,
                                std::string_view optionsFile) -> bool;

    void addCompileOptions(const std::string &options);

    auto getAvailableCompilers() const -> std::vector<std::string>;

    void generateCompileCommands(const std::string &sourceDir);

private:
    auto syntaxCheck(std::string_view code,
                     const std::string &compiler) -> bool;
    auto compileCode(std::string_view code, const std::string &compiler,
                     const std::string &compileOptions,
                     const fs::path &output) -> bool;

    auto findAvailableCompilers() -> std::vector<std::string>;
    auto readCompileOptions(const std::string &optionsFile,
                            const std::vector<std::string> &availableCompilers)
        -> json;
    auto generateCompileCommand(const json &optionsJson,
                                const fs::path &tempSourceFile,
                                const fs::path &outputDir) -> std::string;

    std::unordered_map<std::string, fs::path> cache_;
    std::string customCompileOptions_;
    ToolchainManager toolchainManager_;
    std::shared_ptr<CompileCommandGenerator> compileCommandGenerator_;
    std::shared_ptr<CompilerOutputParser> compilerOutputParser_;
};

Compiler::Compiler() : impl_(std::make_unique<CompilerImpl>()) {}

Compiler::~Compiler() = default;

auto Compiler::compileToSharedLibrary(std::string_view code,
                                      std::string_view moduleName,
                                      std::string_view functionName,
                                      std::string_view optionsFile) -> bool {
    return impl_->compileToSharedLibrary(code, moduleName, functionName,
                                         optionsFile);
}

void Compiler::addCompileOptions(const std::string &options) {
    impl_->addCompileOptions(options);
}

auto Compiler::getAvailableCompilers() const -> std::vector<std::string> {
    return impl_->getAvailableCompilers();
}

CompilerImpl::CompilerImpl()
    : compileCommandGenerator_(std::make_shared<CompileCommandGenerator>()) {
    LOG_F(INFO, "Initializing CompilerImpl...");
    toolchainManager_.scanForToolchains();
    LOG_F(INFO, "Toolchains scanned.");
    auto availableCompilers = toolchainManager_.getAvailableCompilers();
    if (!availableCompilers.empty()) {
        compileCommandGenerator_->setCompiler(availableCompilers[0]);
    }
    compileCommandGenerator_->setOption("include_flag", "-I./include");
    compileCommandGenerator_->setOption("output_flag", "-o output");
    compileCommandGenerator_->addDefine(".cpp");
    compileCommandGenerator_->addDefine(".c");
    LOG_F(INFO, "CompileCommandGenerator initialized with default settings.");
}

void CompilerImpl::generateCompileCommands(const std::string &sourceDir) {
    LOG_F(INFO, "Generating compile commands for source directory: {}",
          sourceDir);
    compileCommandGenerator_->setOption("source_dir", sourceDir);
    compileCommandGenerator_->setOption("outputPath", "compile_commands.json");
    compileCommandGenerator_->generate();
    LOG_F(INFO, "Compile commands generation complete for directory: {}",
          sourceDir);
}

auto CompilerImpl::compileToSharedLibrary(
    std::string_view code, std::string_view moduleName,
    std::string_view functionName, std::string_view optionsFile) -> bool {
    LOG_F(INFO, "Compiling module {}::{} with options file: {}", moduleName,
          functionName, optionsFile);

    if (code.empty() || moduleName.empty() || functionName.empty()) {
        LOG_F(
            ERROR,
            "Invalid parameters: code, moduleName, or functionName is empty.");
        return false;
    }

    std::string cacheKey = std::format("{}::{}", moduleName, functionName);
    if (cache_.find(cacheKey) != cache_.end()) {
        LOG_F(WARNING, "Module {} is already compiled, using cached result.",
              cacheKey);
        return true;
    }

    const fs::path OUTPUT_DIR = "atom/global";
    if (atom::io::createDirectory(OUTPUT_DIR)) {
        LOG_F(INFO, "Output directory created at: {}", OUTPUT_DIR.string());
    } else {
        LOG_F(ERROR, "Failed to create output directory at: {}",
              OUTPUT_DIR.string());
        return false;
    }

    auto availableCompilers = findAvailableCompilers();
    if (availableCompilers.empty()) {
        LOG_F(ERROR, "No available compilers found.");
        return false;
    }
    LOG_F(INFO, "Available compilers: {}",
          atom::utils::toString(availableCompilers));

    // Read compile options
    auto optionsJson =
        readCompileOptions(std::string(optionsFile), availableCompilers);

    // Use CompileCommandGenerator to generate compile command
    fs::path tempSourceFile = fs::temp_directory_path() / "temp_code.cpp";
    {
        std::ofstream tempFile(tempSourceFile);
        tempFile << code;
    }
    LOG_F(INFO, "Temporary source file created at: {}",
          tempSourceFile.string());

    auto command =
        generateCompileCommand(optionsJson, tempSourceFile, OUTPUT_DIR);

    // Use the generated command to compile
    LOG_F(INFO, "Executing compilation command: {}", command);
    std::string compilationOutput = atom::system::executeCommand(command);
    if (!compilationOutput.empty()) {
        LOG_F(ERROR, "Compilation failed:\n{}", compilationOutput);
        // 解析编译输出
        std::istringstream outputStream(compilationOutput);
        std::string line;
        while (std::getline(outputStream, line)) {
            compilerOutputParser_->parseLine(line);
        }
        fs::remove(tempSourceFile);
        return false;
    }

    // Cache the compilation result
    cache_[cacheKey] =
        OUTPUT_DIR / std::format("{}{}{}", Constants::LIB_EXTENSION, moduleName,
                                 Constants::LIB_EXTENSION);
    LOG_F(INFO, "Compilation successful, result cached with key: {}", cacheKey);
    fs::remove(tempSourceFile);
    return true;
}

auto CompilerImpl::syntaxCheck(std::string_view code,
                               const std::string &compiler) -> bool {
    LOG_F(INFO, "Starting syntax check using compiler: {}", compiler);
    compileCommandGenerator_->setCompiler(compiler);
    compileCommandGenerator_->setOption("include_flag", "-fsyntax-only");
    compileCommandGenerator_->setOption("output_flag", "");

    fs::path tempSourceFile = fs::temp_directory_path() / "syntax_check.cpp";
    {
        std::ofstream tempFile(tempSourceFile);
        tempFile << code;
    }
    LOG_F(INFO, "Temporary file for syntax check created at: {}",
          tempSourceFile.string());

    compileCommandGenerator_->setOption("source_dir",
                                        tempSourceFile.parent_path().string());
    compileCommandGenerator_->setOption(
        "outputPath",
        (fs::temp_directory_path() / "syntax_check_commands.json").string());
    compileCommandGenerator_->generate();

    json syntaxCheckCommands;
    {
        std::ifstream commandsFile(fs::temp_directory_path() /
                                   "syntax_check_commands.json");
        commandsFile >> syntaxCheckCommands;
    }
    LOG_F(INFO, "Syntax check commands file read.");

    if (!syntaxCheckCommands["commands"].empty()) {
        auto command =
            syntaxCheckCommands["commands"][0]["command"].get<std::string>();
        LOG_F(INFO, "Executing syntax check command: {}", command);
        std::string output = atom::system::executeCommand(command);

        fs::remove(tempSourceFile);
        fs::remove(fs::temp_directory_path() / "syntax_check_commands.json");

        if (!output.empty()) {
            LOG_F(ERROR, "Syntax check failed:\n{}", output);
            return false;
        }
        LOG_F(INFO, "Syntax check passed.");
        return true;
    }
    LOG_F(ERROR, "Failed to generate syntax check command.");
    fs::remove(tempSourceFile);
    fs::remove(fs::temp_directory_path() / "syntax_check_commands.json");
    return false;
}

auto CompilerImpl::compileCode(std::string_view code,
                               const std::string &compiler,
                               const std::string &compileOptions,
                               const fs::path &output) -> bool {
    LOG_F(INFO,
          "Starting compilation with compiler: {}, options: {}, output: {}",
          compiler, compileOptions, output.string());

    // Use CompileCommandGenerator to generate the compile command
    compileCommandGenerator_->setCompiler(compiler);
    compileCommandGenerator_->setOption("include_flag", compileOptions);
    compileCommandGenerator_->setOption("output_flag", "-o " + output.string());

    fs::path tempSourceFile = fs::temp_directory_path() / "compile_code.cpp";
    {
        std::ofstream tempFile(tempSourceFile);
        tempFile << code;
    }
    LOG_F(INFO, "Temporary file for compilation created at: {}",
          tempSourceFile.string());

    compileCommandGenerator_->setOption("source_dir",
                                        tempSourceFile.parent_path().string());
    compileCommandGenerator_->setOption(
        "outputPath",
        (fs::temp_directory_path() / "compile_code_commands.json").string());
    compileCommandGenerator_->generate();

    json compileCodeCommands;
    {
        std::ifstream commandsFile(fs::temp_directory_path() /
                                   "compile_code_commands.json");
        commandsFile >> compileCodeCommands;
    }
    LOG_F(INFO, "Compile commands file read.");

    if (!compileCodeCommands["commands"].empty()) {
        auto command =
            compileCodeCommands["commands"][0]["command"].get<std::string>();
        LOG_F(INFO, "Executing compilation command: {}", command);
        std::string compilationOutput = atom::system::executeCommand(command);

        fs::remove(tempSourceFile);
        fs::remove(fs::temp_directory_path() / "compile_code_commands.json");

        if (!compilationOutput.empty()) {
            LOG_F(ERROR, "Compilation failed:\n{}", compilationOutput);
            return false;
        }
        LOG_F(INFO, "Compilation successful, output file: {}", output.string());
        return true;
    }

    LOG_F(ERROR, "Failed to generate compile command.");
    fs::remove(tempSourceFile);
    fs::remove(fs::temp_directory_path() / "compile_code_commands.json");
    return false;
}

auto CompilerImpl::findAvailableCompilers() -> std::vector<std::string> {
    LOG_F(INFO, "Finding available compilers...");
    auto compilers = toolchainManager_.getAvailableCompilers();
    if (compilers.empty()) {
        LOG_F(WARNING, "No compilers found.");
    } else {
        LOG_F(INFO, "Found compilers: {}", atom::utils::toString(compilers));
    }
    return compilers;
}

void CompilerImpl::addCompileOptions(const std::string &options) {
    LOG_F(INFO, "Adding custom compile options: {}", options);
    customCompileOptions_ = options;
}

auto CompilerImpl::getAvailableCompilers() const -> std::vector<std::string> {
    LOG_F(INFO, "Retrieving available compilers...");
    return toolchainManager_.getAvailableCompilers();
}

auto CompilerImpl::readCompileOptions(
    const std::string &optionsFile,
    const std::vector<std::string> &availableCompilers) -> json {
    std::ifstream optionsStream(optionsFile);
    json optionsJson;
    if (!optionsStream) {
        LOG_F(WARNING,
              "Failed to open compile options file {}, using default options.",
              optionsFile);
        optionsJson = {{"compiler", availableCompilers[0]},
                       {"optimization_level", "-O2"},
                       {"cplus_version", "-std=c++20"},
                       {"warnings", "-Wall"},
                       {"include_paths", json::array()},
                       {"library_paths", json::array()},
                       {"defines", json::array()}};
    } else {
        try {
            optionsStream >> optionsJson;
            LOG_F(INFO, "Compile options file {} successfully parsed.",
                  optionsFile);
        } catch (const json::parse_error &e) {
            LOG_F(ERROR, "Failed to parse compile options file {}: {}",
                  optionsFile, e.what());
            throw std::runtime_error("Failed to parse compile options file");
        }
    }
    return optionsJson;
}

auto CompilerImpl::generateCompileCommand(
    const json &optionsJson, const fs::path &tempSourceFile,
    const fs::path &outputDir) -> std::string {
    std::string compiler = optionsJson.value("compiler", "");
    compileCommandGenerator_->setCompiler(compiler);
    compileCommandGenerator_->setOption(
        "include_flag", optionsJson.value("include_flag", "-I./include"));
    compileCommandGenerator_->setOption(
        "output_flag", optionsJson.value("output_flag", "-o output"));
    compileCommandGenerator_->setOption(
        "project_name", optionsJson.value("project_name", "project"));
    compileCommandGenerator_->setOption(
        "project_version", optionsJson.value("project_version", "1.0.0"));

    for (const auto &includePath :
         optionsJson.value("include_paths", json::array())) {
        compileCommandGenerator_->addDefine(includePath.get<std::string>());
    }
    for (const auto &libraryPath :
         optionsJson.value("library_paths", json::array())) {
        compileCommandGenerator_->addLibrary(libraryPath.get<std::string>());
    }
    for (const auto &define : optionsJson.value("defines", json::array())) {
        compileCommandGenerator_->addDefine(define.get<std::string>());
    }

    compileCommandGenerator_->setOption("source_dir",
                                        tempSourceFile.parent_path().string());
    compileCommandGenerator_->setOption(
        "outputPath", (outputDir / "compile_commands.json").string());
    compileCommandGenerator_->generate();

    // Read generated compile_commands.json
    json compileCommands;
    {
        std::ifstream commandsFile(outputDir / "compile_commands.json");
        commandsFile >> compileCommands;
    }
    LOG_F(INFO, "Compile commands file read from: {}",
          (outputDir / "compile_commands.json").string());

    if (!compileCommands["commands"].empty()) {
        auto command =
            compileCommands["commands"][0]["command"].get<std::string>();
        command += " " + customCompileOptions_;
        command +=
            " -o " + (outputDir /
                      std::format("{}{}{}", Constants::LIB_EXTENSION,
                                  optionsJson.value("project_name", "project"),
                                  Constants::LIB_EXTENSION))
                         .string();
        return command;
    }

    throw std::runtime_error("Failed to generate compile command");
}

void Compiler::generateCompileCommands(const std::string &sourceDir) {
    LOG_F(INFO,
          "Generating compile commands in Compiler for source directory: {}",
          sourceDir);
    impl_->generateCompileCommands(sourceDir);
}

}  // namespace lithium