/*
 * task_generator.hpp
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

#pragma once

#define TASK_GENERATOR_ENABLE

#include <unordered_map>
#include <fstream>

#include "nlohmann/json.hpp"


using json = nlohmann::json;

class TaskGenerator
{
private:
    std::unordered_map<std::string, std::string> m_MacroMap; // 宏名称到宏内容的映射表

public:

    explicit TaskGenerator();
    
    bool loadMacros(const std::string &macroFileName);
    bool loadMacrosFromFolder(const std::string &folderPath);
    bool addMacro(const std::string &name, const std::string &content);
    bool deleteMacro(const std::string &name);
    std::optional<std::string> getMacroContent(const std::string &name);

    bool generateTasks(const std::string &tomlFileName);

private:
    bool parseTomlFile(const std::string &tomlFileName, toml::table &table);
    void getTasksFromManagers(DeviceManager &deviceManager, PluginManager &pluginManager, std::vector<std::shared_ptr<Lithium::Task::BasicTask>> &tasks);
    void saveTasksToJson(const std::string &jsonFileName, const std::vector<std::shared_ptr<Lithium::Task::BasicTask>> &tasks);
};