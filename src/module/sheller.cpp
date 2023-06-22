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
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

void ScriptManager::AddScript(const std::string &scriptName, const std::string &scriptPath, int scriptType,
                              const std::string &arguments, const std::string &argumentTypes)
{
    std::scoped_lock lock(m_scriptInfoMutex);
    ScriptInfo scriptInfo;
    scriptInfo.path = scriptPath;
    scriptInfo.type = scriptType;
    scriptInfo.arguments = arguments;
    scriptInfo.argumentTypes = argumentTypes;
    scriptInfo.isRunning = false;
    m_scriptInfoMap[scriptName] = std::move(scriptInfo);
}

void ScriptManager::RemoveScript(const std::string &scriptName)
{
    std::scoped_lock lock(m_scriptInfoMutex);
    m_scriptInfoMap.erase(scriptName);
}

std::vector<std::string> ScriptManager::GetScriptNames() const
{
    std::scoped_lock lock(m_scriptInfoMutex);
    std::vector<std::string> scriptNames;
    for (const auto &[key, value] : m_scriptInfoMap)
    {
        scriptNames.push_back(key);
    }
    return scriptNames;
}

std::map<std::string, std::string> ScriptManager::GetScript(const std::string &scriptName, bool getContent /*= false*/) const
{
    std::scoped_lock lock(m_scriptInfoMutex);
    if (getContent)
    {
        std::string content = ReadScriptFromFile(GetScriptFilePath(scriptName));
        std::map<std::string, std::string> scriptInfo;
        scriptInfo["name"] = scriptName;
        scriptInfo["content"] = content;
        scriptInfo["arguments"] = m_scriptInfoMap.at(scriptName).arguments;
        scriptInfo["argument_types"] = m_scriptInfoMap.at(scriptName).argumentTypes;
        return scriptInfo;
    }
    else
    {
        if (m_scriptInfoMap.contains(scriptName))
        {
            std::map<std::string, std::string> scriptInfo;
            const auto &info = m_scriptInfoMap.at(scriptName);
            scriptInfo["name"] = scriptName;
            scriptInfo["path"] = info.path;
            scriptInfo["type"] = std::to_string(info.type);
            scriptInfo["output"] = info.output;
            scriptInfo["arguments"] = info.arguments;
            scriptInfo["argument_types"] = info.argumentTypes;
            return scriptInfo;
        }
        else
        {
            return {};
        }
    }
}

std::map<std::string, std::map<std::string, std::string>> ScriptManager::GetAllScripts() const
{
    std::scoped_lock lock(m_scriptInfoMutex);
    std::map<std::string, std::map<std::string, std::string>> allScripts;
    for (const auto &[scriptName, scriptInfo] : m_scriptInfoMap)
    {
        std::map<std::string, std::string> script;
        script["name"] = scriptName;
        script["path"] = scriptInfo.path;
        script["type"] = std::to_string(scriptInfo.type);
        script["output"] = scriptInfo.output;
        script["arguments"] = scriptInfo.arguments;
        script["argument_types"] = scriptInfo.argumentTypes;
        allScripts[scriptName] = script;
    }
    return allScripts;
}

void ScriptManager::SaveScriptsInfoToFile(const std::string &filePath) const
{
    std::scoped_lock lock(m_scriptInfoMutex);
    nlohmann::json j;
    for (const auto &[scriptName, scriptInfo] : m_scriptInfoMap)
    {
        nlohmann::json scriptJson;
        scriptJson["name"] = scriptName;
        scriptJson["path"] = scriptInfo.path;
        scriptJson["type"] = scriptInfo.type;
        scriptJson["arguments"] = scriptInfo.arguments;
        scriptJson["argument_types"] = scriptInfo.argumentTypes;
        j[scriptName] = scriptJson;
    }

    std::ofstream infoFile(filePath);
    if (!infoFile)
    {
        spdlog::error("Can't open script info file for writing: {}", filePath);
        return;
    }
    infoFile << j.dump(4);
}

