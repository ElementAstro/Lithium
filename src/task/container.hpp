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

#include "atom/task/task.hpp"
#include <shared_mutex>

namespace lithium {
class TaskContainer {
public:
    static std::shared_ptr<TaskContainer> createShared();

    // Task management
    void addTask(const std::shared_ptr<SimpleTask> &task);
    std::optional<std::shared_ptr<SimpleTask>> getTask(const std::string &name);
    bool removeTask(const std::string &name);
    std::vector<std::shared_ptr<SimpleTask>> getAllTasks();
    size_t getTaskCount();
    void clearTasks();
    std::vector<std::shared_ptr<SimpleTask>> findTasks(int priority,
                                                       bool status);
    void sortTasks(
        const std::function<bool(const std::shared_ptr<SimpleTask> &,
                                 const std::shared_ptr<SimpleTask> &)> &cmp);
    void batchAddTasks(
        const std::vector<std::shared_ptr<SimpleTask>> &tasksToAdd);
    void batchRemoveTasks(const std::vector<std::string> &taskNamesToRemove);
    void batchModifyTasks(
        const std::function<void(std::shared_ptr<SimpleTask> &)> &modifyFunc);

    // Task parameters management
    bool addOrUpdateTaskParams(const std::string &name, const json &params);
    bool insertTaskParams(const std::string &name, const json &params,
                          const int &position);
    std::optional<json> getTaskParams(const std::string &name) const;
    void listTaskParams() const;

private:
    mutable std::shared_mutex mtx;  ///< Mutex for thread-safe operations.
    std::unordered_map<std::string, std::shared_ptr<SimpleTask>>
        tasks;  ///< The container holding tasks.
    std::unordered_map<std::string, json>
        taskParams;  ///< The container holding task parameters.
};
}  // namespace lithium

#endif
