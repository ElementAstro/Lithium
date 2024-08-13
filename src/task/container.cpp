/**
 * @file container.cpp
 * @brief Task container class.
 *
 * This file contains the definition of the task container class, which is used
 * for managing tasks within the application.
 *
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#include "container.hpp"

#include <mutex>

#include "atom/log/loguru.hpp"
#include "macro.hpp"

namespace lithium {
auto TaskContainer::createShared() -> std::shared_ptr<TaskContainer> {
    return std::make_shared<TaskContainer>();
}

void TaskContainer::addTask(const std::shared_ptr<Task> &task) {
    if (!task) {
        LOG_F(ERROR, "TaskContainer::addTask - Task is null.");
        return;
    }
    std::unique_lock lock(mtx_);
    tasks_[task->getName()] = task;
}

auto TaskContainer::getTask(const std::string &name)
    -> std::optional<std::shared_ptr<Task>> {
    std::shared_lock lock(mtx_);
    auto it = tasks_.find(name);
    if (it != tasks_.end()) {
        return it->second;
    }
    LOG_F(ERROR, "TaskContainer::getTask - Task with name {} not found.", name);
    return std::nullopt;
}

auto TaskContainer::removeTask(const std::string &name) -> bool {
    std::unique_lock lock(mtx_);
    return tasks_.erase(name) > 0;
}

auto TaskContainer::getAllTasks() -> std::vector<std::shared_ptr<Task>> {
    std::shared_lock lock(mtx_);
    std::vector<std::shared_ptr<Task>> result;
    result.reserve(tasks_.size());
    for (const auto &[_, task] : tasks_) {
        result.push_back(task);
    }
    return result;
}

auto TaskContainer::getTaskCount() -> size_t {
    std::shared_lock lock(mtx_);
    return tasks_.size();
}

void TaskContainer::clearTasks() {
    std::unique_lock lock(mtx_);
    tasks_.clear();
}

auto TaskContainer::findTasks(ATOM_UNUSED int priority, Task::Status status)
    -> std::vector<std::shared_ptr<Task>> {
    std::shared_lock lock(mtx_);
    std::vector<std::shared_ptr<Task>> result;
    for (const auto &[key, task] : tasks_) {
        if (task->getStatus() == status) {
            result.push_back(task);
        }
    }
    return result;
}

void TaskContainer::sortTasks(
    const std::function<bool(const std::shared_ptr<Task> &,
                             const std::shared_ptr<Task> &)> &cmp) {
    std::unique_lock lock(mtx_);
    std::vector<std::shared_ptr<Task>> vec;
    vec.reserve(tasks_.size());
    for (const auto &[_, task] : tasks_) {
        vec.push_back(task);
    }
    std::sort(vec.begin(), vec.end(), cmp);
    tasks_.clear();
    for (const auto &task : vec) {
        tasks_[task->getName()] = task;
    }
}

void TaskContainer::batchAddTasks(
    const std::vector<std::shared_ptr<Task>> &tasks_ToAdd) {
    std::unique_lock lock(mtx_);
    for (const auto &task : tasks_ToAdd) {
        if (task) {
            tasks_[task->getName()] = task;
        }
    }
}

void TaskContainer::batchRemoveTasks(
    const std::vector<std::string> &taskNamesToRemove) {
    std::unique_lock lock(mtx_);
    for (const auto &name : taskNamesToRemove) {
        tasks_.erase(name);
    }
}

void TaskContainer::batchModifyTasks(
    const std::function<void(std::shared_ptr<Task> &)> &modifyFunc) {
    std::unique_lock lock(mtx_);
    for (auto &[_, task] : tasks_) {
        modifyFunc(task);
    }
}

auto TaskContainer::addOrUpdateTaskParams(const std::string &name,
                                          const json &params) -> bool {
    std::unique_lock lock(mtx_);
    auto [it, inserted] = taskParams_.try_emplace(name, params);
    if (!inserted) {
        it->second = params;
    }
    return inserted;
}

bool TaskContainer::insertTaskParams(const std::string &name,
                                     const json &params, const int &position) {
    std::unique_lock lock(mtx_);
    if (taskParams_.find(name) != taskParams_.end()) {
        return false;
    }
    if (static_cast<unsigned long long>(position) > taskParams_.size()) {
        return false;
    }
    std::vector<std::pair<std::string, json>> temp(taskParams_.begin(),
                                                   taskParams_.end());
    temp.insert(temp.begin() + position, {name, params});
    taskParams_.clear();
    for (const auto &task : temp) {
        taskParams_[task.first] = task.second;
    }
    return true;
}

std::optional<json> TaskContainer::getTaskParams(
    const std::string &name) const {
    std::shared_lock lock(mtx_);
    auto it = taskParams_.find(name);
    if (it != taskParams_.end()) {
        return it->second;
    }
    return std::nullopt;
}

void TaskContainer::listTaskParams() const {
    std::shared_lock lock(mtx_);
    for (const auto &[name, params] : taskParams_) {
        LOG_F(INFO, "Task name: {}", name);
    }
}
}  // namespace lithium
