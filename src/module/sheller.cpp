/*
 * sheller.cpp
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

Date: 2023-4-9

Description: Shell Manager

**************************************************/

#include "sheller.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <sstream>
#include <cstdlib>
#include <array>

#include <spdlog/spdlog.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <cstdio>
#endif

namespace fs = std::filesystem;
using json = nlohmann::json;

#ifndef _WIN32 // 如果是 Linux 系统
FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
#endif

namespace OpenAPT
{
    ScriptManager::ScriptManager(const std::string &path) : m_path(path), m_files(getScriptFiles()), m_scriptsJson(getScriptsJson(m_files)) {}

    std::vector<std::string> ScriptManager::getScriptFiles() const
    {
        std::vector<std::string> files;
        for (const auto &entry : fs::recursive_directory_iterator(m_path))
        {
            if (entry.is_regular_file() && (entry.path().extension() == ".sh" || entry.path().extension() == ".ps1"))
            {
                files.push_back(entry.path().string());
            }
        }
        return files;
    }

    std::string ScriptManager::readScriptFromFile(const std::string &path) const
    {
        std::ifstream input(path);
        std::string contents((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
        return contents;
    }

    bool ScriptManager::validateScript(const std::string &script, ScriptType scriptType) const
    {
        switch (scriptType)
        {
        case ScriptType::Sh:
            // 在这里实现 Shell 脚本校验逻辑
            return true;
        case ScriptType::Ps:
            // 在这里实现 PowerShell 脚本校验逻辑
            return true;
        default:
            spdlog::error("Unsupported script type");
            return false;
        }
    }

    json ScriptManager::getScriptsJson(const std::vector<std::string> &files) const
    {
        json j;
        for (const auto &file : files)
        {
            std::string name = fs::path(file).stem();
            j[name]["path"] = file;
        }
        return j;
    }

    bool ScriptManager::runScript(const std::string &scriptName, bool async) const
    {
        if (m_scriptsJson.contains(scriptName))
        {
            std::string scriptPath = m_scriptsJson[scriptName]["path"];
            spdlog::debug("Found script \"{}\" at \"{}\"", scriptName, scriptPath);

            std::string scriptContent = readScriptFromFile(scriptPath);
            ScriptType scriptType = getScriptType(scriptPath);

            if (!validateScript(scriptContent, scriptType))
            {
                spdlog::error("Script \"{}\" is invalid", scriptName);
                return false;
            }

            std::string command = buildCommand(scriptPath);
            spdlog::debug("Executing command \"{}\"", command);

            if (async)
            {
#ifdef _WIN32
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory(&si, sizeof(si));
                si.cb = sizeof(si);
                ZeroMemory(&pi, sizeof(pi));
                if (!CreateProcess(NULL, (LPSTR)command.c_str(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi))
                {
                    spdlog::error("Error: CreateProcess failed ({})", GetLastError());
                    return false;
                }
#else
                int ret = std::system(command.c_str());
                if (ret != 0)
                {
                    spdlog::error("Error: command failed ({})", ret);
                    return false;
                }
#endif
            }
            else
            {
#ifdef _WIN32
                std::string output = executeCommand(command);
                spdlog::debug("Script \"{}\" output: \n{}", scriptName, output);
#else
                // 在 Linux 中直接调用 shell 脚本的内容
                spdlog::debug("Script \"{}\" output: ", scriptName);
                int ret = std::system(scriptContent.c_str());
                if (ret != 0)
                {
                    spdlog::error("Error: command failed ({})", ret);
                    return false;
                }
#endif
            }

            return true;
        }
        else
        {
            spdlog::error("Script \"{}\" not found", scriptName);
            return false;
        }
    }

    ScriptType ScriptManager::getScriptType(const std::string &path) const
    {
        std::string extension = fs::path(path).extension();
        if (extension == ".sh")
        {
            return ScriptType::Sh;
        }
        else if (extension == ".ps1")
        {
            return ScriptType::Ps;
        }
        else
        {
            spdlog::error("Unsupported script type");
            return ScriptType::Sh;
        }
    }

    std::string ScriptManager::buildCommand(const std::string &scriptPath) const
    {
        std::stringstream ss;

// 根据操作系统不同构建执行脚本的命令
#ifdef _WIN32
        ss << "powershell.exe -ExecutionPolicy Bypass -File \"" << scriptPath << "\"";
#else
        ss << "sh \"" << scriptPath << "\"";
#endif

        return ss.str();
    }

#ifdef _WIN32 // 如果是 Windows 系统
    std::string ScriptManager::executeCommand(const std::string &command) const
    {
        std::array<char, 128> buffer;
        std::string result;

        FILE *pipe = _popen(command.c_str(), "r");
        if (!pipe)
        {
            spdlog::error("Error: _popen failed");
            return "Error";
        }
        while (fgets(buffer.data(), buffer.size(), pipe) != NULL)
        {
            result += buffer.data();
        }
        _pclose(pipe);

        return result;
    }
#else // 否则为 Linux 系统
    std::string ScriptManager::executeCommand(const std::string &command) const
    {
        std::array<char, 128> buffer;
        std::string result;

        FILE *pipe = popen(command.c_str(), "r");
        if (!pipe)
        {
            spdlog::error("Error: popen failed");
            return "Error";
        }
        while (fgets(buffer.data(), buffer.size(), pipe) != NULL)
        {
            result += buffer.data();
        }
        pclose(pipe);

        return result;
    }
#endif
}
