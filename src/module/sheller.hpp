/*
 * sheller.hpp
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

#pragma once

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <filesystem>

class ScriptManager
{
public:
    void AddScript(const std::string &scriptName, const std::string &scriptPath, int scriptType,
                   const std::string &arguments, const std::string &argumentTypes);
    void RemoveScript(const std::string &scriptName);
    std::vector<std::string> GetScriptNames() const;
    std::map<std::string, std::string> GetScript(const std::string &scriptName, bool getContent = false) const;
    std::map<std::string, std::map<std::string, std::string>> GetAllScripts() const;
    void SaveScriptsInfoToFile(const std::string &filePath) const;
    std::string RunScript(const std::string &scriptName, const std::vector<std::string> &scriptArgs,
                          bool block = true);
    void StopScript(const std::string &scriptName);
    std::string GetScriptOutput(const std::string &scriptName) const;

private:
    struct ScriptInfo
    {
        std::string path;
        int type;
        std::string arguments;
        std::string argumentTypes;
        std::string output;
        bool isRunning;
        mutable std::condition_variable cv;

        ScriptInfo &operator=(const ScriptInfo &) = default;
        ScriptInfo &operator=(ScriptInfo &&other) noexcept
        {
            if (this != &other)
            {
                path = std::move(other.path);
                type = other.type;
                arguments = std::move(other.arguments);
                argumentTypes = std::move(other.argumentTypes);
                isRunning = other.isRunning;
                output = std::move(other.output);
            }
            return *this;
        }
    };

    bool IsScriptRunning(const std::string &scriptName) const;
    std::string GetScriptFilePath(const std::string &scriptName) const;
    std::string BuildCommand(const std::string &scriptPath, const std::vector<std::string> &scriptArgs) const;
    std::string ExecuteCommand(const std::string &command) const;
    std::string ReadScriptFromFile(const std::string &scriptPath) const;
    void RemoveFromRunningScripts(const std::string &scriptName);
    std::map<std::string, ScriptInfo> m_scriptInfoMap;
    mutable std::mutex m_scriptInfoMutex;
};
