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

extern MyApp m_App;

namespace OpenAPT
{

    // This function returns a std::function that takes a const nlohmann::json& parameter and returns void.
    // It takes the function name as a string, the module name as a string, and a pointer to ModuleLoader as parameters.
    std::function<void(const nlohmann::json &)> getTaskFunction(const std::string &funcName, const std::string &moduleName, const ModuleLoader *moduleLoader)
    {
        // If the function name is "Print", return a lambda function that prints a debug message.
        if (funcName == "Print")
        {
            return [](const nlohmann::json &j)
            {
                spdlog::debug("Simple task is called");
            };
        }
        // If the function name is "Sum", return a lambda function that calculates the sum of the array in the json object and prints it out using spdlog.
        else if (funcName == "Sum")
        {
            return [](const nlohmann::json &j)
            {
                int sum = std::accumulate(j.begin(), j.end(), 0, [](int a, const nlohmann::json &b)
                                          { return a + b.get<int>(); });
                spdlog::debug("The sum of the array is {}", sum);
            };
        }
        // If the module name is not an empty string, return a lambda function that loads and runs the specified module function using ModuleLoader.
        else if (moduleName != "")
        {
            return [moduleName, funcName](const nlohmann::json &j)
            {
                spdlog::debug("Simple modules task is called");
                m_App.GetModuleLoader()->LoadAndRunFunction<void>(moduleName, funcName, funcName, false);
                spdlog::debug("Simple modules task is finished");
            };
        }
        // If the function name is not "Print" or "Sum" and the module name is an empty string, print an error message and return a null std::function.
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
        if constexpr (std::is_same_v<decltype(taskFunction), decltype(getTaskFunction(funcName, moduleName, m_App.GetModuleLoader()))>)
        {
            taskFunction = getTaskFunction(funcName, moduleName, m_App.GetModuleLoader());
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
        if (auto it = tasksJson.find("tasks"); it == tasksJson.end())
        {
            spdlog::error("JSON does not contain a 'tasks' array");
            return tasks;
        }
        for (const auto &taskJson : it.value())
        {
            if (!taskJson.contains("type") || !taskJson.contains("name") || !taskJson.contains("description"))
            {
                spdlog::error("Invalid task JSON: missing required field");
                continue;
            }
            const std::string &type = taskJson["type"].get<std::string>();
            const std::string &name = taskJson["name"].get<std::string>();
            const std::string &desc = taskJson["description"].get<std::string>();
            const json &params = taskJson.value("params", json::array());
            const std::string &moduleName = taskJson.value("module_name", "");
            const std::string &funcName = taskJson.value("func_name", "");
            auto task = generateTask(type, name, desc, params, moduleName, funcName);
            if (!task)
            {
                spdlog::error("Failed to generate task {}", name);
            }
            else
            {
                tasks.push_back(std::move(task));
            }
        }
        if (!tasks.empty())
        {
            spdlog::info("Generated {} tasks", tasks.size());
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
        if (!fileName.empty())
        {
        }
    }

    void TaskManager::addTask(std::shared_ptr<BasicTask> task, bool canExecute)
    {
        if (!task)
        {
            spdlog::error("Cannot add empty task!");
            return;
        }

        task->setCanExecute(canExecute);

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

        task->setCanExecute(canExecute);

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
        // Set m_CurrentTask to null before executing any task
        m_CurrentTask = nullptr;
        // Set m_StopFlag to false at the beginning of the execution cycle
        m_StopFlag = false;
        // Loop through all the tasks in m_taskList
        for (auto &task : m_taskList)
        {
            // If stop flag has not been set
            if (!m_StopFlag)
            {
                try
                {
                    // If the task is executable
                    if (task->canExecute())
                    {
                        // Set m_CurrentTask to the current task being executed
                        m_CurrentTask = task;
                        // Log the task being executed in debug mode
                        spdlog::debug("Executing task {}", task->getName());
                        // Execute the task
                        task->execute();
                        // Log the completion of the task in debug mode
                        spdlog::debug("Finished task {}", task->getName());
                        // Increment completed task count
                        ++m_completedTaskCount;
                        // Reset m_CurrentTask to null after the task is finished
                        m_CurrentTask = nullptr;
                    }
                }
                // Handle exceptions during task execution
                catch (const std::exception &e)
                {
                    // Log the error message in case of exception
                    spdlog::error("Task {} execution failed: {}", task->getName(), e.what());
                    // Reset m_CurrentTask to null after an error occurs
                    m_CurrentTask = nullptr;
                }
            }
        }
    }

    // This function stops the current task if there is any running and sets the stop flag to true.
    void TaskManager::stopTask()
    {
        m_StopFlag = true;
        if (m_CurrentTask != nullptr)
        {
            // Log the name of the task being stopped.
            spdlog::info("Stopping task {}", m_CurrentTask->getName());
            m_CurrentTask->stop();
            // Reset the current task to null.
            m_CurrentTask = nullptr;
        }
        // Return to exit the function.
        return;
    }

    // This function executes the task specified by its name.
    void TaskManager::executeTaskByName(const std::string &name)
    {
        // Find the task with the specified name.
        auto iter = std::find_if(m_taskList.begin(), m_taskList.end(),
                                 [&](const std::shared_ptr<BasicTask> &task)
                                 { return task->getName() == name; });
        if (iter == m_taskList.end())
        {
            // If the task is not found, log an error message and return.
            spdlog::error("Task name not found!");
            return;
        }
        const std::shared_ptr<BasicTask> &task = (*iter);
        try
        {
            if (task->canExecute())
            {
                // If the task can be executed, log a debug message and execute the task.
                spdlog::debug("Executing task {}", task->getName());
                task->execute();
                // After executing the task, log a debug message indicating its completion.
                spdlog::debug("Finished task {}", task->getName());
            }
        }
        catch (const std::exception &e)
        {
            // If an exception occurs during task execution, log an error message with the exception details.
            spdlog::error("Task {} execution failed: {}", task->getName(), e.what());
        }
    }

    // This function modifies the task properties at a specified index position.
    void TaskManager::modifyTask(int taskIndex, std::shared_ptr<BasicTask> task, bool canExecute)
    {
        if (!task)
        {
            // If the task to be modified is empty, log an error message and return.
            spdlog::error("Cannot modify with empty task!");
            return;
        }
        // Set the task's canExecute property to the specified value.
        task->setCanExecute(canExecute);
        if (taskIndex < 0 || taskIndex >= m_taskList.size())
        {
            // If the task index is out of range, log an error message and return.
            spdlog::error("Task index out of range!");
            return;
        }
        // Replace the task at the specified index position with the modified task.
        m_taskList[taskIndex] = task;
        // Log a success message indicating the task was modified.
        spdlog::info("Modified task {} successfully", task->getName());
    }

    // This function modifies a task with a given name.
    // The task is replaced with another task, and its "canExecute" flag is updated.
    // If the task with the given name is not found, an error message is logged and nothing happens.
    // If the new task is empty, an error message is logged and nothing happens.
    void TaskManager::modifyTaskByName(const std::string &name, std::shared_ptr<BasicTask> task, bool canExecute)
    {
        auto iter = std::find_if(m_taskList.begin(), m_taskList.end(), [&](const std::shared_ptr<BasicTask> &t)
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
        task->setCanExecute(canExecute);
        (*iter) = task;
        spdlog::info("Modified task {} successfully", name);
    }
    // This function deletes a task with a given index.
    // If the index is out of range, an error message is logged and nothing happens.
    // If the task to be deleted is present in the skipList, it is removed from the skipList first.
    // The name of the deleted task is logged.
    void TaskManager::deleteTask(int taskIndex)
    {
        if (taskIndex < 0 || taskIndex >= m_taskList.size())
        {
            spdlog::error("Task index out of range!");
            return;
        }
        int actualIndex = taskIndex;
        auto iter = std::find(skipList.begin(), skipList.end(), actualIndex);
        if (iter != skipList.end())
        {
            skipList.erase(iter);
        }
        const std::string taskName = m_taskList[actualIndex]->getName();
        m_taskList.erase(m_taskList.begin() + actualIndex);
        spdlog::info("Deleted task {} successfully", taskName);
    }
    // This function deletes a task with a given name.
    // If the name is not found, an error message is logged and nothing happens.
    // If the task to be deleted is present in the skipList, it is removed from the skipList first.
    // The name of the deleted task is logged.
    void TaskManager::deleteTaskByName(const std::string &name)
    {
        auto iter = std::find_if(m_taskList.begin(), m_taskList.end(), [&](const std::shared_ptr<BasicTask> &task)
                                 { return task->getName() == name; });
        if (iter == m_taskList.end())
        {
            spdlog::error("Task name not found!");
            return;
        }
        int actualIndex = iter - m_taskList.begin();
        auto iter1 = std::find(skipList.begin(), skipList.end(), actualIndex);
        if (iter1 != skipList.end())
        {
            skipList.erase(iter1);
        }
        const std::string taskName = (*iter)->getName();
        m_taskList.erase(iter);
        spdlog::info("Deleted task {} successfully", taskName);
    }

    // Searches for a task with a specific name and displays it
    void TaskManager::queryTaskByName(const std::string &name)
    {
        // Find task by name
        auto iter = std::find_if(m_taskList.begin(), m_taskList.end(),
                                 [&](const std::shared_ptr<BasicTask> &task)
                                 { return task->getName() == name; });
        // If the task is not found, log an error message and return
        if (iter == m_taskList.end())
        {
            spdlog::error("Task name not found!");
            return;
        }
        // Display task information
        const std::shared_ptr<BasicTask> &task = (*iter);
        spdlog::info("Found task {} with type {}, description: {}", task->getName(), typeid(*task.get()).name(), task->getDescription());
    }
    // Returns a reference to the list of tasks
    const std::vector<std::shared_ptr<BasicTask>> &TaskManager::getTaskList() const
    {
        return m_taskList;
    }
    // Saves the list of tasks to a JSON file
    void TaskManager::saveTasksToJson(const std::string &fileName)
    {
        try
        {
            // Open output file stream
            std::ofstream outFile(fileName);
            // If the file fails to open, log an error message and return
            if (!outFile.is_open())
            {
                spdlog::error("Failed to save json file, could not create it!");
                return;
            }
            // Create JSON object and add each task to it
            json j;
            for (const auto &task : m_taskList)
            {
                json jTask;
                jTask["id"] = task->getId();
                jTask["description"] = task->getDescription();
                j.push_back(jTask);
            }
            // Write JSON to file with indentation
            outFile << std::setw(4) << j << std::endl;
            spdlog::info("Saved task information to {} successfully!", fileName);
        }
        catch (const std::exception &e)
        {
            // If an exception occurs, log an error message with the exception details
            spdlog::error("Failed to save task into a json file: {}", e.what());
        }
    }
}