std::string ScriptManager::RunScript(const std::string &scriptName, const std::vector<std::string> &scriptArgs, bool block /*= true*/)
{
    std::string scriptPath = GetScriptFilePath(scriptName);

    // 检查脚本是否正在运行
    if (IsScriptRunning(scriptName))
    {
        throw std::runtime_error("Script is already running: " + scriptName);
    }

    // 构建脚本命令
    std::string command = BuildCommand(scriptPath, scriptArgs);

    // 执行脚本命令
    std::string output;
    try
    {
        output = ExecuteCommand(command);
    }
    catch (const std::exception &e)
    {
        spdlog::error("Failed to execute script: {}", e.what());
        throw;
    }

    // 更新脚本输出
    std::unique_lock<std::mutex> lock(m_scriptInfoMutex);
    ScriptInfo &info = m_scriptInfoMap.at(scriptName);
    info.output = output;

    return output;
}

void ScriptManager::StopScript(const std::string &scriptName)
{
    if (IsScriptRunning(scriptName))
    {
        RemoveFromRunningScripts(scriptName);
    }
}

std::string ScriptManager::GetScriptOutput(const std::string &scriptName) const
{
    if (!m_scriptInfoMap.contains(scriptName))
    {
        return "";
    }

    const ScriptInfo &info = m_scriptInfoMap.at(scriptName);

    if (info.isRunning)
    {
        /*
        
        */
        // 将 shared_lock 转换为 unique_lock
        std::unique_lock<std::mutex> uniqueLock(m_scriptInfoMutex, std::adopt_lock);
        info.cv.wait(uniqueLock, [&info]()
                     { return !info.isRunning; });
    }

    return info.output;
}

bool ScriptManager::IsScriptRunning(const std ::string &scriptName) const
{
    if (m_scriptInfoMap.contains(scriptName))
    {
        const ScriptInfo &info = m_scriptInfoMap.at(scriptName);
        return info.isRunning;
    }
    return false;
}

std::string ScriptManager::GetScriptFilePath(const std::string &scriptName) const
{
    if (m_scriptInfoMap.contains(scriptName))
    {
        const ScriptInfo &info = m_scriptInfoMap.at(scriptName);
        return info.path;
    }
    throw std::runtime_error("Script not found: " + scriptName);
}

std::string ScriptManager::BuildCommand(const std::string &scriptPath, const std::vector<std::string> &scriptArgs) const
{
    std::string command = scriptPath;
    for (const auto &arg : scriptArgs)
    {
        command += " " + arg;
    }
    return command;
}

std::string ScriptManager::ExecuteCommand(const std::string &command) const
{
    std::array<char, 4096> buffer;
    std::string result;

    auto handle = ::popen(command.c_str(), "r");
    if (!handle)
    {
        throw std::runtime_error("Error opening pipe for command: " + command);
    }
    while (!feof(handle))
    {
        if (fgets(buffer.data(), static_cast<int>(buffer.size()), handle) != nullptr)
        {
            result += buffer.data();
        }
    }
    int ret = ::pclose(handle);
    if (ret != 0)
    {
        spdlog::error("Command failed with error code: {}", ret);
        throw std::runtime_error("Command failed with error code: " + std::to_string(ret));
    }
    return result;
}

std::string ScriptManager::ReadScriptFromFile(const std::string &scriptPath) const
{
    std::filesystem::path path(scriptPath);
    std::ifstream file(path);
    if (!file.is_open())
    {
        spdlog::error("Can't open script file: {}", scriptPath);
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void ScriptManager::RemoveFromRunningScripts(const std::string &scriptName)
{
    std::scoped_lock lock(m_scriptInfoMutex);
    if (m_scriptInfoMap.contains(scriptName))
    {
        ScriptInfo &info = m_scriptInfoMap.at(scriptName);
        info.isRunning = false;
        info.cv.notify_all();
    }
}
