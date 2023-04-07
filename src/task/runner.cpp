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

#include <iostream>
#include <fstream>
#include <spdlog/spdlog.h>

#include "define.hpp"
#include "runner.hpp"
#include "openapt.hpp"

namespace OpenAPT
{

    std::function<void(const nlohmann::json &)> getTaskFunction(const std::string &funcName, const std::string &moduleName, ModuleLoader &moduleLoader)
    {
        if (funcName == "Print")
        {
            return [](const nlohmann::json &j)
            {
                spdlog::info("Execute generated simple task with param {}", j.dump());
                spdlog::debug("Simple task is called");
            };
        }
        else if (funcName == "Sum")
        {
            return [](const nlohmann::json &j)
            {
                int sum = std::accumulate(j.begin(), j.end(), 0, [](int a, const nlohmann::json &b)
                                          { return a + b.get<int>(); });
                spdlog::info("The sum of the array is {}", sum);
            };
        }
        else if (moduleName != "")
        {
            return [&, moduleName, funcName](const nlohmann::json &j)
            {
                spdlog::info("Execute generated simple task with param for modules {}", j.dump());
                spdlog::debug("Simple modules task is called");
                moduleLoader.LoadAndRunFunction<void>(moduleName, funcName, funcName, false);
                spdlog::debug("Simple modules task is finished");
            };
        }
        else
        {
            spdlog::error("Unsupported function type: {}", funcName);
            return nullptr;
        }
    }

    std::shared_ptr<BasicTask> TaskGenerator::generateSimpleTask(const std::string &taskName,
                                                                 const std::string &description, const nlohmann::json &params, const std::string &moduleName,
                                                                 const std::string &funcName)
    {
        spdlog::debug("Generating simple task with task name {} and description {}", taskName, description);

        std::function<void(const nlohmann::json &)> taskFunction = getTaskFunction(funcName, moduleName, m_ModuleLoader);
        if (!taskFunction)
        {
            return nullptr;
        }

        std::shared_ptr<BasicTask> task;
        try
        {
            task = std::make_shared<SimpleTask>(taskFunction, params);
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to create simple task: {}", e.what());
            return nullptr;
        }

        if (task)
        {
            task->setName(taskName);
            task->setDescription(description);
            spdlog::info("Simple task created successfully: name={}, description={}", task->getName(), task->getDescription());
        }
        return task;
    }

    std::shared_ptr<BasicTask> TaskGenerator::generateConditionalTask(const std::string &taskName, const std::string &description, const nlohmann::json &params)
    {
        spdlog::debug("Generating conditional task with task name {} and description {}", taskName, description);

        // 使用lambda表达式替代函数指针指向函数体，让代码更加清晰易读。
        auto predicate = [](const json &j)
        { return j.contains("status") && j["status"].get<int>() == 1; };
        auto func = [&, taskName]()
        {
            spdlog::info("Execute generated conditional task: {}", taskName);
            // 在任务执行前，增加一个日志输出，打印任务名称，方便查看哪个任务被执行了。
        };

        std::shared_ptr<BasicTask> task;
        try
        {
            task = std::make_shared<ConditionalTask>(func, params, predicate);
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to create conditional task: {}", e.what());
            return nullptr;
        }

        if (task)
        {
            task->setName(taskName);
            task->setDescription(description);
            spdlog::info("Conditional task created successfully: name={}, description={}", task->getName(), task->getDescription());
        }

        return task;
    }

    std::shared_ptr<BasicTask> TaskGenerator::generateLoopTask(const std::string &taskName, const std::string &description, const nlohmann::json &params)
    {
        spdlog::debug("Generating loop task with task name {} and description {}", taskName, description);

        auto func = [&, params](const json &j)
        { spdlog::info("Execute generated loop task with param {}", params.dump()); };

        std::shared_ptr<BasicTask> task;
        try
        {
            task = std::make_shared<LoopTask>(func, params);
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to create loop task: {}", e.what());
            return nullptr;
        }

        if (task)
        {
            task->setName(taskName);
            task->setDescription(description);
            spdlog::info("Loop task created successfully: name={}, description={}", task->getName(), task->getDescription());
        }

        return task;
    }

    // 从 JSON 文件中加载任务
    std::vector<std::shared_ptr<BasicTask>> TaskGenerator::generateTasksFromFile(const std::string &filePath)
    {
        spdlog::info("Loading tasks from file {}", filePath);

        // 读取 JSON 文件
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            spdlog::error("Failed to open file: {}", filePath);
            return {};
        }

        json tasksJson;
        try
        {
            // 解析 JSON 文件中的任务
            file >> tasksJson;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to parse JSON from file {}: {}", filePath, e.what());
            return {};
        }

