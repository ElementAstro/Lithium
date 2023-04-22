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

#include <fstream>
#include <cstdlib>
#include <array>
#include <unordered_map>

#ifdef _WIN32
#include <Windows.h>
#else
#include <cstdio>
#include <thread>
#endif

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace OpenAPT
{
    ScriptManager::ScriptManager(const std::string &scriptPath)
    {
    std::string path = std::filesystem::path(scriptPath).parent_path().string();
    m_path = std::filesystem::absolute(path);
    m_files = getScriptFiles();
    m_scriptsJson = getScriptsJson(m_files);
    }

    std::vector<std::string> ScriptManager::getScriptFiles() const
    {
        std::vector<std::string> files;
        for (const auto &entry : fs::recursive_directory_iterator(m_path))
        {
            if (entry.is_regular_file() && (entry.path().extension() == ".sh" || entry.path().extension() == ".ps1"))
            {
                std::string filePath = entry.path().string();
                std::string relativePath = std::filesystem::relative(filePath, m_path).generic_string();
                files.push_back(relativePath);
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
            // 检查 Shell 脚本是否以正确的 shebang 开头（#!/bin/bash 或 #!/bin/sh）
            if (script.substr(0, 2) != "#!")
            {
                spdlog::error("Invalid script: missing shebang");
                return false;
            }
            if (script.find("#!/bin/bash") == std::string::npos && script.find("#!/bin/sh") == std::string::npos)
            {
                spdlog::error("Unsupported script: wrong shebang");
                return false;
            }
            return true;
        case ScriptType::Ps:
            // 检查 PowerShell 脚本代码是否符合语法规范
            // 使用 PowerShell 的系统命令 "$ErrorActionPreference = 'Stop'; $null = & {" + scriptContent + "}"
            // 如果脚本有语法错误，则会抛出异常，因此使用 try-catch 捕获异常来判断脚本是否有效
            try
            {
                std::string command = "powershell.exe -Command \"$ErrorActionPreference = 'Stop'; $null = & {" + script + "}\"";
                executeCommand(command);
            }
            catch (const std::runtime_error &e)
            {
                spdlog::error("Invalid script: {}", e.what());
                return false;
            }
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
        if (!m_scriptsJson.contains(scriptName))
        {
            spdlog::error("Script \"{}\" not found", scriptName);
            return false;
        }

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
            std::thread t([=](){
                int ret = std::system(command.c_str());
                if (ret != 0)
                {
                    spdlog::error("Error: command failed ({})", ret);
                    return false;
                }
                return true;
            });
            t.detach();
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

    ScriptType ScriptManager::getScriptType(const std::string &path) const
    {
        static const std::unordered_map<std::string, ScriptType> suffixMap = {{"sh", ScriptType::Sh}, {"ps1", ScriptType::Ps}};
        std::string extension = std::filesystem::path(path).extension().string().substr(1);
        auto it = suffixMap.find(extension);
        if (it != suffixMap.end())
        {
            return it->second;
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
