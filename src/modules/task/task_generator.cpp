/*
 * task_generator.cpp
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

Date: 2023-7-21

Description: Task Generator

**************************************************/

#include "task_generator.hpp"

#include "modules/io/io.hpp"

#include <filesystem>

#include "toml++/toml.hpp"

namespace fs = std::filesystem;

bool TaskGenerator::loadMacros(const std::string &macroFileName)
{
    try
    {
        if (!fs::exists(macroFileName))
        {
            LOG_F(ERROR, "Macro file not found : %s", macroFileName.c_str());
            return false;
        }
        try
        {
            toml::table macros = toml::parse_file(macroFileName);
        }
        catch (const toml::parse_error &e)
        {
            LOG_F(ERROR, "Failed to parse file %s , error : %s , in %s", e.source(), e.description(), e.source().begin);
            return false;
        }

        for (const auto &kv : macros.as_table())
        {
            if (kv.first.is_string() && kv.second.is_string())
            {
                m_MacroMap[kv.first.as_string()] = kv.second.as_string();
            }
        }
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error while loading macro file: %s", e.what());
        return false;
    }

    return true;
}

bool TaskGenerator::loadMacrosFromFolder(const std::string &folderPath)
{
    try
    {
        if (!fs::is_directory(folderPath))
        {
            LOG_F(ERROR, "Invalid folder path: %s", folderPath.c_str());
            return false;
        }

        for (const auto &entry : fs::directory_iterator(folderPath))
        {
            const auto &filePath = entry.path();

            if (fs::is_regular_file(filePath) && filePath.extension() == ".json")
            {
                std::ifstream file(filePath);
                if (!file)
                {
                    LOG_F(ERROR, "Failed to open macro file: %s", filePath.c_str());
                    continue;
                }

                nlohmann::json jsonMacro;
                try
                {
                    file >> jsonMacro;
                }
                catch (const std::exception &e)
                {
                    LOG_F(ERROR, "Failed to parse macro file: %s, error: %s", filePath.c_str(), e.what());
                    continue;
                }

                if (jsonMacro.is_object())
                {
                    for (const auto &[name, content] : jsonMacro.items())
                    {
                        if (content.is_string())
                        {
                            m_MacroMap[name] = content.get<std::string>();
                        }
                    }
                }
                else
                {
                    LOG_F(ERROR, "Invalid macro file format: %s", filePath.c_str());
                    continue;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error while loading macros from folder: %s", e.what());
        return false;
    }

    return true;
}

bool TaskGenerator::addMacro(const std::string &name, const std::string &content)
{
    m_MacroMap[name] = content;
    return true;
}

bool TaskGenerator::deleteMacro(const std::string &name)
{
    return (m_MacroMap.erase(name) > 0);
}

std::optional<std::string> TaskGenerator::getMacroContent(const std::string &name)
{
    const auto it = m_MacroMap.find(name);
    if (it != m_MacroMap.end())
    {
        return it->second;
    }
    else
    {
        return std::nullopt;
    }
}

bool TaskGenerator::generateTasks(const std::string &tomlFileName)
{
    // 使用 toml++ 解析 TOML 文件
    toml::table table;
    if (!parseTomlFile(tomlFileName, table))
    {
        return false;
    }

    // 从 DeviceManager 和 PluginManager 获取任务（仅提供接口，需要实现该部分逻辑）
    DeviceManager deviceManager;
    PluginManager pluginManager;
    std::vector<std::shared_ptr<Lithium::Task::BasicTask>> tasks;

    getTasksFromManagers(deviceManager, pluginManager, tasks);

    // 将解析得到的任务添加到 TaskManager 中
    for (const auto &task : tasks)
    {
        taskManager.addTask(task);
    }

    // 将任务清单保存为 JSON 格式
    std::string jsonFileName = tomlFileName + ".json";
    saveTasksToJson(jsonFileName, tasks);

    return true;
}

bool TaskGenerator::parseTomlFile(const std::string &tomlFileName, toml::table &table)
{
    try
    {
        std::ifstream file(tomlFileName);
        if (!file)
        {
            LOG_F(ERROR, "Failed to open TOML file: %s", tomlFileName.c_str());
            return false;
        }
        table = toml::parse(file);
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error while parsing TOML file: %s", e.what());
        return false;
    }

    return true;
}

void TaskGenerator::getTasksFromManagers(DeviceManager &deviceManager, PluginManager &pluginManager, std::vector<std::shared_ptr<Lithium::Task::BasicTask>> &tasks)
{
    // 在这里调用 DeviceManager 和 PluginManager 的接口获取任务
    // ...

    // 示例：添加两个任务到 tasks 中
    tasks.push_back(std::make_shared<Lithium::Task::BasicTask>("Task 1", "Type 1"));
    tasks.push_back(std::make_shared<Lithium::Task::BasicTask>("Task 2", "Type 2"));
}

void TaskGenerator::saveTasksToJson(const std::string &jsonFileName, const std::vector<std::shared_ptr<Lithium::Task::BasicTask>> &tasks)
{
    json jsonTasks;
    for (const auto &task : tasks)
    {
        json jsonTask;
        jsonTask["name"] = task->getName();
        jsonTask["type"] = task->getType();
        jsonTasks.push_back(jsonTask);
    }

    try
    {
        std::ofstream jsonFile(jsonFileName);
        if (!jsonFile)
        {
            LOG_F(ERROR, "Failed to open JSON file: %s", jsonFileName.c_str());
            return;
        }
        jsonFile << jsonTasks.dump(4); // 使用四个空格缩进
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error while saving JSON file: %s", e.what());
        return;
    }
}