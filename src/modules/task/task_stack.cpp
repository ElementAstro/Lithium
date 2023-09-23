/*
 * task_stack.cpp
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

Description: Task Stack (just log the task)

**************************************************/

#include "task_stack.hpp"

namespace Lithium::Task
{
    void TaskStack::AddTask(std::shared_ptr<BasicTask> task)
    {
        tasks_.push_back(task);
        task_status_.push_back(TaskStatus::Pending);
    }

    void TaskStack::AddTask(std::shared_ptr<BasicTask> task, const std::string &taskName)
    {
        bool canInsert = true;

        // Check if the task being added is mutually exclusive with any existing tasks
        for (const auto &kv : mutually_exclusive_tasks_)
        {
            const std::string &existingTaskName = kv.first;
            const std::unordered_set<std::string> &exclusiveTasks = kv.second;

            if (exclusiveTasks.find(taskName) != exclusiveTasks.end())
            {
                // The task being added is mutually exclusive with an existing task
                std::cout << "Cannot add task '" << taskName << "'. It is mutually exclusive with task '" << existingTaskName << "'." << std::endl;
                canInsert = false;
                break;
            }
        }

        if (canInsert)
        {
            tasks_.push_back(task);
            task_status_.push_back(TaskStatus::Pending);
            task_names_.push_back(taskName);
        }
    }

    void TaskStack::RegisterMutuallyExclusiveTasks(const std::string &taskA, const std::unordered_set<std::string> &exclusiveTasks)
    {
        mutually_exclusive_tasks_[taskA] = exclusiveTasks;
        for (const auto &exclusiveTask : exclusiveTasks)
        {
            mutually_exclusive_tasks_[exclusiveTask].insert(taskA);
        }
    }

    bool TaskStack::CheckMutuallyExclusiveTasks() const
    {
        if (IsTaskInStack("Task A"))
        {
            return false;
        }

        for (const auto &kv : mutually_exclusive_tasks_)
        {
            const std::string &taskName = kv.first;
            const std::unordered_set<std::string> &exclusiveTasks = kv.second;
            for (const auto &exclusiveTask : exclusiveTasks)
            {
                if (IsTaskInStack(exclusiveTask) && IsTaskInStack(taskName))
                {
                    return false;
                }
            }
        }
        return true;
    }

    TaskStatus TaskStack::GetTaskStatus(size_t index) const
    {
        if (index < task_status_.size())
        {
            return task_status_[index];
        }
        return TaskStatus::Pending;
    }

    bool TaskStack::IsTaskInStack(const std::string &taskName) const
    {
        return std::find(task_names_.begin(), task_names_.end(), taskName) != task_names_.end();
    }
}

/*
int main()
{
    auto task_a = std::make_shared<TaskA>();
    auto task_b = std::make_shared<TaskB>();
    auto task_c = std::make_shared<TaskC>();
    TaskStack task_stack;
    std::unordered_set<std::string> exclusiveTasks = {"Task A", "Task B", "Task C"};
    task_stack.RegisterMutuallyExclusiveTasks("Task A", exclusiveTasks);

    task_stack.AddTask(task_a, "Task A");
    task_stack.AddTask(task_b, "Task B");
    task_stack.AddTask(task_c, "Task C");

    task_stack.ExecuteAll();

    return 0;
}
*/
