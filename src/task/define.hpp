/*
 * define.hpp
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

Date: 2023-3-28

Description: Define All of the Tasks

**************************************************/

#ifndef _TASK_DEFINE_HPP_
#define _TASK_DEFINE_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace OpenAPT {

    // 任务基类
    class BasicTask
    {
        public:
            virtual void execute() {}
            virtual json toJson() { return json(); }
            virtual bool isDone() const { return m_done; }
            int getId() const { return m_id; }
            void setId(int id) { m_id = id; }
            const std::string& getName() const { return m_name; }
            void setName(const std::string& name) { m_name = name; }
            const std::string& getDescription() const { return m_description; }
            void setDescription(const std::string& description) { m_description = description; }

        protected:
            bool m_done = false;
            bool m_saved = false; // 标记任务是否已经保存
            int m_id;
            std::string m_name;
            std::string m_description;
    };

    // 条件任务
    class ConditionalTask : public BasicTask
    {
        public:
            ConditionalTask(const std::function<void()> &func, const json &params, const std::function<bool(const json &)> &condition)
                : m_func(func), m_params(params), m_condition(condition) {}

            void execute() override
            {
                if (m_condition(m_params))
                {
                    m_func();
                }
                m_done = true;
            }
            
            json toJson() override
            {
                json j;
                j["type"] = "conditional";
                j["condition"] = m_params;
                return j;
            }

        private:
            std::function<void()> m_func;
            json m_params;
            std::function<bool(const json &)> m_condition;
    };

    // 循环任务
    class LoopTask : public BasicTask
    {
        public:
            LoopTask(const std::function<void(const json &)> &func, const json &params)
                : m_func(func), m_params(params) {}

            void execute() override
            {
                for (int i = m_progress; i < m_params["total"].get<int>(); ++i)
                {
                    m_func(m_params["items"][i]);
                    std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟任务执行时间
                    m_progress = i + 1;
                }
                m_done = true;
            }

            json toJson() override
            {
                json j;
                j["type"] = "loop";
                j["params"] = m_params;
                j["progress"] = m_progress;
                return j;
            }

        private:
            std::function<void(const json &)> m_func;
            json m_params;
            int m_progress = 0;
    };

    // 普通任务
    class SimpleTask : public BasicTask
    {
        public:
            SimpleTask(const std::function<void(const json &)> &func, const json &params)
                : m_func(func), m_params(params) {}

            void execute() override
            {
                m_func(m_params);
                m_done = true;
            }

            json toJson() override
            {
                json j;
                j["type"] = "simple";
                j["params"] = m_params;
                return j;
            }

        private:
            std::function<void(const json &)> m_func;
            json m_params;
    };

    // 任务管理器类
    class TaskManager
    {
    public:
        TaskManager() {}

        TaskManager(const std::string &fileName)
        {
            // 如果文件名不为空，则从文件中加载任务列表和进度
            if (!fileName.empty())
            {
                loadTasksFromJson(fileName);
            }
        }

        void addTask(std::shared_ptr<BasicTask> task)
        {
            m_taskList.push_back(task);
        }

        void deleteTask(int taskIndex)
        {
            if (taskIndex >= 0 && taskIndex < m_taskList.size())
            {
                m_taskList.erase(m_taskList.begin() + taskIndex);
            }
            else
            {
                std::cerr << "Task index out of range!" << std::endl;
            }
        }

        void modifyTask(int taskIndex, std::shared_ptr<BasicTask> task)
        {
            if (taskIndex >= 0 && taskIndex < m_taskList.size())
            {
                m_taskList[taskIndex] = task;
            }
            else
            {
                std::cerr << "Task index out of range!" << std::endl;
            }
        }

        void executeAllTasks()
        {
            for (auto &task : m_taskList)
            {
                if (!task->isDone())
                {
                    task->execute();
                }
            }
        }

        void loadTasksFromJson(std::string file_path) {
            std::ifstream file(file_path);
            nlohmann::json j;
            file >> j;

            for (auto& task_j : j["tasks"]) {

            }
        }

        void saveTasksToJson(const std::string &fileName)
        {
            json j;
            for (auto &task : m_taskList)
            {
                if (!task->isDone())
                {
                    j.push_back(task->toJson());
                }
            }
            std::ofstream ofs(fileName);
            ofs << j.dump(4);
        }

    private:
        std::vector<std::shared_ptr<BasicTask>> m_taskList;
    };

}

#endif