#include "compiler.hpp"
#include "utils/constant.hpp"

#include <fstream>
#include <functional>
#include <unordered_map>
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/to_string.hpp"

using json = nlohmann::json;

namespace lithium {

class CompilerImpl {
public:
    auto compileToSharedLibrary(std::string_view code,
                                std::string_view moduleName,
                                std::string_view functionName,
                                std::string_view optionsFile) -> bool;

    void addCompileOptions(const std::string& options);

    auto getAvailableCompilers() const -> std::vector<std::string>;

private:
    void createOutputDirectory(const std::filesystem::path& outputDir);
    auto syntaxCheck(std::string_view code, std::string_view compiler) -> bool;
    auto compileCode(std::string_view code, std::string_view compiler,
                     std::string_view compileOptions,
                     const std::filesystem::path& output) -> bool;

    auto findAvailableCompilers() -> std::vector<std::string>;

    std::unordered_map<std::string, std::filesystem::path> cache_;
    std::string customCompileOptions_;
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

void Compiler::addCompileOptions(const std::string& options) {
    impl_->addCompileOptions(options);
}

auto Compiler::getAvailableCompilers() const -> std::vector<std::string> {
    return impl_->getAvailableCompilers();
}

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
    const std::filesystem::path outputDir = "atom/global";
    createOutputDirectory(outputDir);

    auto availableCompilers = findAvailableCompilers();
    if (availableCompilers.empty()) {
        LOG_F(ERROR, "No available compilers found.");
        return false;
    }
    LOG_F(INFO, "Available compilers: {}",
          atom::utils::toString(availableCompilers));

    // 读取编译选项
    std::ifstream optionsStream(optionsFile.data());
    std::string compileOptions = [&optionsStream, this] {
        if (!optionsStream) {
            LOG_F(
                WARNING,
                "Failed to open compile options file, using default options.");
            return std::string{"-O2 -std=c++20 -Wall -shared -fPIC"};
        }

        try {
            json optionsJson;
            optionsStream >> optionsJson;
            return std::format(
                "{} {} {} {}",
                optionsJson["optimization_level"].get<std::string>(),
                optionsJson["cplus_version"].get<std::string>(),
                optionsJson["warnings"].get<std::string>(),
                customCompileOptions_);
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to parse compile options file: {}", e.what());
            return std::string{"-O2 -std=c++20 -Wall -shared -fPIC"};
        }
    }();

    // 语法检查
    if (!syntaxCheck(code, constants::COMPILER)) {
        return false;
    }

    // 编译代码
    std::filesystem::path outputPath =
        outputDir / std::format("{}{}{}", constants::LIB_EXTENSION, moduleName,
                                constants::LIB_EXTENSION);
    if (!compileCode(code, constants::COMPILER, compileOptions, outputPath)) {
        return false;
    }

    // 缓存编译结果
    cache_[cacheKey] = outputPath;
    return true;
}

void CompilerImpl::createOutputDirectory(
    const std::filesystem::path& outputDir) {
    if (!std::filesystem::exists(outputDir)) {
        LOG_F(WARNING, "Output directory {} does not exist, creating it.",
              outputDir.string());
        std::filesystem::create_directories(outputDir);
    }
}

auto CompilerImpl::syntaxCheck(std::string_view code,
                               std::string_view compiler) -> bool {
    std::string command = std::format("{} -fsyntax-only -xc++ -", compiler);
    std::string output;
    output = atom::system::executeCommand(
        command, false,
        [&](const std::string& line) { output += line + "\n"; });
    if (!output.empty()) {
        LOG_F(ERROR, "Syntax check failed:\n{}", output);
        return false;
    }
    return true;
}

auto CompilerImpl::compileCode(std::string_view code, std::string_view compiler,
                               std::string_view compileOptions,
                               const std::filesystem::path& output) -> bool {
    std::string command = std::format("{} {} -xc++ - -o {}", compiler,
                                      compileOptions, output.string());
    std::string compilationOutput;
    compilationOutput = atom::system::executeCommand(
        command, false,
        [&](const std::string& line) { compilationOutput += line + "\n"; });
    if (!compilationOutput.empty()) {
        LOG_F(ERROR, "Compilation failed:\n{}", compilationOutput);
        return false;
    }
    return true;
}

auto CompilerImpl::findAvailableCompilers() -> std::vector<std::string> {
    std::vector<std::string> availableCompilers;

    for (const auto& path : constants::COMPILER_PATHS) {
        for (const auto& compiler : constants::COMMON_COMPILERS) {
            std::filesystem::path compilerPath =
                std::filesystem::path(path) / compiler;
            if (std::filesystem::exists(compilerPath)) {
                availableCompilers.push_back(compilerPath.string());
            }
        }
    }

    return availableCompilers;
}

void CompilerImpl::addCompileOptions(const std::string& options) {
    customCompileOptions_ = options;
}

auto CompilerImpl::getAvailableCompilers() const -> std::vector<std::string> {
    std::vector<std::string> availableCompilers;
    availableCompilers.reserve(cache_.size());
    for (const auto& [key, value] : cache_) {
        availableCompilers.push_back(key);
    }
    return availableCompilers;
}

}  // namespace lithium
