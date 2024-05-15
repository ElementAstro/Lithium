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

void TaskContainer::addTask(const std::shared_ptr<SimpleTask> &task) {
    if (!task) {
        LOG_F(ERROR, "TaskContainer::addTask - Task is null.");
        return;
    }
    std::lock_guard lock(mutex);
    tasks[task->getName()] = task;
}

std::optional<std::shared_ptr<SimpleTask>> TaskContainer::getTask(
    const std::string &name) {
    std::lock_guard lock(mutex);
    auto it = tasks.find(name);
    if (it != tasks.end()) {
        return it->second;
    }
    LOG_F(ERROR, "TaskContainer::getTask - Task with name {} not found.", name);
    return std::nullopt;
}

bool TaskContainer::removeTask(const std::string &name) {
    std::lock_guard lock(mutex);
    return tasks.erase(name) > 0;
}

std::vector<std::shared_ptr<SimpleTask>> TaskContainer::getAllTasks() {
    std::lock_guard lock(mutex);
    std::vector<std::shared_ptr<SimpleTask>> result;
    for (const auto &[_, task] : tasks) {
        result.push_back(task);
    }
    return result;
}

size_t TaskContainer::getTaskCount() {
    std::lock_guard lock(mutex);
    return tasks.size();
}

void TaskContainer::clearTasks() {
    std::lock_guard lock(mutex);
    tasks.clear();
}

std::vector<std::shared_ptr<SimpleTask>> TaskContainer::findTasks(int priority,
                                                                  bool status) {
    std::lock_guard lock(mutex);
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
    std::lock_guard lock(mutex);
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
    std::lock_guard lock(mutex);
    for (const auto &task : tasksToAdd) {
        if (task) {
            tasks[task->getName()] = task;
        }
    }
}

void TaskContainer::batchRemoveTasks(
    const std::vector<std::string> &taskNamesToRemove) {
    std::lock_guard lock(mutex);
    for (const auto &name : taskNamesToRemove) {
        tasks.erase(name);
    }
}

void TaskContainer::batchModifyTasks(
    const std::function<void(std::shared_ptr<SimpleTask> &)> &modifyFunc) {
    std::lock_guard lock(mutex);
    for (auto &[_, task] : tasks) {
        modifyFunc(task);
    }
}
}  // namespace lithium
