/*
 * list.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Thread safe task list

**************************************************/

#include "list.hpp"

#include <iostream>
#include <mutex>
#include <thread>


namespace lithium {
std::shared_ptr<TaskList> TaskList::createShared() {
    return std::make_shared<TaskList>();
}

bool TaskList::addOrUpdateTask(const std::string &name, const json &params) {
    std::unique_lock lock(mtx);  // 用于写操作
    auto [it, inserted] = tasks.try_emplace(name, params);
    if (!inserted) {
        it->second = params;  // 更新现有任务
    }
    return inserted;
}

bool TaskList::insertTask(const std::string &name, const json &params,
                          const int &position) {
    std::unique_lock lock(mtx);  // 用于写操作
    if (tasks.find(name) != tasks.end()) {
        return false;  // 任务名已存在
    }
    if (position > tasks.size()) {
        return false;  // 位置超出当前任务列表范围
    }
    // 插入任务到指定位置（需要转换为vector进行操作）
    std::vector<std::pair<std::string, json>> temp(tasks.begin(), tasks.end());
    temp.insert(temp.begin() + position, {name, params});
    tasks.clear();
    for (const auto &task : temp) {
        tasks[task.first] = task.second;
    }
    return true;
}

bool TaskList::removeTask(const std::string &name) {
    std::unique_lock lock(mtx);  // 用于写操作
    return tasks.erase(name) > 0;
}

std::optional<json> TaskList::getTaskParams(const std::string &name) const {
    std::shared_lock lock(mtx);  // 用于读操作
    auto it = tasks.find(name);
    if (it != tasks.end()) {
        return it->second;
    }
    return std::nullopt;
}

void TaskList::listTasks() const {
    std::shared_lock lock(mtx);  // 用于读操作
    for (const auto &[name, params] : tasks) {
        std::cout << name << ": " << params.dump() << std::endl;
    }
}

std::unordered_map<std::string, json> TaskList::getTasks() { return tasks; }
}  // namespace lithium
