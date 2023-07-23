/*
 * task_manager.hpp
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

Description: Task Manager

**************************************************/

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <fstream>

#include "task.hpp"
#include "nlohmann/json.hpp"

namespace Lithium::Task
{
    class TaskManager
    {
    public:
        TaskManager(const std::string &fileName);

        bool addTask(const std::shared_ptr<BasicTask> &task);
        bool insertTask(const std::shared_ptr<BasicTask> &task, int position);
        bool executeAllTasks();
        void stopTask();
        bool executeTaskByName(const std::string &name);
        bool modifyTask(int index, const std::shared_ptr<BasicTask> &task);
        bool modifyTaskByName(const std::string &name, const std::shared_ptr<BasicTask> &task);
        bool deleteTask(int index);
        bool deleteTaskByName(const std::string &name);
        bool queryTaskByName(const std::string &name);
        const std::vector<std::shared_ptr<BasicTask>> &getTaskList() const;
        bool saveTasksToJson() const;

    private:
        std::vector<std::shared_ptr<BasicTask>> m_TaskList;
        std::unordered_map<std::string, std::shared_ptr<BasicTask>> m_TaskMap;
        std::string m_FileName;
        bool m_StopFlag;

        std::unordered_map<std::string, std::shared_ptr<BasicTask>>::iterator findTaskByName(const std::string &name);
    };

} // namespace Lithium
