/*
 * compiler.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-29

Description: Compiler

**************************************************/

#include "compiler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <atomic>

#include "loguru/loguru.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

#ifdef _WIN32
#include <windows.h>
#define COMPILER "cl.exe"
#define CMD_PREFIX ""
#define CMD_SUFFIX ".dll"
#else
#include <cstdio>
#define COMPILER "g++"
#define CMD_PREFIX "lib"
#define CMD_SUFFIX ".so"
#endif

bool Compiler::CompileToSharedLibrary(const std::string &code, const std::string &moduleName, const std::string &functionName)
{
    LOG_F(ERROR, "Compiling module %s::%s...", moduleName.c_str(), functionName.c_str());

    // 参数校验
    if (code.empty() || moduleName.empty() || functionName.empty())
    {
        LOG_F(ERROR, "Invalid parameters.");
        return false;
    }

    // Check if the module is already compiled and cached
    auto cachedResult = cache_.find(moduleName + "::" + functionName);
    if (cachedResult != cache_.end())
    {
        LOG_F(WARNING, "Module %s::%s is already compiled, returning cached result.", moduleName.c_str(), functionName.c_str());
        return true;
    }

    // Create output directory if it does not exist
    const std::string outputDir = "modules/global/";
    if (!fs::exists(outputDir))
    {
        LOG_F(WARNING, "Output directory does not exist, creating it: %s", outputDir.c_str());
        try
        {
            fs::create_directories(outputDir);
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to create output directory: %s", e.what());
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
                LOG_F(ERROR, "Invalid format in compile_options.json.");
                return false;
            }
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Error reading compile_options.json: %s", e.what());
            return false;
        }
    }

    // Specify output file path
    std::string output = outputDir + moduleName + CMD_SUFFIX;

    // Syntax and semantic checking
    std::stringstream syntaxCheckCmd;
    syntaxCheckCmd << COMPILER << " -fsyntax-only -x c++ -";
    std::ostringstream syntaxCheckOutput;
    if (RunShellCommand(syntaxCheckCmd.str(), code, syntaxCheckOutput) != 0)
    {
        LOG_F(ERROR, "Syntax error in C++ code: %s", syntaxCheckOutput.str().c_str());
        return false;
    }

    // Compile code
    std::ostringstream compilationOutput;
    std::string cmd = std::string(COMPILER) + " " + compileOptions + " - " + " -o " + output;
    LOG_F(DEBUG, "%s", cmd.c_str());

    int exitCode = RunShellCommand(cmd, code, compilationOutput);
    if (exitCode != 0)
    {
        LOG_F(ERROR, "Failed to compile C++ code: %s", compilationOutput.str().c_str());
        return false;
    }

    // Cache compiled module
    cache_[moduleName + "::" + functionName] = output;

    /*
    // Load the compiled module
    if(m_App.GetModuleLoader()->LoadModule(output, moduleName)) {
        LOG_S(INFO) << "Module " << moduleName << "::" << functionName << " compiled successfully.";
        return true;
    } else {
        LOG_F(ERROR, "Failed to load the compiled module: %s", output.c_str());
        return false;
    }
    */
    return false;
}

bool Compiler::CopyFile(const std::string &source, const std::string &destination)
{
    std::ifstream src(source, std::ios::binary);
    if (!src)
    {
        // spdlog::error("Failed to open file for copy: {}", source);
        return false;
    }

    std::ofstream dst(destination, std::ios::binary);
    if (!dst)
    {
        // spdlog::error("Failed to create file for copy: {}", destination);
        return false;
    }

    dst << src.rdbuf();
    return true;
}

int Compiler::RunShellCommand(const std::string &command, const std::string &input, std::string &output)
{
    int exitCode = -1;
#ifdef _WIN32
    HANDLE hStdoutRead;

    STARTUPINFO si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    HANDLE hStdinRead, hStdoutWrite;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;
    if (!CreatePipe(&hStdinRead, &hStdoutWrite, &sa, 0))
    {
        LOG_F(ERROR, "Failed to create input pipe for shell command: %s", command.c_str());
        return exitCode;
    }
    if (!SetHandleInformation(hStdoutWrite, HANDLE_FLAG_INHERIT, 0))
    {
        LOG_F(ERROR, "Failed to set input handle information for shell command: %s", command.c_str());
        return exitCode;
    }
    if (!SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0))
    {
        LOG_F(ERROR, "Failed to set output handle information for shell command: %s", command.c_str());
        return exitCode;
    }
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.wShowWindow = SW_HIDE;
    si.hStdInput = hStdinRead;
    si.hStdOutput = hStdoutWrite;
    si.hStdError = hStdoutWrite;
    if (!CreateProcess(NULL, &command[0], NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
    {
        LOG_F(ERROR, "Failed to launch shell command: %s", command.c_str());
        CloseHandle(hStdinRead);
        CloseHandle(hStdoutWrite);
        CloseHandle(hStdoutRead);
        CloseHandle(hStdoutWrite);
        return exitCode;
    }
    CloseHandle(hStdinRead);
    CloseHandle(hStdoutWrite);

    // Read the command output
    std::thread outputThread([&]()
                             {
        char buffer[4096];
        DWORD bytesRead;
        while (ReadFile(hStdoutRead, buffer, sizeof(buffer), &bytesRead, NULL))
        {
            output.append(buffer, bytesRead);
        } });

    // Write the command input
    DWORD bytesWritten;
    if (!WriteFile(hStdinWrite, input.c_str(), input.size(), &bytesWritten, NULL))
    {
        LOG_F(ERROR, "Failed to write input for shell command: %s", command.c_str());
        return exitCode;
    }
    CloseHandle(hStdinWrite);

    // Wait for the command to finish
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, (LPDWORD)&exitCode);

    // Clean up
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hStdoutRead);
    outputThread.join();

#else
    FILE *pipe = popen(command.c_str(), "w");
    if (!pipe)
    {
        LOG_F(ERROR, "Failed to popen shell command: %s", command.c_str());
        return exitCode;
    }

    fwrite(input.c_str(), 1, input.size(), pipe);
    fclose(pipe);

    exitCode = WEXITSTATUS(pclose(pipe));
#endif

    return exitCode;
}