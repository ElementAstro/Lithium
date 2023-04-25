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
    class TaskGenerator
    {
    public:
        TaskGenerator() {}

        /**
         * @brief 根据指定参数生成一个任务。
         * @param taskType 任务类型（simple/conditional/loop）
         * @param taskName 任务名称
         * @param description 任务描述
         * @param params 任务参数
         * @param moduleName 动态链接库名称（可选，默认为空）
         * @param funcName 函数名称（可选，默认为空）
         * @return 生成的带执行时间和描述信息的基础任务指针，如果生成失败则返回 nullptr。
         * @details 根据指定的任务类型，生成对应的任务。如果指定的任务类型未知，则返回 nullptr。
         */
        std::shared_ptr<BasicTask> generateTask(const std::string &taskType, const std::string &taskName, const std::string &description, const nlohmann::json &params, const std::string &moduleName, const std::string &funcName);

        /**
         * @brief 生成一个简单任务。
         * @param taskName 任务名称
         * @param description 任务描述
         * @param params 任务参数
         * @param moduleName 动态链接库名称（可选，默认为空）
         * @param funcName 函数名称（可选，默认为空）
         * @return 生成的带执行时间和描述信息的基础任务指针，如果生成失败则返回 nullptr。
         * @details 根据指定的参数，生成一个简单任务。如果生成失败，则返回 nullptr。
         */
        std::shared_ptr<BasicTask> generateSimpleTask(const std::string &taskName, const std::string &description, const nlohmann::json &params, const std::string &moduleName = "", const std::string &funcName = "");

        /**
         * @brief 生成一个条件判断任务。
         * @param taskName 任务名称
         * @param description 任务描述
         * @param params 任务参数
         * @return 生成的带执行时间和描述信息的基础任务指针，如果生成失败则返回 nullptr。
         * @details 根据指定的参数，生成一个条件判断任务。如果生成失败，则返回 nullptr。
         */
        std::shared_ptr<BasicTask> generateConditionalTask(const std::string &taskName, const std::string &description, const nlohmann::json &params);

        /**
         * @brief 生成一个循环任务。
         * @param taskName 任务名称
         * @param description 任务描述
         * @param params 任务参数
         * @return 生成的带执行时间和描述信息的基础任务指针，如果生成失败则返回 nullptr。
         * @details 根据指定的参数，生成一个循环任务。如果生成失败，则返回 nullptr。
         */
        std::shared_ptr<BasicTask> generateLoopTask(const std::string &taskName, const std::string &description, const nlohmann::json &params);

        /**
         * @brief 读取 JSON 文件。
         * @param filePath 文件路径
         * @param tasksJson 存储 JSON 数据的对象
         * @return 如果成功读取文件，则返回 true，否则返回 false。
         * @details 读取指定路径下的 JSON 文件，并将其解析成 json 对象。如果文件路径不存在或文件格式错误，则返回 false。
         */
        bool readJsonFile(const std::string &filePath, nlohmann::json &tasksJson);

        /**
         * @brief 根据 JSON 字符串生成任务。
         * @param tasksJson 存储任务 JSON 数据的对象
         * @return 生成的基础任务指针列表
         * @details 根据传入的 JSON 数据，生成对应的任务。如果生成失败，则返回空列表。
         */
        std::vector<std::shared_ptr<BasicTask>> generateTasksFromJson(const nlohmann::json &tasksJson);

        /**
         * @brief 从 JSON 文件中加载任务。
         * @param filePath 文件路径
         * @return 生成的基础任务指针列表
         * @details 从指定的 JSON 文件中加载任务。如果文件路径不存在或文件格式错误，则返回空列表。
         */
        std::vector<std::shared_ptr<BasicTask>> generateTasksFromFile(const std::string &filePath);

    private:
        /**
         * @brief 计算字符串的哈希值。
         * @param str 字符串指针
         * @param value 初始哈希值（可选，默认为0）
         * @return 字符串的哈希值
         * @details 计算输入字符串的哈希值，采用快速哈希算法（FNV-1a）。
         */
        constexpr std::size_t hash(const char *str, std::size_t value = 0) const noexcept
        {
            return (str[0] == '\0') ? value : hash(&str[1], (value ^ std::size_t(str[0])) * 0x100000001b3);
        }
    };

    class TaskManager
    {
    public:
        TaskManager() {
            m_TaskGenerator = nullptr;
            m_CurrentTask = nullptr;
        }

        ~TaskManager()
        {
            delete m_TaskGenerator;
            m_TaskGenerator = nullptr;
            m_CurrentTask = nullptr;
        }

        TaskManager(const std::string &fileName);

        void addTask(std::shared_ptr<BasicTask> task, bool canExecute = true);

        void insertTask(int taskIndex, std::shared_ptr<BasicTask> task, bool canExecute = true);

        void deleteTask(int taskIndex);

        void deleteTaskByName(const std::string &name);

        void modifyTask(int taskIndex, std::shared_ptr<BasicTask> task, bool canExecute);

        void modifyTaskByName(const std::string &name, std::shared_ptr<BasicTask> task, bool canExecute);

        void executeAllTasks();

        void executeTaskByName(const std::string &name);

        void stopTask();

        const std::vector<std::shared_ptr<BasicTask>>& getTaskList() const;

        void saveTasksToJson(const std::string &fileName);

        void queryTaskByName(const std::string &name);

        TaskGenerator *getGenerator()
        {
            return m_TaskGenerator;
        }

    private:
        std::vector<std::shared_ptr<BasicTask>> m_taskList;
        size_t m_completedTaskCount = 0;
        std::vector<int> skipList;

        TaskGenerator *m_TaskGenerator;

        bool m_StopFlag = false;
        std::shared_ptr<BasicTask> m_CurrentTask;
    };

} // namespace OpenAPT
