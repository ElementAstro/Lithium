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

#include "loguru/loguru.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace Lithium
{
    TaskGenerator::TaskGenerator(std::shared_ptr<DeviceManager> deviceManager)
    {
        m_DeviceManager = deviceManager;
    }

    bool TaskGenerator::loadMacros(const std::string &macroFileName)
    {
        try
        {
            if (!fs::exists(macroFileName))
            {
                DLOG_F(ERROR, "Macro file not found : %s", macroFileName.c_str());
                return false;
            }
            try
            {
                json macros;
                std::ifstream file(macroFileName);
                file >> macros;
            }
            catch (const std::exception &e)
            {
                DLOG_F(ERROR, "Failed to parse file %s , error : %s", macroFileName.c_str(), e.what());
                return false;
            }
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "Error while loading macro file: %s", e.what());
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
                DLOG_F(ERROR, "Invalid folder path: %s", folderPath.c_str());
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
                        DLOG_F(ERROR, "Failed to open macro file: %s", filePath.c_str());
                        continue;
                    }

                    json jsonMacro;
                    try
                    {
                        file >> jsonMacro;
                    }
                    catch (const std::exception &e)
                    {
                        DLOG_F(ERROR, "Failed to parse macro file: %s, error: %s", filePath.c_str(), e.what());
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
                        DLOG_F(ERROR, "Invalid macro file format: %s", filePath.c_str());
                        continue;
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "Error while loading macros from folder: %s", e.what());
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

    bool TaskGenerator::generateTasks(const std::string &jsonFileName)
    {
        json jsonTasks;
        if (!parseJsonFile(jsonFileName, jsonTasks))
        {
            return false;
        }

        // 从 DeviceManager 和 PluginManager 获取任务（仅提供接口，需要实现该部分逻辑）

        // 将解析得到的任务添加到 TaskManager 中
        // 将任务清单保存为 JSON 格式
        std::string outputJsonFileName = jsonFileName + ".json";
        saveTasksToJson(outputJsonFileName, jsonTasks);

        return true;
    }

    bool TaskGenerator::parseJsonFile(const std::string &jsonFileName, json &jsonTasks)
    {
        try
        {
            std::ifstream file(jsonFileName);
            if (!file)
            {
                DLOG_F(ERROR, "Failed to open JSON file: %s", jsonFileName.c_str());
                return false;
            }
            file >> jsonTasks;
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "Error while parsing JSON file: %s", e.what());
            return false;
        }

        return true;
    }

    void TaskGenerator::saveTasksToJson(const std::string &jsonFileName, const json &jsonTasks)
    {
        try
        {
            std::ofstream jsonFile(jsonFileName);
            if (!jsonFile)
            {
                DLOG_F(ERROR, "Failed to open JSON file: %s", jsonFileName.c_str());
                return;
            }
            jsonFile << jsonTasks.dump(4); // 使用四个空格缩进
        }
        catch (const std::exception &e)
        {
            DLOG_F(ERROR, "Error while saving JSON file: %s", e.what());
            return;
        }
    }
}
