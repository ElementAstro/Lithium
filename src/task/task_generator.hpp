/*
 * task_generator.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-7-21

Description: Task Generator

**************************************************/

#pragma once

#define TASK_GENERATOR_ENABLE

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <fstream>

#include "components/device/device_manager.hpp"
#include "components/plugin/plugin_loader.hpp"
#include "atom/property/task/task.hpp"

using json = nlohmann::json;

namespace Lithium::Task
{
    class TaskGenerator
    {
    public:
        explicit TaskGenerator(std::shared_ptr<DeviceManager> deviceManager);
        bool loadMacros(const std::string &macroFileName);
        bool loadMacrosFromFolder(const std::string &folderPath);
        bool addMacro(const std::string &name, const std::string &content);
        bool deleteMacro(const std::string &name);
        std::optional<std::string> getMacroContent(const std::string &name);
        bool generateTasks(const std::string &jsonFileName);

    private:
        bool parseJsonFile(const std::string &jsonFileName, json &jsonTasks);
        void saveTasksToJson(const std::string &jsonFileName, const json &jsonTasks);
        void getTasksFromManagers();

    private:
        std::unordered_map<std::string, std::string> m_MacroMap;

        std::shared_ptr<DeviceManager> m_DeviceManager;
    };

}
