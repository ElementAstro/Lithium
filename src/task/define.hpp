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
#include <thread>

#include "nlohmann/json.hpp"
#include <spdlog/spdlog.h>

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
        ConditionalTask(const std::function<void()> &func, 
                        const json &params, 
                        const std::function<bool(const json &)> &condition)
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
            j["name"] = m_name;
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
            j["name"] = m_name;
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
            j["name"] = m_name;
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
                spdlog::info("add task {} success", task->getName());
            }

            void insertTask(int taskIndex, std::shared_ptr<BasicTask> task)
            {
                if (taskIndex >= 0 && taskIndex <= m_taskList.size())
                {
                    m_taskList.insert(m_taskList.begin() + taskIndex, task);
                    spdlog::info("insert task {} success", task->getName());
                }
                else
                {
                    spdlog::error("Insert position out of range!");
                }
            }

            void deleteTask(int taskIndex)
            {
                if (taskIndex >= 0 && taskIndex < m_taskList.size())
                {
                    const std::string taskName = m_taskList[taskIndex]->getName();
                    m_taskList.erase(m_taskList.begin() + taskIndex);
                    spdlog::info("delete task {} success", taskName);
                }
                else
                {
                    spdlog::error("Task index out of range!");
                }
            }

            void deleteTaskByName(const std::string& name) 
            {
                for (auto iter = m_taskList.begin(); iter != m_taskList.end(); ++iter) {
                    if ((*iter)->getName() == name) {
                        const int index = std::distance(m_taskList.begin(), iter);
                        m_taskList.erase(iter);
                        spdlog::info("delete task {} success", name);
                        return;
                    }
                }
                spdlog::error("Task name not found!");
            }

            void modifyTask(int taskIndex, std::shared_ptr<BasicTask> task)
            {
                if (taskIndex >= 0 && taskIndex < m_taskList.size())
                {
                    m_taskList[taskIndex] = task;
                    spdlog::info("modify task {} success", task->getName());
                }
                else
                {
                    spdlog::error("Task index out of range!");
                }
            }

            void modifyTaskByName(const std::string& name, std::shared_ptr<BasicTask> task) 
            {
                for(auto& i : m_taskList){
                    if(i->getName() == name){
                        i = task;
                        spdlog::info("modify task {} success", name);
                        return;
                    }
                }
                spdlog::error("Task name not found!");
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

            void loadTasksFromJson(const std::string& fileName) {
                std::ifstream file(fileName);
                json j;
                file >> j;

                for (auto& task_j : j["tasks"]) {
                    const std::string type = task_j["type"].get<std::string>();
                    const std::string name = task_j["name"].get<std::string>();
                    std::shared_ptr<BasicTask> task;
                    if(type == "conditional") {
                        const auto& condition = task_j["condition"];
                        auto func = [&]() { spdlog::info("Execute conditional task {}", name); };
                        task = std::make_shared<ConditionalTask>(func, condition, [](const json &j) { return j["status"].get<int>() == 1; });
                    } else if (type == "loop") {
                        const auto& params = task_j["params"];
                        auto func = [&](const json &j) { spdlog::info("Execute loop task {} with param {}", name, j.dump()); };
                        task = std::make_shared<LoopTask>(func, params);
                    } else {
                        const auto& params = task_j["params"];
                        auto func = [&](const json &j) { spdlog::info("Execute simple task {} with param {}", name, j.dump()); };
                        task = std::make_shared<SimpleTask>(func, params);
                    }
                    task->setName(task_j["name"]);
                    addTask(task);
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
                spdlog::info("save tasks to json file {} success", fileName);
            }

            void queryTaskByName(const std::string& name) {
                for(auto& i : m_taskList){
                    if(i->getName() == name){
                        spdlog::info("Task found. Type: {}, Name: {}, Description: {}", typeid(*i.get()).name(), i->getName(), i->getDescription());
                        return;
                    }
                }
                spdlog::error("Task name not found!");
            }

        private:
            std::vector<std::shared_ptr<BasicTask>> m_taskList;
    };
}

#endif