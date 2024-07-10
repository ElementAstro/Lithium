/*
 * compiler.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Compiler

**************************************************/

#include "compiler.hpp"

#include "utils/constant.hpp"

#include <fstream>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/to_string.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

#include <fstream>

namespace lithium {

bool Compiler::compileToSharedLibrary(std::string_view code,
                                      std::string_view moduleName,
                                      std::string_view functionName,
                                      std::string_view optionsFile) {
    LOG_F(INFO, "Compiling module {}::{}...", moduleName, functionName);

    if (code.empty() || moduleName.empty() || functionName.empty()) {
        LOG_F(ERROR, "Invalid parameters.");
        return false;
    }

    // 检查模块是否已编译并缓存
    std::string cacheKey = std::format("{}::{}", moduleName, functionName);
    if (cache_.find(cacheKey) != cache_.end()) {
        LOG_F(WARNING, "Module {} is already compiled, using cached result.", cacheKey);
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
    LOG_F(INFO, "Available compilers: {}", atom::utils::toString(availableCompilers));

    // 读取编译选项
    std::ifstream optionsStream(optionsFile.data());
    std::string compileOptions = [&optionsStream, this] {
        if (!optionsStream) {
            LOG_F(WARNING, "Failed to open compile options file, using default options.");
            return std::string{"-O2 -std=c++20 -Wall -shared -fPIC"};
        }

        try {
            json optionsJson;
            optionsStream >> optionsJson;
            return std::format("{} {} {} {}", 
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
    std::filesystem::path outputPath = outputDir / std::format("{}{}{}", constants::LIB_EXTENSION, moduleName, constants::LIB_EXTENSION);
    if (!compileCode(code, constants::COMPILER, compileOptions, outputPath)) {
        return false;
    }

    // 缓存编译结果
    cache_[cacheKey] = outputPath;
    return true;
}

void Compiler::createOutputDirectory(const std::filesystem::path& outputDir) {
    if (!std::filesystem::exists(outputDir)) {
        LOG_F(WARNING, "Output directory {} does not exist, creating it.", outputDir.string());
        std::filesystem::create_directories(outputDir);
    }
}

bool Compiler::syntaxCheck(std::string_view code, std::string_view compiler) {
    std::string command = std::format("{} -fsyntax-only -xc++ -", compiler);
    std::string output;
    int exitCode = runCommand(command, code, output);
    if (exitCode != 0) {
        LOG_F(ERROR, "Syntax check failed:\n{}", output);
        return false;
    }
    return true;
}

bool Compiler::compileCode(std::string_view code, std::string_view compiler,
                           std::string_view compileOptions, const std::filesystem::path& output) {
    std::string command = std::format("{} {} -xc++ - -o {}", compiler, compileOptions, output.string());
    std::string compilationOutput;
    int exitCode = runCommand(command, code, compilationOutput);
    if (exitCode != 0) {
        LOG_F(ERROR, "Compilation failed:\n{}", compilationOutput);
        return false;
    }
    return true;
}

int Compiler::runCommand(std::string_view command, std::string_view input, std::string& output) {
    std::array<char, 128> buffer;
    output.clear();

    auto pipe = popen(command.data(), "r+");
    if (!pipe) {
        LOG_F(ERROR, "Failed to run command: {}", command);
        return -1;
    }

    fwrite(input.data(), sizeof(char), input.size(), pipe);
    fclose(pipe);

    pipe = popen(command.data(), "r");
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }
    return pclose(pipe);
}

std::vector<std::string> Compiler::findAvailableCompilers() {
    std::vector<std::string> availableCompilers;

    for (const auto& path : constants::COMPILER_PATHS) {
        for (const auto& compiler : constants::COMMON_COMPILERS) {
            std::filesystem::path compilerPath = std::filesystem::path(path) / compiler;
            if (std::filesystem::exists(compilerPath)) {
                availableCompilers.push_back(compilerPath.string());
            }
        }
    }

    return availableCompilers;
}

void Compiler::addCompileOptions(const std::string& options) {
    customCompileOptions_ = options;
}

std::vector<std::string> Compiler::getAvailableCompilers() const {
    std::vector<std::string> availableCompilers;
    for (const auto& [key, value] : cache_) {
        availableCompilers.push_back(key);
    }
    return availableCompilers;
}

}  // namespace lithium