        // 生成任务并存储在 vector 中
        std::vector<std::shared_ptr<BasicTask>> tasks;
        for (const auto &taskJson : tasksJson["tasks"])
        {
            const std::string &type = taskJson["type"].get<std::string>();
            const std::string &name = taskJson["name"].get<std::string>();
            const std::string &desc = taskJson["description"].get<std::string>();
            const json &params = taskJson.value("params", json::array());
            const std::string &moduleName = taskJson.value("module_name", "");
            const std::string &funcName = taskJson.value("func_name", "");

            std::shared_ptr<BasicTask> task;
            if (type == "simple")
            {
                task = generateSimpleTask(name, desc, params, moduleName, funcName);
            }
            else if (type == "conditional")
            {
                task = generateConditionalTask(name, desc, params);
            }
            else if (type == "loop")
            {
                task = generateLoopTask(name, desc, params);
            }
            else
            {
                spdlog::error("Unknown task type: {}", type);
                continue;
            }

            if (task)
            {
                tasks.push_back(task);
            }
        }

        spdlog::info("Loaded {} tasks from file {}", tasks.size(), filePath);
        return tasks;
    }

    TaskManager::TaskManager(const std::string &fileName)
    {
        // 如果文件名不为空，则从文件中加载任务列表和进度
        if (!fileName.empty())
        {
            // loadTasksFromJson(fileName);
        }
    }

    void TaskManager::addTask(std::shared_ptr<BasicTask> task)
    {
        if (!task)
        {
            spdlog::error("Cannot add empty task!");
            return;
        }

        m_taskList.push_back(task);
        spdlog::info("Added task {} successfully", task->getName());
    }

    void TaskManager::insertTask(int taskIndex, std::shared_ptr<BasicTask> task)
    {
        if (!task)
        {
            spdlog::error("Cannot insert empty task!");
            return;
        }

        if (taskIndex < 0 || taskIndex > m_taskList.size())
        {
            spdlog::error("Insert position out of range!");
            return;
        }

        m_taskList.insert(m_taskList.begin() + taskIndex, task);
        spdlog::info("Inserted task {} successfully", task->getName());
    }

    void TaskManager::deleteTask(int taskIndex)
    {
        if (taskIndex < 0 || taskIndex >= m_taskList.size())
        {
            spdlog::error("Task index out of range!");
            return;
        }

        const std::string taskName = m_taskList[taskIndex]->getName();
        m_taskList.erase(m_taskList.begin() + taskIndex);
        spdlog::info("Deleted task {} successfully", taskName);
    }

    void TaskManager::deleteTaskByName(const std::string &name)
    {
        auto iter = std::find_if(m_taskList.begin(), m_taskList.end(),
                                 [&](const std::shared_ptr<BasicTask> &task)
                                 { return task->getName() == name; });

        if (iter == m_taskList.end())
        {
            spdlog::error("Task name not found!");
            return;
        }

        const std::string taskName = (*iter)->getName();
        m_taskList.erase(iter);
        spdlog::info("Deleted task {} successfully", taskName);
    }

    void TaskManager::modifyTask(int taskIndex, std::shared_ptr<BasicTask> task)
    {
        if (!task)
        {
            spdlog::error("Cannot modify with empty task!");
            return;
        }

        if (taskIndex < 0 || taskIndex >= m_taskList.size())
        {
            spdlog::error("Task index out of range!");
            return;
        }

        m_taskList[taskIndex] = task;
        spdlog::info("Modified task {} successfully", task->getName());
    }

    void TaskManager::modifyTaskByName(const std::string &name, std::shared_ptr<BasicTask> task)
    {
        auto iter = std::find_if(m_taskList.begin(), m_taskList.end(),
                                 [&](const std::shared_ptr<BasicTask> &t)
                                 { return t->getName() == name; });

        if (iter == m_taskList.end())
        {
            spdlog::error("Task name not found!");
            return;
        }

        if (!task)
        {
            spdlog::error("Cannot modify with empty task!");
            return;
        }

        (*iter) = task;
        spdlog::info("Modified task {} successfully", name);
    }

    void TaskManager::executeAllTasks()
    {
        for (auto &task : m_taskList)
        {
            if (!task->isDone())
            {
                spdlog::debug("Executing task {}", task->getName());
                task->execute();
                spdlog::debug("Finished task {}", task->getName());
            }
        }
    }

    void TaskManager::queryTaskByName(const std::string &name)
    {
        auto iter = std::find_if(m_taskList.begin(), m_taskList.end(),
                                 [&](const std::shared_ptr<BasicTask> &task)
                                 { return task->getName() == name; });

        if (iter == m_taskList.end())
        {
            spdlog::error("Task name not found!");
            return;
        }

        const std::shared_ptr<BasicTask> &task = (*iter);
        spdlog::info("Found task {} with type {}, description: {}", task->getName(), typeid(*task.get()).name(), task->getDescription());
    }

}

/**
 * @brief 检查 JSON 文件是否格式正确
 *
 * @param filename JSON 文件名
 * @return true JSON 格式正确
 * @return false JSON 格式错误或文件无法打开
 */
bool check_json(const std::string &filename)
{
    // 打开 JSON 文件
    std::ifstream fin(filename);
    if (!fin)
    {
        spdlog::error("Failed to open {}", filename);
        return false;
    }
    // 读取 JSON 数据
    nlohmann::json j;
    try
    {
        fin >> j;
    }
    catch (nlohmann::json::parse_error &e)
    {
        spdlog::error("JSON Format error : {}", e.what());
        return false;
    }
    fin.close();
    spdlog::info("{} passed check", filename);
    return true;
}
