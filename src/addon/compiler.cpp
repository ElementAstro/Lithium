#include "compiler.hpp"
#include "command.hpp"
#include "toolchain.hpp"

#include "utils/constant.hpp"

#include <fstream>
#include <ios>
#include <unordered_map>
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
    void createOutputDirectory(const fs::path &outputDir);
    auto syntaxCheck(std::string_view code,
                     const std::string &compiler) -> bool;
    auto compileCode(std::string_view code, const std::string &compiler,
                     const std::string &compileOptions,
                     const fs::path &output) -> bool;

    auto findAvailableCompilers() -> std::vector<std::string>;

    std::unordered_map<std::string, fs::path> cache_;
    std::string customCompileOptions_;
    ToolchainManager toolchainManager_;
    std::unique_ptr<CompileCommandGenerator> compileCommandGenerator_;
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
    : compileCommandGenerator_(std::make_unique<CompileCommandGenerator>()) {
    LOG_F(INFO, "Initializing CompilerImpl...");
    toolchainManager_.scanForToolchains();
    LOG_F(INFO, "Toolchains scanned.");
    auto availableCompilers = toolchainManager_.getAvailableCompilers();
    if (!availableCompilers.empty()) {
        compileCommandGenerator_->setCompiler(availableCompilers[0]);
    }
    compileCommandGenerator_->setIncludeFlag("-I./include");
    compileCommandGenerator_->setOutputFlag("-o output");
    compileCommandGenerator_->addExtension(".cpp");
    compileCommandGenerator_->addExtension(".c");
    LOG_F(INFO, "CompileCommandGenerator initialized with default settings.");
}

void CompilerImpl::generateCompileCommands(const std::string &sourceDir) {
    LOG_F(INFO, "Generating compile commands for source directory: {}",
          sourceDir);
    compileCommandGenerator_->setSourceDir(sourceDir);
    compileCommandGenerator_->setOutputPath("compile_commands.json");
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
    createOutputDirectory(OUTPUT_DIR);

    auto availableCompilers = findAvailableCompilers();
    if (availableCompilers.empty()) {
        LOG_F(ERROR, "No available compilers found.");
        return false;
    }
    LOG_F(INFO, "Available compilers: {}",
          atom::utils::toString(availableCompilers));

    // Read compile options
    std::ifstream optionsStream(optionsFile.data());
    json optionsJson;
    if (!optionsStream) {
        LOG_F(WARNING,
              "Failed to open compile options file {}, using default options.",
              optionsFile);
        optionsJson = {{"compiler", availableCompilers[0]},
                       {"optimization_level", "-O2"},
                       {"cplus_version", "-std=c++20"},
                       {"warnings", "-Wall"}};
    } else {
        try {
            optionsStream >> optionsJson;
            LOG_F(INFO, "Compile options file {} successfully parsed.",
                  optionsFile);
        } catch (const json::parse_error &e) {
            LOG_F(ERROR, "Failed to parse compile options file {}: {}",
                  optionsFile, e.what());
            return false;
        }
    }

    std::string compiler = optionsJson.value("compiler", availableCompilers[0]);
    if (std::find(availableCompilers.begin(), availableCompilers.end(),
                  compiler) == availableCompilers.end()) {
        LOG_F(WARNING,
              "Compiler {} is not available, using default compiler {}.",
              compiler, availableCompilers[0]);
        compiler = availableCompilers[0];
    }

    // Use CompileCommandGenerator to generate compile command
    compileCommandGenerator_->setCompiler(compiler);
    compileCommandGenerator_->setIncludeFlag(
        optionsJson.value("include_flag", "-I./include"));
    compileCommandGenerator_->setOutputFlag(
        optionsJson.value("output_flag", "-o output"));
    compileCommandGenerator_->setProjectName(std::string(moduleName));
    compileCommandGenerator_->setProjectVersion("1.0.0");

    // Temporarily create a file to store the code
    fs::path tempSourceFile = fs::temp_directory_path() / "temp_code.cpp";
    {
        std::ofstream tempFile(tempSourceFile);
        tempFile << code;
    }
    LOG_F(INFO, "Temporary source file created at: {}",
          tempSourceFile.string());

    compileCommandGenerator_->setSourceDir(
        tempSourceFile.parent_path().string());
    compileCommandGenerator_->setOutputPath(
        (OUTPUT_DIR / "compile_commands.json").string());
    compileCommandGenerator_->generate();

    // Read generated compile_commands.json
    json compileCommands;
    {
        std::ifstream commandsFile(OUTPUT_DIR / "compile_commands.json");
        commandsFile >> compileCommands;
    }
    LOG_F(INFO, "Compile commands file read from: {}",
          (OUTPUT_DIR / "compile_commands.json").string());

    // Use the generated command to compile
    if (!compileCommands["commands"].empty()) {
        auto command =
            compileCommands["commands"][0]["command"].get<std::string>();
        command += " " + customCompileOptions_;

        fs::path outputPath =
            OUTPUT_DIR / std::format("{}{}{}", Constants::LIB_EXTENSION,
                                     moduleName, Constants::LIB_EXTENSION);
        command += " -o " + outputPath.string();

        LOG_F(INFO, "Executing compilation command: {}", command);
        std::string compilationOutput = atom::system::executeCommand(command);
        if (!compilationOutput.empty()) {
            LOG_F(ERROR, "Compilation failed:\n{}", compilationOutput);
            fs::remove(tempSourceFile);
            return false;
        }

        // Cache the compilation result
        cache_[cacheKey] = outputPath;
        LOG_F(INFO, "Compilation successful, result cached with key: {}",
              cacheKey);
        fs::remove(tempSourceFile);
        return true;
    }

    LOG_F(ERROR, "Failed to generate compile command.");
    fs::remove(tempSourceFile);
    return false;
}

