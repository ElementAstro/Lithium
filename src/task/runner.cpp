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
                spdlog::debug("Simple task is called");
            };
        }
        else if (funcName == "Sum")
        {
            return [](const nlohmann::json &j)
            {
                int sum = std::accumulate(j.begin(), j.end(), 0, [](int a, const nlohmann::json &b)
                                          { return a + b.get<int>(); });
                spdlog::debug("The sum of the array is {}", sum);
            };
        }
        else if (moduleName != "")
        {
            return [&, moduleName, funcName](const nlohmann::json &j)
            {
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

    std::shared_ptr<BasicTask> TaskGenerator::generateTask(const std::string &taskType, const std::string &taskName, const std::string &description, const nlohmann::json &params, const std::string &moduleName, const std::string &funcName)
    {
        spdlog::debug("Generating {} task with task name {} and description {}", taskType, taskName, description);

        std::function<void(const nlohmann::json &)> taskFunction;
        if constexpr (std::is_same_v<decltype(taskFunction), decltype(getTaskFunction(funcName, moduleName, m_ModuleLoader))>)
        {
            taskFunction = getTaskFunction(funcName, moduleName, m_ModuleLoader);
        }
        else
        {
            spdlog::error("Unknown task type: {}", taskType);
            return nullptr;
        }

        std::shared_ptr<BasicTask> task;
        try
        {
            if (taskType == "simple")
            {
                task = std::make_shared<SimpleTask>(taskFunction, params);
            }
            else if (taskType == "conditional")
            {
                auto predicate = [](const json &j)
                { return j.contains("status") && j["status"].get<int>() == 1; };
                task = std::make_shared<ConditionalTask>(taskFunction, params, predicate);
            }
            else if (taskType == "loop")
            {
                task = std::make_shared<LoopTask>(taskFunction, params);
            }
            else
            {
                spdlog::error("Unknown task type: {}", taskType);
                return nullptr;
            }
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to create {} task: {}", taskType, e.what());
            return nullptr;
        }

        if (task)
        {
            task->setName(taskName);
            task->setDescription(description);
            spdlog::info("{} task created successfully: name={}, description={}", taskType, task->getName(), task->getDescription());
        }

        return task;
    }

    std::shared_ptr<BasicTask> TaskGenerator::generateSimpleTask(const std::string &taskName, const std::string &description, const nlohmann::json &params, const std::string &moduleName, const std::string &funcName)
    {
        return generateTask("simple", taskName, description, params, moduleName, funcName);
    }

    std::shared_ptr<BasicTask> TaskGenerator::generateConditionalTask(const std::string &taskName, const std::string &description, const nlohmann::json &params)
    {
        return generateTask("conditional", taskName, description, params, "", "");
    }

    std::shared_ptr<BasicTask> TaskGenerator::generateLoopTask(const std::string &taskName, const std::string &description, const nlohmann::json &params)
    {
        return generateTask("loop", taskName, description, params, "", "");
    }

    bool TaskGenerator::readJsonFile(const std::string &filePath, json &tasksJson)
    {
        std::ifstream file(filePath);
        if (!file.is_open())
        {
            spdlog::error("Failed to open file: {}", filePath);
            return false;
        }

        try
        {
            file >> tasksJson;
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to parse JSON from file {}: {}", filePath, e.what());
            return false;
        }

        return true;
    }

    std::vector<std::shared_ptr<BasicTask>> TaskGenerator::generateTasksFromJson(const json &tasksJson)
    {
        std::vector<std::shared_ptr<BasicTask>> tasks;
        for (const auto &taskJson : tasksJson["tasks"])
        {
            const std::string &type = taskJson["type"].get<std::string>();
            const std::string &name = taskJson["name"].get<std::string>();
            const std::string &desc = taskJson["description"].get<std::string>();
            const json &params = taskJson.value("params", json::array());
            const std::string &moduleName = taskJson.value("module_name", "");
            const std::string &funcName = taskJson.value("func_name", "");

            std::shared_ptr<BasicTask> task = generateTask(type, name, desc, params, moduleName, funcName);
            if (task)
            {
                tasks.push_back(task);
            }
        }

        return tasks;
    }

    std::vector<std::shared_ptr<BasicTask>> TaskGenerator::generateTasksFromFile(const std::string &filePath)
    {
        spdlog::info("Loading tasks from file {}", filePath);

        json tasksJson;
        if (!readJsonFile(filePath, tasksJson))
        {
            return {};
        }

        std::vector<std::shared_ptr<BasicTask>> tasks = generateTasksFromJson(tasksJson);
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

    void TaskManager::addTask(std::shared_ptr<BasicTask> task, bool canExecute)
    {
        if (!task)
        {
            spdlog::error("Cannot add empty task!");
            return;
        }

        task->setCanExecute(canExecute); // 将 canExecute 的值赋给任务的 canExecute 变量

        m_taskList.push_back(task);
        spdlog::info("Added task {} successfully", task->getName());
    }

    void TaskManager::insertTask(int taskIndex, std::shared_ptr<BasicTask> task, bool canExecute)
    {
        if (!task)
        {
            spdlog::error("Cannot insert empty task!");
            return;
        }

        task->setCanExecute(canExecute); // 将 canExecute 的值赋给任务的 canExecute 变量

        if (taskIndex < 0 || taskIndex > m_taskList.size())
        {
            spdlog::error("Insert position out of range!");
            return;
        }

        m_taskList.insert(m_taskList.begin() + taskIndex, task);
        spdlog::info("Inserted task {} successfully", task->getName());
    }

    void TaskManager::executeAllTasks()
    {
        for (auto &task : m_taskList)
        {
            if (!task->isDone() && task->canExecute()) // 判断任务是否可以执行
            {
                spdlog::debug("Executing task {}", task->getName());
                task->execute();
                spdlog::debug("Finished task {}", task->getName());
            }
        }
    }

    void TaskManager::modifyTask(int taskIndex, std::shared_ptr<BasicTask> task, bool canExecute)
    {
        if (!task)
        {
            spdlog::error("Cannot modify with empty task!");
            return;
        }

        task->setCanExecute(canExecute); // 将 canExecute 的值赋给任务的 canExecute 变量

        if (taskIndex < 0 || taskIndex >= m_taskList.size())
        {
            spdlog::error("Task index out of range!");
            return;
        }

        m_taskList[taskIndex] = task;
        spdlog::info("Modified task {} successfully", task->getName());
    }

    void TaskManager::modifyTaskByName(const std::string &name, std::shared_ptr<BasicTask> task, bool canExecute)
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

        task->setCanExecute(canExecute); // 将 canExecute 的值赋给任务的 canExecute 变量

        (*iter) = task;
        spdlog::info("Modified task {} successfully", name);
    }

    void TaskManager::deleteTask(int taskIndex)
    {
        if (taskIndex < 0 || taskIndex >= m_taskList.size())
        {
            spdlog::error("Task index out of range!");
            return;
        }

        int actualIndex = taskIndex;
        auto iter = std::find(skipList.begin(), skipList.end(), actualIndex);
        if (iter != skipList.end()) // 如果要删除的任务在 skipList 中，则先将其从 skipList 中移除
        {
            skipList.erase(iter);
        }

        const std::string taskName = m_taskList[actualIndex]->getName();
        m_taskList.erase(m_taskList.begin() + actualIndex);
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

        int actualIndex = iter - m_taskList.begin();
        auto iter1 = std::find(skipList.begin(), skipList.end(), actualIndex);
        if (iter1 != skipList.end()) // 如果要删除的任务在 skipList 中，则先将其从 skipList 中移除
        {
            skipList.erase(iter1);
        }

        const std::string taskName = (*iter)->getName();
        m_taskList.erase(iter);
        spdlog::info("Deleted task {} successfully", taskName);
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

    void TaskManager::cleanCompletedTasks()
    {
        auto iter = m_taskList.begin();
        while (iter != m_taskList.end())
        {
            if (true)
            {
                iter = m_taskList.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }

    void TaskManager::saveTasksToJson(const std::string &fileName)
    {
        try
        {
            std::ofstream outFile(fileName);
            if (!outFile.is_open())
            {
                spdlog::error("Failed to save json file , could not create it!");
                return;
            }
            json j;
            for (const auto &task : m_taskList)
            {
                json jTask;
                jTask["id"] = task->getId();
                jTask["description"] = task->getDescription();
                jTask["priority"] = task->getPriority();
                j.push_back(jTask);
            }
            outFile << std::setw(4) << j << std::endl;
            spdlog::info("Saved task infomation to {} successfully!", fileName);
        }
        catch (const std::exception &e)
        {
            spdlog::error("Failed to save task into a json file : {}", e.what());
        }
    }

    void TaskManager::runFromJson(const std::vector<std::string> &jsonFileNames, bool traverseScriptsFolder)
    {
        // Load tasks from specified JSON files or all JSON files in the scripts folder
        if (traverseScriptsFolder)
        {
            for (const auto &entry : std::filesystem::directory_iterator("scripts"))
            {
                if (entry.is_regular_file() && entry.path().extension() == ".json")
                {
                    std::ifstream ifs(entry.path());
                    nlohmann::json json;
                    ifs >> json;
                    ifs.close();
                }
            }
        }
        else
        {
            for (const auto &jsonFileName : jsonFileNames)
            {
                std::ifstream ifs(jsonFileName);
                nlohmann::json json;
                ifs >> json;
                ifs.close();

                for (auto &elem : json)
                {
                }
            }
        }

        // Execute the loaded tasks
        executeAllTasks();
    }

    int TaskManager::getCompletedTaskCount() const
    {
        return m_completedTaskCount;
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
