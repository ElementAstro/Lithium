/*
 * task_stack.hpp
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

Description: Task Stack

**************************************************/

#pragma once

#include <memory>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "core/property/task/task.hpp"

enum class TaskStatus
{
    Pending,
    Executing,
    Completed
};

namespace Lithium::Task
{
    /**
     * @brief Represents a stack of tasks.
     */
    class TaskStack
    {
    public:
        /**
         * @brief Adds a task to the task stack.
         * @param task The task to add.
         */
        void AddTask(std::shared_ptr<BasicTask> task);

        /**
         * @brief Adds a named task to the task stack.
         * @param task The task to add.
         * @param taskName The name of the task.
         */
        void AddTask(std::shared_ptr<BasicTask> task, const std::string &taskName);

        /**
         * @brief Registers mutually exclusive tasks.
         * @param taskA The first task.
         * @param exclusiveTasks The set of tasks that are mutually exclusive with taskA.
         */
        void RegisterMutuallyExclusiveTasks(const std::string &taskA, const std::unordered_set<std::string> &exclusiveTasks);

        /**
         * @brief Checks if there are any mutually exclusive tasks in the task stack.
         * @return True if there are no mutually exclusive tasks, false otherwise.
         */
        bool CheckMutuallyExclusiveTasks() const;

        /**
         * @brief Gets the status of a task in the task stack.
         * @param index The index of the task.
         * @return The status of the task.
         */
        TaskStatus GetTaskStatus(size_t index) const;

    private:
        std::vector<std::shared_ptr<BasicTask>> tasks_;                                             /**< The tasks in the task stack. */
        std::vector<TaskStatus> task_status_;                                                       /**< The status of each task in the task stack. */
        std::vector<std::string> task_names_;                                                       /**< The names of the tasks in the task stack. */
        std::unordered_map<std::string, std::unordered_set<std::string>> mutually_exclusive_tasks_; /**< The map of mutually exclusive tasks. */

        /**
         * @brief Checks if a task with the given name exists in the task stack.
         * @param taskName The name of the task to check.
         * @return True if the task exists, false otherwise.
         */
        bool IsTaskInStack(const std::string &taskName) const;
    };

}