void CompilerImpl::createOutputDirectory(const fs::path &outputDir) {
    if (!fs::exists(outputDir)) {
        LOG_F(WARNING, "Output directory {} does not exist, creating it.",
              outputDir.string());
        fs::create_directories(outputDir);
        LOG_F(INFO, "Output directory {} created.", outputDir.string());
    } else {
        LOG_F(INFO, "Output directory {} already exists.", outputDir.string());
    }
}

auto CompilerImpl::syntaxCheck(std::string_view code,
                               const std::string &compiler) -> bool {
    LOG_F(INFO, "Starting syntax check using compiler: {}", compiler);
    compileCommandGenerator_->setCompiler(compiler);
    compileCommandGenerator_->setIncludeFlag("-fsyntax-only");
    compileCommandGenerator_->setOutputFlag("");

    fs::path tempSourceFile = fs::temp_directory_path() / "syntax_check.cpp";
    {
        std::ofstream tempFile(tempSourceFile);
        tempFile << code;
    }
    LOG_F(INFO, "Temporary file for syntax check created at: {}",
          tempSourceFile.string());

    compileCommandGenerator_->setSourceDir(
        tempSourceFile.parent_path().string());
    compileCommandGenerator_->setOutputPath(
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
    compileCommandGenerator_->setIncludeFlag(compileOptions);
    compileCommandGenerator_->setOutputFlag("-o " + output.string());

    fs::path tempSourceFile = fs::temp_directory_path() / "compile_code.cpp";
    {
        std::ofstream tempFile(tempSourceFile);
        tempFile << code;
    }
    LOG_F(INFO, "Temporary file for compilation created at: {}",
          tempSourceFile.string());

    compileCommandGenerator_->setSourceDir(
        tempSourceFile.parent_path().string());
    compileCommandGenerator_->setOutputPath(
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

void Compiler::generateCompileCommands(const std::string &sourceDir) {
    LOG_F(INFO,
          "Generating compile commands in Compiler for source directory: {}",
          sourceDir);
    impl_->generateCompileCommands(sourceDir);
}

}  // namespace lithium
