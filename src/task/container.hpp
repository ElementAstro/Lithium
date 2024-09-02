/**
 * @file container.hpp
 * @brief Task container class.
 *
 * This file contains the definition of the task container class, which is used
 * for managing tasks within the application.
 *
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef LITHIUM_TASK_CONTAINER_HPP
#define LITHIUM_TASK_CONTAINER_HPP

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include <shared_mutex>
#include "task.hpp"

#include "atom/type/json_fwd.hpp"

namespace lithium {
class TaskContainer {
public:
    static auto createShared() -> std::shared_ptr<TaskContainer>;

    // Task management
    void addTask(const std::shared_ptr<Task> &task);
    auto getTask(const std::string &name)
        -> std::optional<std::shared_ptr<Task>>;
    auto removeTask(const std::string &name) -> bool;
    auto getAllTasks() -> std::vector<std::shared_ptr<Task>>;
    auto getTaskCount() -> size_t;
    void clearTasks();
    auto findTasks(int priority,
                   Task::Status status) -> std::vector<std::shared_ptr<Task>>;
    void sortTasks(
        const std::function<bool(const std::shared_ptr<Task> &,
                                 const std::shared_ptr<Task> &)> &cmp);
    void batchAddTasks(const std::vector<std::shared_ptr<Task>> &tasksToAdd);
    void batchRemoveTasks(const std::vector<std::string> &taskNamesToRemove);
    void batchModifyTasks(
        const std::function<void(std::shared_ptr<Task> &)> &modifyFunc);

    // Task parameters management
    auto addOrUpdateTaskParams(const std::string &name,
                               const json &params) -> bool;
    auto insertTaskParams(const std::string &name, const json &params,
                          const int &position) -> bool;
    auto getTaskParams(const std::string &name) const -> std::optional<json>;
    void listTaskParams() const;

private:
    mutable std::shared_mutex mtx_;  ///< Mutex for thread-safe operations.
    std::unordered_map<std::string, std::shared_ptr<Task>>
        tasks_;  ///< The container holding tasks.
    std::unordered_map<std::string, json>
        taskParams_;  ///< The container holding task parameters.
};
}  // namespace lithium

#endif
