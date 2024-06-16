/*
 * container.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Task container class.

**************************************************/

#include "container.hpp"

#include "atom/log/loguru.hpp"

namespace lithium {
std::shared_ptr<TaskContainer> TaskContainer::createShared() {
    return std::make_shared<TaskContainer>();
}

// Task management

void TaskContainer::addTask(const std::shared_ptr<SimpleTask> &task) {
    if (!task) {
        // LOG_F(ERROR, "TaskContainer::addTask - Task is null.");
        return;
    }
    std::unique_lock lock(mtx);
    tasks[task->getName()] = task;
}

std::optional<std::shared_ptr<SimpleTask>> TaskContainer::getTask(
    const std::string &name) {
    std::shared_lock lock(mtx);
    auto it = tasks.find(name);
    if (it != tasks.end()) {
        return it->second;
    }
    // LOG_F(ERROR, "TaskContainer::getTask - Task with name {} not found.",
    // name);
    return std::nullopt;
}

bool TaskContainer::removeTask(const std::string &name) {
    std::unique_lock lock(mtx);
    return tasks.erase(name) > 0;
}

std::vector<std::shared_ptr<SimpleTask>> TaskContainer::getAllTasks() {
    std::shared_lock lock(mtx);
    std::vector<std::shared_ptr<SimpleTask>> result;
    for (const auto &[_, task] : tasks) {
        result.push_back(task);
    }
    return result;
}

size_t TaskContainer::getTaskCount() {
    std::shared_lock lock(mtx);
    return tasks.size();
}

void TaskContainer::clearTasks() {
    std::unique_lock lock(mtx);
    tasks.clear();
}

std::vector<std::shared_ptr<SimpleTask>> TaskContainer::findTasks(int priority,
                                                                  bool status) {
    std::shared_lock lock(mtx);
    std::vector<std::shared_ptr<SimpleTask>> result;
    for (const auto &[key, task] : tasks) {
        if (task->getPriority() == priority && task->getStatus() == status) {
            result.push_back(task);
        }
    }
    return result;
}

void TaskContainer::sortTasks(
    const std::function<bool(const std::shared_ptr<SimpleTask> &,
                             const std::shared_ptr<SimpleTask> &)> &cmp) {
    std::unique_lock lock(mtx);
    std::vector<std::shared_ptr<SimpleTask>> vec;
    for (const auto &[_, task] : tasks) {
        vec.push_back(task);
    }
    std::sort(vec.begin(), vec.end(), cmp);
    tasks.clear();
    for (const auto &task : vec) {
        tasks[task->getName()] = task;
    }
}

void TaskContainer::batchAddTasks(
    const std::vector<std::shared_ptr<SimpleTask>> &tasksToAdd) {
    std::unique_lock lock(mtx);
    for (const auto &task : tasksToAdd) {
        if (task) {
            tasks[task->getName()] = task;
        }
    }
}

void TaskContainer::batchRemoveTasks(
    const std::vector<std::string> &taskNamesToRemove) {
    std::unique_lock lock(mtx);
    for (const auto &name : taskNamesToRemove) {
        tasks.erase(name);
    }
}

void TaskContainer::batchModifyTasks(
    const std::function<void(std::shared_ptr<SimpleTask> &)> &modifyFunc) {
    std::unique_lock lock(mtx);
    for (auto &[_, task] : tasks) {
        modifyFunc(task);
    }
}

// Task parameters management

bool TaskContainer::addOrUpdateTaskParams(const std::string &name,
                                          const json &params) {
    std::unique_lock lock(mtx);
    auto [it, inserted] = taskParams.try_emplace(name, params);
    if (!inserted) {
        it->second = params;  // 更新现有任务参数
    }
    return inserted;
}

bool TaskContainer::insertTaskParams(const std::string &name,
                                     const json &params, const int &position) {
    std::unique_lock lock(mtx);
    if (taskParams.find(name) != taskParams.end()) {
        return false;  // 任务名已存在
    }
    if (position > taskParams.size()) {
        return false;  // 位置超出当前任务列表范围
    }
    // 插入任务参数到指定位置（需要转换为vector进行操作）
    std::vector<std::pair<std::string, json>> temp(taskParams.begin(),
                                                   taskParams.end());
    temp.insert(temp.begin() + position, {name, params});
    taskParams.clear();
    for (const auto &task : temp) {
        taskParams[task.first] = task.second;
    }
    return true;
}

std::optional<json> TaskContainer::getTaskParams(
    const std::string &name) const {
    std::shared_lock lock(mtx);
    auto it = taskParams.find(name);
    if (it != taskParams.end()) {
        return it->second;
    }
    return std::nullopt;
}

void TaskContainer::listTaskParams() const {
    std::shared_lock lock(mtx);
    for (const auto &[name, params] : taskParams) {
        LOG_F(INFO, "Task name: {}", name);
    }
}
}  // namespace lithium
