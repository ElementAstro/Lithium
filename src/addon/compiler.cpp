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

#include <atomic>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
#define COMPILER "cl.exe"
#define CMD_PREFIX ""
#define CMD_SUFFIX ".dll"
#elif __APPLE__
#include <cstdio>
#define COMPILER "clang++"
#define CMD_PREFIX "lib"
#define CMD_SUFFIX ".dylib"
#else
#include <cstdio>
#define COMPILER "g++"
#define CMD_PREFIX "lib"
#define CMD_SUFFIX ".so"
#endif

namespace Lithium {
bool Compiler::CompileToSharedLibraryAllinOne(const std::string &code,
                                              const std::string &moduleName,
                                              const std::string &functionName) {
    DLOG_F(INFO, "Compiling module {}::{}...", moduleName, functionName);

    // 参数校验
    if (code.empty() || moduleName.empty() || functionName.empty()) {
        LOG_F(ERROR, "Invalid parameters.");
        return false;
    }

    // Check if the module is already compiled and cached
    auto cachedResult = cache_.find(moduleName + "::" + functionName);
    if (cachedResult != cache_.end()) {
        DLOG_F(WARNING,
               "Module {}::{} is already compiled, returning cached result.",
               moduleName, functionName);
        return true;
    }

    // Create output directory if it does not exist
    const std::string outputDir = "atom/global/";
    if (!fs::exists(outputDir)) {
        DLOG_F(WARNING, "Output directory does not exist, creating it: {}",
               outputDir);
        try {
            fs::create_directories(outputDir);
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Failed to create output directory: {}", e.what());
            return false;
        }
    }

    // Read compile options from JSON file
    std::string compileOptions = "-shared -fPIC -x c++ ";
    std::ifstream compileOptionFile("compile_options.json");
    if (compileOptionFile.is_open()) {
        json compileOptionsJson;
        try {
            compileOptionFile >> compileOptionsJson;
            if (compileOptionsJson.contains("optimization_level") &&
                compileOptionsJson.contains("cplus_version") &&
                compileOptionsJson.contains("warnings")) {
                compileOptions =
                    compileOptionsJson["optimization_level"]
                        .get<std::string>() +
                    " " +
                    compileOptionsJson["cplus_version"].get<std::string>() +
                    " " + compileOptionsJson["warnings"].get<std::string>() +
                    " ";
            } else {
                LOG_F(ERROR, "Invalid format in compile_options.json.");
                return false;
            }
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Error reading compile_options.json: {}", e.what());
            return false;
        }
    }

    // Specify output file path
    std::string output = outputDir + moduleName + CMD_SUFFIX;

    // Syntax and semantic checking
    std::stringstream syntaxCheckCmd;
    syntaxCheckCmd << COMPILER << " -fsyntax-only -x c++ -";
    std::string syntaxCheckOutput;
    if (RunShellCommand(syntaxCheckCmd.str(), code, syntaxCheckOutput) != 0) {
        LOG_F(ERROR, "Syntax error in C++ code: {}", syntaxCheckOutput);
        return false;
    }

    // Compile code
    std::string compilationOutput;
    std::string cmd =
        std::string(COMPILER) + " " + compileOptions + " - " + " -o " + output;
    DLOG_F(INFO, "{}", cmd);

    int exitCode = RunShellCommand(cmd, code, compilationOutput);
    if (exitCode != 0) {
        LOG_F(ERROR, "Failed to compile C++ code: {}", compilationOutput);
        return false;
    }

    // Cache compiled module
    cache_[moduleName + "::" + functionName] = output;

    /*
    // Load the compiled module
    if(m_App.GetModuleLoader()->LoadModule(output, moduleName)) {
        LOG_S(INFO) << "Module " << moduleName << "::" << functionName << "
    compiled successfully."; return true; } else { LOG_F(ERROR, "Failed to load
    the compiled module: {}", output); return false;
    }
    */
    return false;
}

// 编译为共享库
bool Compiler::CompileToSharedLibrary(const std::string &code,
                                      const std::string &moduleName,
                                      const std::string &functionName,
                                      const std::string &optionsFile) {
    LOG_F(INFO, "Compiling module {}::{}...", moduleName, functionName);

    // 参数校验
    if (!CheckParameters(code, moduleName, functionName)) {
        return false;
    }

    // 检查是否已经编译并缓存
    if (IsModuleCached(moduleName, functionName, cache_)) {
        return true;
    }

    // 创建输出目录
    const std::string outputDir = "atom/global/";
    if (!CreateOutputDirectory(outputDir)) {
        return false;
    }

    // 读取编译选项
    std::string compileOptions = ReadCompileOptions(optionsFile);
    if (compileOptions.empty()) {
        return false;
    }

    // 检查语法
    if (!SyntaxCheck(code, COMPILER)) {
        return false;
    }

    // 指定输出文件路径
    std::string output = outputDir + moduleName + CMD_SUFFIX;

    // 编译代码
    if (!CompileCode(code, COMPILER, compileOptions, output)) {
        return false;
    }

    // 缓存已编译的模块
    CacheCompiledModule(moduleName, functionName, output, cache_);

    return true;
}

// 检查参数是否有效
bool Compiler::CheckParameters(const std::string &code,
                               const std::string &moduleName,
                               const std::string &functionName) {
    if (code.empty() || moduleName.empty() || functionName.empty()) {
        LOG_F(ERROR, "Invalid parameters.");
        return false;
    }
    return true;
}

// 检查模块是否已经编译并缓存
bool Compiler::IsModuleCached(
    const std::string &moduleName, const std::string &functionName,
    std::unordered_map<std::string, std::string> &cache_) {
    std::string key = moduleName + "::" + functionName;
    auto cachedResult = cache_.find(key);
    if (cachedResult != cache_.end()) {
        LOG_F(WARNING, "Module {}::{} is already compiled.", moduleName,
              functionName);
        return true;
    }
    return false;
}

// 创建输出目录
bool Compiler::CreateOutputDirectory(const std::string &outputDir) {
    if (!fs::exists(outputDir)) {
        LOG_F(WARNING, "Output directory does not exist.");
        try {
            fs::create_directories(outputDir);
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Failed to create output directory. {}", e.what());
            return false;
        }
    }
    return true;
}

// 从 JSON 文件中读取编译选项
std::string Compiler::ReadCompileOptions(const std::string &optionsFile) {
    std::ifstream compileOptionFile(optionsFile);
    if (compileOptionFile.is_open()) {
        json compileOptionsJson;
        try {
            compileOptionFile >> compileOptionsJson;
            if (compileOptionsJson.contains("optimization_level") &&
                compileOptionsJson.contains("cplus_version") &&
                compileOptionsJson.contains("warnings")) {
                std::string compileOptions =
                    compileOptionsJson["optimization_level"]
                        .get<std::string>() +
                    " " +
                    compileOptionsJson["cplus_version"].get<std::string>() +
                    " " + compileOptionsJson["warnings"].get<std::string>();
                return compileOptions;
            } else {
                LOG_F(ERROR, "Invalid format in compile_options.json.");
                return "";
            }
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Error reading compile_options.json: {}", e.what());
            return "";
        }
    }
    return "";
}

// 语法检查
bool Compiler::SyntaxCheck(const std::string &code,
                           const std::string &compiler) {
    std::stringstream syntaxCheckCmd;
    syntaxCheckCmd << compiler << " -fsyntax-only -x c++ -";
    std::string syntaxCheckOutput;
    if (RunShellCommand(syntaxCheckCmd.str(), code, syntaxCheckOutput) != 0) {
        LOG_F(ERROR, "Syntax error in C++ code: {}", syntaxCheckOutput);
        return false;
    }
    return true;
}

// 编译代码
bool Compiler::CompileCode(const std::string &code, const std::string &compiler,
                           const std::string &compileOptions,
                           const std::string &output) {
    std::string cmd = compiler + " " + compileOptions + " - " + " -o " + output;
    DLOG_F(INFO, "{}", cmd);

    std::string compilationOutput;
    int exitCode = RunShellCommand(cmd, code, compilationOutput);
    if (exitCode != 0) {
        LOG_F(ERROR, "Failed to compile C++ code: {}", compilationOutput);
        return false;
    }
    return true;
}

// 缓存已编译的模块
void Compiler::CacheCompiledModule(
    const std::string &moduleName, const std::string &functionName,
    const std::string &output,
    std::unordered_map<std::string, std::string> &cache_) {
    std::string key = moduleName + "::" + functionName;
    cache_[key] = output;
}

bool Compiler::CopyFile_(const std::string &source,
                         const std::string &destination) {
    try {
        std::ifstream src(source, std::ios::binary);
        if (!src) {
            LOG_F(ERROR, "Failed to open file for copy: {}", source);
            return false;
        }

        std::ofstream dst(destination, std::ios::binary);
        if (!dst) {
            LOG_F(ERROR, "Failed to create file for copy: {}", destination);
            return false;
        }

        dst << src.rdbuf();

        if (!dst.good()) {
            LOG_F(ERROR, "Error occurred while writing to destination file: {}",
                  destination);
            return false;
        }

        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Exception occurred during file copy: {}", e.what());
        return false;
    }
}

int Compiler::RunShellCommand(const std::string &command,
                              const std::string &input, std::string &output) {
    int exitCode = -1;
#ifdef _WIN32
    HANDLE hStdoutRead = NULL;

    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    HANDLE hStdinRead = NULL, hStdoutWrite = NULL;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&hStdinRead, &hStdoutWrite, &sa, 0)) {
        LOG_F(ERROR, "Failed to create input pipe for shell command: {}",
              command);
        return exitCode;
    }
    if (!SetHandleInformation(hStdoutWrite, HANDLE_FLAG_INHERIT, 0)) {
        LOG_F(ERROR,
              "Failed to set input handle information for shell command: {}",
              command);
        CloseHandle(hStdinRead);
        return exitCode;
    }
    if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0)) {
        LOG_F(ERROR, "Failed to create output pipe for shell command: {}",
              command);
        CloseHandle(hStdinRead);
        CloseHandle(hStdoutWrite);
        return exitCode;
    }

    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdInput = hStdinRead;
    si.hStdOutput = hStdoutWrite;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    std::vector<char> commandBuffer(command.begin(), command.end());
    commandBuffer.push_back('\0');

    if (!CreateProcess(NULL, &commandBuffer[0], NULL, NULL, TRUE, 0, NULL, NULL,
                       &si, &pi)) {
        LOG_F(ERROR, "Failed to launch shell command: {}", command);
        CloseHandle(hStdinRead);
        CloseHandle(hStdoutWrite);
        CloseHandle(hStdoutRead);
        return exitCode;
    }
    CloseHandle(hStdinRead);
    CloseHandle(hStdoutWrite);

    // Read the command output
    char buffer[4096];
    DWORD bytesRead;
    while (ReadFile(hStdoutRead, buffer, sizeof(buffer), &bytesRead, NULL)) {
        if (bytesRead > 0) {
            output.append(buffer, bytesRead);
        } else {
            break;
        }
    }

    // Wait for the command to finish
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, (LPDWORD)&exitCode);

    // Clean up
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hStdoutRead);
#else
    FILE *pipe = popen(command.c_str(), "w");
    if (!pipe) {
        LOG_F(ERROR, "Failed to popen shell command: {}", command);
        return exitCode;
    }

    fwrite(input.c_str(), 1, input.size(), pipe);
    fclose(pipe);

    exitCode = WEXITSTATUS(pclose(pipe));
#endif

    return exitCode;
}

}  // namespace Lithium
