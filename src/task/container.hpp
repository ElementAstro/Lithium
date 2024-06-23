/*
 * container.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Task container class.

**************************************************/

#ifndef LITHIUM_TASK_CONTAINER_HPP
#define LITHIUM_TASK_CONTAINER_HPP

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "task.hpp"
#include <shared_mutex>

namespace lithium {
class TaskContainer {
public:
    static std::shared_ptr<TaskContainer> createShared();

    // Task management
    void addTask(const std::shared_ptr<Task> &task);
    std::optional<std::shared_ptr<Task>> getTask(const std::string &name);
    bool removeTask(const std::string &name);
    std::vector<std::shared_ptr<Task>> getAllTasks();
    size_t getTaskCount();
    void clearTasks();
    std::vector<std::shared_ptr<Task>> findTasks(int priority,
                                                       Task::Status status);
    void sortTasks(
        const std::function<bool(const std::shared_ptr<Task> &,
                                 const std::shared_ptr<Task> &)> &cmp);
    void batchAddTasks(
        const std::vector<std::shared_ptr<Task>> &tasksToAdd);
    void batchRemoveTasks(const std::vector<std::string> &taskNamesToRemove);
    void batchModifyTasks(
        const std::function<void(std::shared_ptr<Task> &)> &modifyFunc);

    // Task parameters management
    bool addOrUpdateTaskParams(const std::string &name, const json &params);
    bool insertTaskParams(const std::string &name, const json &params,
                          const int &position);
    std::optional<json> getTaskParams(const std::string &name) const;
    void listTaskParams() const;

private:
    mutable std::shared_mutex mtx;  ///< Mutex for thread-safe operations.
    std::unordered_map<std::string, std::shared_ptr<Task>>
        tasks;  ///< The container holding tasks.
    std::unordered_map<std::string, json>
        taskParams;  ///< The container holding task parameters.
};
}  // namespace lithium

#endif
