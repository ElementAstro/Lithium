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
    ScriptManager::ScriptManager(const std::string &scriptPath) : m_path(std::filesystem::absolute(std::filesystem::path(scriptPath).parent_path().string())), m_files(getScriptFiles()), m_scriptsJson(getScriptsJson(m_files)) {}

    std::vector<std::string> ScriptManager::getScriptFiles() const
    {
        std::vector<std::string> files;
        for (const auto &entry : std::filesystem::recursive_directory_iterator(m_path))
        {
            if (entry.is_regular_file() && (entry.path().extension() == ".sh" || entry.path().extension() == ".ps1"))
            {
                files.push_back(std::filesystem::relative(entry.path(), m_path).generic_string());
            }
        }
        return files;
    }

    std::string ScriptManager::readScriptFromFile(const std::string &path) const
    {
        std::ifstream input(path);
        return { std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>() };
    }

    bool ScriptManager::validateScript(const std::string &script, ScriptType scriptType) const
    {
        switch (scriptType)
        {
        case ScriptType::Sh:
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
            j[std::filesystem::path(file).stem()].emplace("path", file);
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

        const auto &scriptPath = m_scriptsJson[scriptName]["path"].get<std::string>();
        spdlog::debug("Found script \"{}\" at \"{}\"", scriptName, scriptPath);

        const auto &scriptContent = readScriptFromFile(scriptPath);
        const auto scriptType = getScriptType(scriptPath);

        if (!validateScript(scriptContent, scriptType))
        {
            spdlog::error("Script \"{}\" is invalid", scriptName);
            return false;
        }

        const auto command = buildCommand(scriptPath);
        spdlog::debug("Executing command \"{}\"", command);

        if (async)
        {
#ifdef _WIN32
            if (!_spawnlp(_P_NOWAIT, "powershell.exe", "powershell.exe", "-ExecutionPolicy", "Bypass", "-File", command.c_str(), nullptr))
            {
                spdlog::error("Error: _spawnlp failed ({})", errno);
                return false;
            }
#else
            if (auto pid = fork(); pid == 0)
            {
                if (execlp("/bin/sh", "sh", "-c", scriptContent.c_str(), nullptr) == -1)
                {
                    spdlog::error("Error: execlp failed ({})", errno);
                    exit(1);
                }
                exit(0);
            }
#endif
        }
        else
        {
#ifdef _WIN32
            const auto output = executeCommand(command);
            spdlog::debug("Script \"{}\" output: \n{}", scriptName, output);
#else
            spdlog::debug("Script \"{}\" output: ", scriptName);
            if (auto ret = std::system(scriptContent.c_str()); ret != 0)
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
        static const std::unordered_map<std::string_view, ScriptType> suffixMap = { { "sh", ScriptType::Sh },{ "ps1", ScriptType::Ps } };
        const auto extension = std::filesystem::path(path).extension().string().substr(1);
        if (auto it = suffixMap.find(extension); it != suffixMap.end())
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

#ifdef _WIN32
        ss << "\"" << scriptPath << "\"";
#else
        ss << "sh \"" << scriptPath << "\"";
#endif

        return ss.str();
    }

#ifdef _WIN32
    std::string ScriptManager::executeCommand(const std::string &command) const
    {
        std::array<char, 128> buffer;
        std::string result;

        FILE *pipe;
        if ((pipe = _popen(command.c_str(), "r")) == nullptr)
        {
            spdlog::error("Error: _popen failed");
            return result;
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
        {
            result += buffer.data();
        }
        _pclose(pipe);

        return result;
    }
#else
    std::string ScriptManager::executeCommand(const std::string &command) const
    {
        std::array<char, 128> buffer;
        std::string result;

        FILE *pipe;
        if ((pipe = popen(command.c_str(), "r")) == nullptr)
        {
            spdlog::error("Error: popen failed");
            return result;
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr)
        {
            result += buffer.data();
        }
        pclose(pipe);

        return result;
    }
#endif
}
