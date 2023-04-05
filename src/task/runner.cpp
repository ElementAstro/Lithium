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

namespace OpenAPT {

    std::shared_ptr<BasicTask> TaskGenerator::generateSimpleTask(const std::string& taskName, const std::string& description, const nlohmann::json& params) {
        spdlog::debug("Generating simple task with task name {} and description {}", taskName, description);

        auto func = [&](const json &j) { spdlog::info("Execute generated simple task with param {}", j.dump()); };

        std::shared_ptr<BasicTask> task;
        try {
            task = std::make_shared<SimpleTask>(func, params);
        } catch (const std::exception& e) {
            spdlog::error("Failed to create simple task: {}", e.what());
            return nullptr;
        }

        if (task) {
            task->setName(taskName);
            task->setDescription(description);
            spdlog::info("Simple task created successfully: name={}, description={}", task->getName(), task->getDescription());
        }

        return task;
    }

    std::shared_ptr<BasicTask> TaskGenerator::generateConditionalTask(const std::string& taskName, const std::string& description, const nlohmann::json& params) {
        spdlog::debug("Generating conditional task with task name {} and description {}", taskName, description);

        auto predicate = [](const json &j) { return j["status"].get<int>() == 1; };
        auto func = [&]() { spdlog::info("Execute generated conditional task"); };

        std::shared_ptr<BasicTask> task;
        try {
            task = std::make_shared<ConditionalTask>(func, params, predicate);
        } catch (const std::exception& e) {
            spdlog::error("Failed to create conditional task: {}", e.what());
            return nullptr;
        }

        if (task) {
            task->setName(taskName);
            task->setDescription(description);
            spdlog::info("Conditional task created successfully: name={}, description={}", task->getName(), task->getDescription());
        }

        return task;
    }

    std::shared_ptr<BasicTask> TaskGenerator::generateLoopTask(const std::string& taskName, const std::string& description, const nlohmann::json& params) {
        spdlog::debug("Generating loop task with task name {} and description {}", taskName, description);

        auto func = [&](const json &j) { spdlog::info("Execute generated loop task with param {}", j.dump()); };

        std::shared_ptr<BasicTask> task;
        try {
            task = std::make_shared<LoopTask>(func, params);
        } catch (const std::exception& e) {
            spdlog::error("Failed to create loop task: {}", e.what());
            return nullptr;
        }

        if (task) {
            task->setName(taskName);
            task->setDescription(description);
            spdlog::info("Loop task created successfully: name={}, description={}", task->getName(), task->getDescription());
        }

        return task;
    }

    // 从 JSON 文件中加载任务
    std::vector<std::shared_ptr<BasicTask>> TaskGenerator::generateTasksFromFile(const std::string& filePath) {
        spdlog::info("Loading tasks from file {}", filePath);

        // 读取 JSON 文件
        std::ifstream file(filePath);
        if (!file.is_open()) {
            spdlog::error("Failed to open file: {}", filePath);
            return {};
        }

        // 解析 JSON 文件中的任务
        json tasksJson;
        try {
            file >> tasksJson;
        } catch (const std::exception& e) {
            spdlog::error("Failed to parse JSON from file {}: {}", filePath, e.what());
            return {};
        }

        // 生成任务并存储在 vector 中
        std::vector<std::shared_ptr<BasicTask>> tasks;
        for (const auto& taskJson : tasksJson["tasks"]) {
            const std::string& type = taskJson["type"].get<std::string>();
            const std::string& name = taskJson["name"].get<std::string>();
            const std::string& desc = taskJson["description"].get<std::string>();
            const json& params = taskJson["params"];

            std::shared_ptr<BasicTask> task;
            if (type == "simple") {
                task = generateSimpleTask(name, desc, params);
            } else if (type == "conditional") {
                task = generateConditionalTask(name, desc, params);
            } else if (type == "loop") {
                task = generateLoopTask(name, desc, params);
            } else {
                spdlog::error("Unknown task type: {}", type);
                continue;
            }

            if (task) {
                tasks.push_back(task);
            }
        }

        spdlog::info("Loaded {} tasks from file {}", tasks.size(), filePath);
        return tasks;
    }

    TaskManager::TaskManager(const std::string &fileName) {
        // 如果文件名不为空，则从文件中加载任务列表和进度
        if (!fileName.empty()) {
            loadTasksFromJson(fileName);
        }
    }

