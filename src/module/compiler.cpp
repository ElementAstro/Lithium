#include "compiler.hpp"
#include "openapt.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#ifdef _MSC_VER
#define COMPILER "cl.exe"
#define CMD_PREFIX ""
#define CMD_SUFFIX ".dll"
#else
#define COMPILER "g++"
#define CMD_PREFIX "lib"
#define CMD_SUFFIX ".so"
#endif

bool Compiler::CompileToSharedLibrary(const std::string& code, const std::string& moduleName, const std::string& functionName)
{
    spdlog::info("Compiling module {}::{}...", moduleName, functionName);

    // 参数校验
    if (code.empty() || moduleName.empty() || functionName.empty())
    {
        spdlog::error("Invalid parameters.");
        return false;
    }

    // Check if the module is already compiled and cached
    auto cachedResult = cache_.find(moduleName + "::" + functionName);
    if (cachedResult != cache_.end())
    {
        spdlog::info("Module {}::{} is already compiled, returning cached result.", moduleName, functionName);
        return true;
    }

    // Create output directory if it does not exist
    std::string outputDir = "modules/global/";
    if (!std::filesystem::exists(outputDir))
    {
        spdlog::info("Output directory does not exist, creating it: {}", outputDir);
        if (!std::filesystem::create_directories(outputDir))
        {
            spdlog::error("Failed to create output directory.");
            return false;
        }
    }

    // Read compile options from JSON file
    std::string compileOptions = "-shared -fPIC -x c++ ";
    std::ifstream compileOptionFile("compile_options.json");
    if (compileOptionFile.is_open())
    {
        json compileOptionsJson;
        try
        {
            compileOptionFile >> compileOptionsJson;
            if (compileOptionsJson.contains("optimization_level") && compileOptionsJson.contains("cplus_version") && compileOptionsJson.contains("warnings"))
            {
                compileOptions = compileOptionsJson["optimization_level"].get<std::string>() + " " +
                                  compileOptionsJson["cplus_version"].get<std::string>() + " " +
                                  compileOptionsJson["warnings"].get<std::string>() + " ";
            }
            else
            {
                spdlog::error("Invalid format in compile_options.json.");
                return false;
            }
        }
        catch (const std::exception& e)
        {
            spdlog::error("Error reading compile_options.json: {}", e.what());
            return false;
        }
    }

    // Specify output file path and compiler command
    std::string output = outputDir + moduleName + ".so";
    std::string cmd = std::string(COMPILER) + " " + compileOptions + " - " + " -o " + output;
    spdlog::debug("{}",cmd);

    // Syntax and semantic checking
    std::istringstream codeStream(code);
    std::istringstream ccodeStream(code);
    std::ostringstream outputStream;
    
    if (RunShellCommand(std::string(COMPILER) + " -fsyntax-only -x c++ -", ccodeStream, outputStream) != 0)
    {
        spdlog::error("Syntax error in C++ code: {}", outputStream.str());
        return false;
    }

    int exitCode = RunShellCommand(cmd, codeStream, outputStream);
    if (exitCode != 0)
    {
        spdlog::error("Failed to compile C++ code: {}", outputStream.str());
        return false;
    }

    cache_[moduleName + "::" + functionName] = output;

    // Load the compiled module
    m_ModuleLoader.LoadModule(output, moduleName);

    spdlog::info("Module {}::{} compiled successfully.", moduleName, functionName);

    return true;
}

bool Compiler::CopyFile(const std::string& source, const std::string& destination)
{
    std::ifstream src(source, std::ios::binary);
    if (!src)
    {
        spdlog::error("Failed to open file for copy: {}", source);
        return false;
    }

    std::ofstream dst(destination, std::ios::binary);
    if (!dst)
    {
        spdlog::error("Failed to create file for copy: {}", destination);
        return false;
    }

    dst << src.rdbuf();
    return true;
}

int Compiler::RunShellCommand(const std::string &command, std::istream &inputStream, std::ostream &outputStream)
{
    int exitCode = -1;

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    HANDLE hStdinRead, hStdoutWrite;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0))
    {
        spdlog::error("Failed to create input pipe for shell command: {}", command);
        return exitCode;
    }
    if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
    {
        spdlog::error("Failed to create output pipe for shell command: {}", command);
        return exitCode;
    }
    if (!SetHandleInformation(hStdinWrite, HANDLE_FLAG_INHERIT, 0))
    {
        spdlog::error("Failed to set input handle information for shell command: {}", command);
        return exitCode;
    }
    if (!SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0))
    {
        spdlog::error("Failed to set output handle information for shell command: {}", command);
        return exitCode;
    }
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = hStdinRead;
    si.hStdOutput = hStdoutWrite;
    si.hStdError = hStdoutWrite;
    if (!CreateProcess(NULL, &command[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        spdlog::error("Failed to launch shell command: {}", command);
        CloseHandle(hStdinRead);
        CloseHandle(hStdinWrite);
        CloseHandle(hStdoutRead);
        CloseHandle(hStdoutWrite);
        return exitCode;
    }
    CloseHandle(hStdinRead);
    CloseHandle(hStdoutWrite);

    std::thread inputThread([&inputStream, hStdinWrite]() {
        std::string line;
        while (std::getline(inputStream, line))
        {
            line += "\n";
            DWORD written = 0;
            WriteFile(hStdinWrite, line.c_str(), line.size(), &written, NULL);
        }
        CloseHandle(hStdinWrite);
    });

    std::array<char, 8192> buffer;
    DWORD bytesRead = 0;

    while (true)
    {
        if (!ReadFile(hStdoutRead, buffer.data(), buffer.size(), &bytesRead, NULL) || bytesRead == 0)
        {
            break;
        }
        outputStream.write(buffer.data(), bytesRead);
    }

    DWORD exitCodeNative = 0;
    GetExitCodeProcess(pi.hProcess, &exitCodeNative);

    inputThread.join();

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    exitCode = static_cast<int>(exitCodeNative);
#else
    std::array<char, 8192> buffer;

    FILE *pipe = popen(command.c_str(), "w");
    if (pipe == nullptr)
    {
        spdlog::error("Failed to launch shell command: {}", command);
        return exitCode;
    }

    while (inputStream.good() && !inputStream.eof())
    {
        inputStream.read(buffer.data(), buffer.size());
        std::size_t bytesRead = inputStream.gcount();
        fwrite(buffer.data(), sizeof(char), bytesRead, pipe);
    }

    fflush(pipe);

    int fdout = fileno(pipe);

    while (true)
    {
        ssize_t bytesRead = read(fdout, buffer.data(), buffer.size());
        if (bytesRead == -1 || bytesRead == 0)
        {
            break;
        }
        outputStream.write(buffer.data(), bytesRead);
    }

    exitCode = pclose(pipe);
#endif

    return exitCode;
}