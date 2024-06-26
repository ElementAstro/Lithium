/*
 * task_stack.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-21

Description: Task Stack

**************************************************/

#pragma once

#include <algorithm>
#include <memory>
#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_set8.hpp"
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#include <unordered_set>
#endif
#include <vector>

#include "atom/property/task/task.hpp"

enum class TaskStatus { Pending, Executing, Completed, Failed, Cancelled };

namespace lithium::Task {
/**
 * @brief Represents a stack of tasks.
 */
class TaskStack {
public:
    /**
     * @brief Adds a task to the task stack.
     * @param task The task to add.
     */
    void AddTask(std::shared_ptr<Atom::Task::SimpleTask> task);

    /**
     * @brief Adds a named task to the task stack.
     * @param task The task to add.
     * @param taskName The name of the task.
     */
    void AddTask(std::shared_ptr<Atom::Task::SimpleTask> task,
                 const std::string &taskName);

    /**
     * @brief Registers mutually exclusive tasks.
     * @param taskA The first task.
     * @param exclusiveTasks The set of tasks that are mutually exclusive with
     * taskA.
     */
    void RegisterMutuallyExclusiveTasks(
        const std::string &taskA,
        const std::unordered_set<std::string> &exclusiveTasks);

    /**
     * @brief Checks if there are any mutually exclusive tasks in the task
     * stack.
     * @return True if there are no mutually exclusive tasks, false otherwise.
     */
    bool CheckMutuallyExclusiveTasks() const;

    /**
     * @brief Gets the status of a task in the task stack.
     * @param index The index of the task.
     * @return The status of the task.
     */
    TaskStatus GetTaskStatus(size_t index) const;

private:
    std::vector<std::shared_ptr<Atom::Task::SimpleTask>>
        tasks_; /**< The tasks in the task stack. */
    std::vector<TaskStatus>
        task_status_; /**< The status of each task in the task stack. */
    std::vector<std::string>
        task_names_; /**< The names of the tasks in the task stack. */
#ifdef ENABLE_FASTHASH
    emhash8::hashMap<std::string, emhash8::hashSet<std::string>>
        mutually_exclusive_tasks_; /**< The map of mutually exclusive tasks. */
#else
    std::unordered_map<std::string, std::unordered_set<std::string>>
        mutually_exclusive_tasks_; /**< The map of mutually exclusive tasks. */
#endif
    /**
     * @brief Checks if a task with the given name exists in the task stack.
     * @param taskName The name of the task to check.
     * @return True if the task exists, false otherwise.
     */
    bool IsTaskInStack(const std::string &taskName) const;
};

}  // namespace lithium::Task