    void TaskManager::addTask(std::shared_ptr<BasicTask> task) {
        m_taskList.push_back(task);
        spdlog::info("add task {} success", task->getName());
    }

    void TaskManager::insertTask(int taskIndex, std::shared_ptr<BasicTask> task) {
        if (taskIndex >= 0 && taskIndex <= m_taskList.size()) {
            m_taskList.insert(m_taskList.begin() + taskIndex, task);
            spdlog::info("insert task {} success", task->getName());
        } else {
            spdlog::error("Insert position out of range!");
        }
    }

    void TaskManager::deleteTask(int taskIndex) {
        if (taskIndex >= 0 && taskIndex < m_taskList.size()) {
            const std::string taskName = m_taskList[taskIndex]->getName();
            m_taskList.erase(m_taskList.begin() + taskIndex);
            spdlog::info("delete task {} success", taskName);
        } else {
            spdlog::error("Task index out of range!");
        }
    }

    void TaskManager::deleteTaskByName(const std::string& name) {
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

    void TaskManager::modifyTask(int taskIndex, std::shared_ptr<BasicTask> task) {
        if (taskIndex >= 0 && taskIndex < m_taskList.size()) {
            m_taskList[taskIndex] = task;
            spdlog::info("modify task {} success", task->getName());
        } else {
            spdlog::error("Task index out of range!");
        }
    }

    void TaskManager::modifyTaskByName(const std::string& name, std::shared_ptr<BasicTask> task) {
        for (auto& i : m_taskList) {
            if(i->getName() == name) {
                i = task;
                spdlog::info("modify task {} success", name);
                return;
            }
        }
        spdlog::error("Task name not found!");
    }

    void TaskManager::executeAllTasks() {
        for (auto &task : m_taskList) {
            if (!task->isDone()) {
                task->execute();
            }
        }
    }

    void TaskManager::loadTasksFromJson(const std::string& fileName) {
        std::ifstream file(fileName);
        json j;
        file >> j;
        for (auto& task_j : j["tasks"]) {
            const std::string type = task_j["type"].get<std::string>();
            const std::string name = task_j["name"].get<std::string>();
            std::shared_ptr<BasicTask> task;
            if(type == "conditional") {
                const auto& condition = task_j["condition"];
                auto func = [&]() {
                    spdlog::info("Execute conditional task {}", name);
                }
                ;
                task = std::make_shared<ConditionalTask>(func, condition, [](const json &j) {
                    return j["status"].get<int>() == 1;
                }
                );
            } else if (type == "loop") {
                const auto& params = task_j["params"];
                auto func = [&](const json &j) {
                    spdlog::info("Execute loop task {} with param {}", name, j.dump());
                }
                ;
                task = std::make_shared<LoopTask>(func, params);
            } else {
                const auto& params = task_j["params"];
                auto func = [&](const json &j) {
                    spdlog::info("Execute simple task {} with param {}", name, j.dump());
                }
                ;
                task = std::make_shared<SimpleTask>(func, params);
            }
            task->setName(task_j["name"]);
            addTask(task);
        }
    }

    void TaskManager::saveTasksToJson(const std::string &fileName) {
        json j;
        for (auto &task : m_taskList) {
            if (!task->isDone()) {
                j.push_back(task->toJson());
            }
        }
        std::ofstream ofs(fileName);
        ofs << j.dump(4);
        spdlog::info("save tasks to json file {} success", fileName);
    }

    void TaskManager::queryTaskByName(const std::string& name) {
        for (auto& i : m_taskList) {
            if(i->getName() == name) {
                spdlog::info("Task found. Type: {}, Name: {}, Description: {}", typeid(*i.get()).name(), i->getName(), i->getDescription());
                return;
            }
        }
        spdlog::error("Task name not found!");
    }
}

/**
 * @brief 检查 JSON 文件是否格式正确
 * 
 * @param filename JSON 文件名
 * @return true JSON 格式正确
 * @return false JSON 格式错误或文件无法打开
 */
bool check_json(const std::string& filename) {
    // 打开 JSON 文件
    std::ifstream fin(filename);
    if (!fin) {
        spdlog::error("Failed to open {}", filename);
        return false;
    }
    // 读取 JSON 数据
    nlohmann::json j;
    try {
        fin >> j;
    } catch (nlohmann::json::parse_error& e) {
        spdlog::error("JSON Format error : {}", e.what());
        return false;
    }
    fin.close();
    spdlog::info("{} passed check", filename);
    return true;
}

