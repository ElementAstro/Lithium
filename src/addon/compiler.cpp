#include "compiler.hpp"
#include "io/io.hpp"
#include "toolchain.hpp"

#include "utils/constant.hpp"

#include <fstream>
#include <functional>
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

private:
    void createOutputDirectory(const fs::path &outputDir);
    auto syntaxCheck(std::string_view code, std::string_view compiler) -> bool;
    auto compileCode(std::string_view code, std::string_view compiler,
                     std::string_view compileOptions,
                     const fs::path &output) -> bool;

    auto findAvailableCompilers() -> std::vector<std::string>;

    std::unordered_map<std::string, fs::path> cache_;
    std::string customCompileOptions_;
    ToolchainManager toolchainManager_;
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

CompilerImpl::CompilerImpl() { toolchainManager_.scanForToolchains(); }

auto CompilerImpl::compileToSharedLibrary(
    std::string_view code, std::string_view moduleName,
    std::string_view functionName, std::string_view optionsFile) -> bool {
    LOG_F(INFO, "Compiling module {}::{}...", moduleName, functionName);

    if (code.empty() || moduleName.empty() || functionName.empty()) {
        LOG_F(ERROR, "Invalid parameters.");
        return false;
    }

    // 检查模块是否已编译并缓存
    std::string cacheKey = std::format("{}::{}", moduleName, functionName);
    if (cache_.find(cacheKey) != cache_.end()) {
        LOG_F(WARNING, "Module {} is already compiled, using cached result.",
              cacheKey);
        return true;
    }

    // 创建输出目录
    const fs::path OUTPUT_DIR = "atom/global";
    createOutputDirectory(OUTPUT_DIR);

    // Max: 检查可用的编译器，然后再和指定的进行比对
    auto availableCompilers = findAvailableCompilers();
    if (availableCompilers.empty()) {
        LOG_F(ERROR, "No available compilers found.");
        return false;
    }
    LOG_F(INFO, "Available compilers: {}",
          atom::utils::toString(availableCompilers));

    // 读取编译选项
    std::ifstream optionsStream(optionsFile.data());
    std::string compileOptions = [&] -> std::string {
        if (!optionsStream) {
            LOG_F(
                WARNING,
                "Failed to open compile options file, using default options.");
            return "-O2 -std=c++20 -Wall -shared -fPIC";
        }

        try {
            json optionsJson;
            optionsStream >> optionsJson;

            auto compiler = optionsJson.value("compiler", constants::COMPILER);
            if (std::find(availableCompilers.begin(), availableCompilers.end(),
                          compiler) == availableCompilers.end()) {
                LOG_F(WARNING, "Compiler {} is not available, using default.",
                      compiler);
                compiler = constants::COMPILER;
            }

            auto cmd = std::format(
                "{} {} {} {} {}", compiler,
                optionsJson.value("optimization_level", "-O2"),
                optionsJson.value("cplus_version", "-std=c++20"),
                optionsJson.value("warnings", "-Wall"), customCompileOptions_);

            LOG_F(INFO, "Compile options: {}", cmd);
            return cmd;

        } catch (const json::parse_error &e) {
            LOG_F(ERROR, "Failed to parse compile options file: {}", e.what());
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Failed to parse compile options file: {}", e.what());
        }

        return constants::COMPILER +
               std::string{"-O2 -std=c++20 -Wall -shared -fPIC"};
    }();

    // 语法检查
    if (!syntaxCheck(code, constants::COMPILER)) {
        return false;
    }

    // 编译代码
    fs::path outputPath =
        OUTPUT_DIR / std::format("{}{}{}", constants::LIB_EXTENSION, moduleName,
                                 constants::LIB_EXTENSION);
    if (!compileCode(code, constants::COMPILER, compileOptions, outputPath)) {
        return false;
    }

    // 缓存编译结果
    cache_[cacheKey] = outputPath;
    return true;
}

void CompilerImpl::createOutputDirectory(const fs::path &outputDir) {
    if (!fs::exists(outputDir)) {
        LOG_F(WARNING, "Output directory {} does not exist, creating it.",
              outputDir.string());
        fs::create_directories(outputDir);
    }
}

auto CompilerImpl::syntaxCheck(std::string_view code,
                               std::string_view compiler) -> bool {
    if (atom::io::isFileExists("temp_code.cpp")) {
        if (!atom::io::removeFile("temp_code.cpp")) {
            LOG_F(ERROR, "Failed to remove temp_code.cpp");
            return false;
        }
    }
    // Create a temporary file to store the code
    std::string tempFileName = "temp_code.cpp";
    {
        std::ofstream tempFile(tempFileName, std::ios::trunc);
        if (!tempFile) {
            LOG_F(ERROR, "Failed to create temporary file for code");
            return false;
        }
        tempFile << code;
    }

    // Format the command to invoke the compiler with syntax-only check
    std::string command =
        std::format("{} -fsyntax-only -x c++ {}", compiler, tempFileName);
    std::string output;

    // Execute the command and process output
    output = atom::system::executeCommand(
        command,
        false,  // No need for a shell
        [&](const std::string &line) { output += line + "\n"; });

    // Clean up temporary file
    if (!atom::io::removeFile("temp_code.cpp")) {
        LOG_F(ERROR, "Failed to remove temp_code.cpp");
        return false;
    }

    // Check the output for errors
    if (!output.empty()) {
        LOG_F(ERROR, "Syntax check failed:\n{}", output);
        return false;
    }
    return true;
}

auto CompilerImpl::compileCode(std::string_view code, std::string_view compiler,
                               std::string_view compileOptions,
                               const fs::path &output) -> bool {
    std::string command = std::format("{} {} -xc++ - -o {}", compiler,
                                      compileOptions, output.string());
    std::string compilationOutput;
    compilationOutput = atom::system::executeCommandWithInput(
        command, std::string(code),
        [&](const std::string &line) { compilationOutput += line + "\n"; });
    if (!compilationOutput.empty()) {
        LOG_F(ERROR, "Compilation failed:\n{}", compilationOutput);
        return false;
    }
    return true;
}

auto CompilerImpl::findAvailableCompilers() -> std::vector<std::string> {
    return toolchainManager_.getAvailableCompilers();
}

void CompilerImpl::addCompileOptions(const std::string &options) {
    customCompileOptions_ = options;
}

auto CompilerImpl::getAvailableCompilers() const -> std::vector<std::string> {
    return toolchainManager_.getAvailableCompilers();
}
}  // namespace lithium
