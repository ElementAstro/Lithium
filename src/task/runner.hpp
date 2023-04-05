/*
 * runner.hpp
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
 
Date: 2023-3-29
 
Description: Task Runner
 
**************************************************/

#pragma once

#include <vector>
#include <memory>

#include "nlohmann/json.hpp"

namespace OpenAPT
{
    class BasicTask;
    class TaskGenerator {
        public:
            TaskGenerator() {}

            /**
             * @brief 生成一个简单任务
             * @param taskName 任务名称
             * @param description 任务描述
             * @param params 任务参数
             * @return 生成的带执行时间和描述信息的基础任务指针
             * @details Generates a new simple task with the specified name, description, and parameters.
             */
            std::shared_ptr<BasicTask> generateSimpleTask(const std::string& taskName, 
            const std::string& description, const nlohmann::json& params, const std::string& module_name, 
            const std::string& func_name);

            /**
             * @brief 生成一个条件判断任务
             * @param taskName 任务名称
             * @param description 任务描述
             * @param params 任务参数
             * @return 生成的带执行时间和描述信息的基础任务指针
             * @details Generates a new conditional task with the specified name, description, and parameters.
             */
            std::shared_ptr<BasicTask> generateConditionalTask(const std::string& taskName, const std::string& description, const nlohmann::json& params);

            /**
             * @brief 生成一个循环任务
             * @param taskName 任务名称
             * @param description 任务描述
             * @param params 任务参数
             * @return 生成的带执行时间和描述信息的基础任务指针
             * @details Generates a new loop task with the specified name, description, and parameters.
             */
            std::shared_ptr<BasicTask> generateLoopTask(const std::string& taskName, const std::string& description, const nlohmann::json& params);

            // 从 JSON 文件中加载任务
            std::vector<std::shared_ptr<BasicTask>> generateTasksFromFile(const std::string& filePath);

        private:
            constexpr std::size_t hash(const char* str, std::size_t value = 0) const noexcept {
                return (str[0] == '\0') ? value : hash(&str[1], (value ^ std::size_t(str[0])) * 0x100000001b3);
            }
    };

    class TaskManager
    {
        public:

            /**
            * @brief 无参构造函数，创建一个空的任务管理器
            */
            TaskManager() {}

           /**
             * @brief 带文件名参数的构造函数，从文件中加载任务数据并创建一个任务管理器
             * @param fileName 文件名，需要包含路径和扩展名
             * @details Constructs a TaskManager object with the specified file name parameter. It loads task data from the file and creates a new task manager.
             */
            TaskManager(const std::string &fileName);

            /**
             * @brief 添加一个任务到任务列表末尾
             * @param task 带执行时间和描述信息的基础任务指针
             * @details Adds a new task to the end of the task list with the specified execution time and description.
             */
            void addTask(std::shared_ptr<BasicTask> task);

            /**
             * @brief 在指定位置插入一个任务
             * @param taskIndex 指定插入位置的下标
             * @param task 带执行时间和描述信息的基础任务指针
             * @details Inserts a new task at the specified index in the task list with the specified execution time and description.
             */
            void insertTask(int taskIndex, std::shared_ptr<BasicTask> task);

            /**
             * @brief 根据下标删除一个任务
             * @param taskIndex 待删除任务的下标
             * @details Deletes the task at the specified index in the task list.
             */
            void deleteTask(int taskIndex);

            /**
             * @brief 根据任务名称删除一个任务
             * @param name 待删除任务的名称
             * @details Deletes the task with the specified name from the task list.
             */
            void deleteTaskByName(const std::string& name);

            /**
             * @brief 根据下标修改一个任务
             * @param taskIndex 待修改任务的下标
             * @param task 修改后的带执行时间和描述信息的基础任务指针
             * @details Modifies the task at the specified index in the task list with the new execution time and description.
             */
            void modifyTask(int taskIndex, std::shared_ptr<BasicTask> task);

            /**
             * @brief 根据任务名称修改一个任务
             * @param name 待修改任务的名称
             * @param task 修改后的带执行时间和描述信息的基础任务指针
             * @details Modifies the task with the specified name in the task list with the new execution time and description.
             */
            void modifyTaskByName(const std::string& name, std::shared_ptr<BasicTask> task);

            /**
             * @brief 执行所有未完成的任务
             * @details Executes all tasks in the task list that have not yet been completed.
             */
            void executeAllTasks();

            /**
             * @brief 从指定文件中加载任务数据到任务列表
             * @param fileName 包含路径和扩展名的文件名
             * @details Loads task data from the specified file into the task list.
             */
            void loadTasksFromJson(const std::string& fileName);

            /**
             * @brief 将任务列表中的数据保存到指定文件中
             * @param fileName 包含路径和扩展名的文件名
             * @details Saves task data from the task list to the specified file.
             */
            void saveTasksToJson(const std::string &fileName);

            /**
             * @brief 根据任务名称查询任务信息
             * @param name 待查询任务的名称
             * @details Queries the task information for the task with the specified name in the task list.
             */
            void queryTaskByName(const std::string& name);

            TaskGenerator m_TaskGenerator;

        private:
            std::vector<std::shared_ptr<BasicTask>> m_taskList;
    
    };

} // namespace OpenAPT
