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

#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <functional>

#include "atom/task/task.hpp"

namespace Lithium
{

    /**
     * @brief Task management container class.
     *
     * @details Provides functionalities to add, retrieve, remove, and manipulate tasks stored within a container.
     *
     * 类 TaskContainer - 任务管理容器类。
     * 提供添加、检索、移除以及操作容器内存储的任务的功能。
     */
    class TaskContainer
    {
    public:
        static std::shared_ptr<TaskContainer> createShared();
        
        /**
         * @brief Add a task to the container.
         *
         * @param task Shared pointer to the task being added.
         *
         * 向容器中添加一个任务。
         * @param task 被添加任务的共享指针。
         */
        void addTask(const std::shared_ptr<SimpleTask> &task);

        /**
         * @brief Retrieve a task by its name.
         *
         * @param name The name of the task to retrieve.
         * @return std::optional<std::shared_ptr<SimpleTask>> An optional containing the task if found.
         *
         * 通过名称检索任务。
         * @param name 要检索的任务的名称。
         * @return std::optional<std::shared_ptr<SimpleTask>> 如果找到任务，则包含该任务的optional。
         */
        std::optional<std::shared_ptr<SimpleTask>> getTask(const std::string &name);

        /**
         * @brief Remove a task from the container by its name.
         *
         * @param name The name of the task to be removed.
         * @return true If the task was successfully removed.
         * @return false If the task could not be found.
         *
         * 通过名称从容器中移除一个任务。
         * @param name 要移除的任务的名称。
         * @return true 如果任务成功被移除。
         * @return false 如果找不到该任务。
         */
        bool removeTask(const std::string &name);

        /**
         * @brief Get all tasks in the container.
         *
         * @return std::vector<std::shared_ptr<SimpleTask>> A vector containing all tasks.
         *
         * 获取容器中的所有任务。
         * @return std::vector<std::shared_ptr<SimpleTask>> 包含所有任务的向量。
         */
        std::vector<std::shared_ptr<SimpleTask>> getAllTasks();

        /**
         * @brief Get the number of tasks in the container.
         *
         * @return size_t The count of tasks.
         *
         * 获取容器中任务的数量。
         * @return size_t 任务的数量。
         */
        size_t getTaskCount();

        /**
         * @brief Clear all tasks from the container.
         *
         * 清空容器中的所有任务。
         */
        void clearTasks();

        /**
         * @brief Find tasks with specific priority and status.
         *
         * @param priority The priority of the tasks to find.
         * @param status The status of the tasks to find.
         * @return std::vector<std::shared_ptr<SimpleTask>> A vector containing the matching tasks.
         *
         * 查找具有特定优先级和状态的任务。
         * @param priority 要查找的任务的优先级。
         * @param status 要查找的任务的状态。
         * @return std::vector<std::shared_ptr<SimpleTask>> 包含匹配任务的向量。
         */
        std::vector<std::shared_ptr<SimpleTask>> findTasks(int priority, bool status);

        /**
         * @brief Sort tasks in the container using a custom comparison function.
         *
         * @param cmp The comparison function to use for sorting.
         *
         * 使用自定义比较函数对容器中的任务进行排序。
         * @param cmp 用于排序的比较函数。
         */
        void sortTasks(const std::function<bool(const std::shared_ptr<SimpleTask> &, const std::shared_ptr<SimpleTask> &)> &cmp);

        /**
         * @brief Add multiple tasks to the container at once.
         *
         * @param tasksToAdd A vector containing the tasks to add.
         *
         * 一次性向容器中添加多个任务。
         * @param tasksToAdd 包含要添加的任务的向量。
         */
        void batchAddTasks(const std::vector<std::shared_ptr<SimpleTask>> &tasksToAdd);

        /**
         * @brief Remove multiple tasks from the container by their names.
         *
         * @param taskNamesToRemove A vector containing the names of the tasks to remove.
         *
         * 通过它们的名称从容器中移除多个任务。
         * @param taskNamesToRemove 包含要移除的任务名称的向量。
         */
        void batchRemoveTasks(const std::vector<std::string> &taskNamesToRemove);

        /**
         * @brief Modify tasks in the container using a custom modification function.
         *
         * @param modifyFunc The modification function to apply to each task.
         *
         * 使用自定义修改函数修改容器中的任务。
         * @param modifyFunc 应用于每个任务的修改函数。
         */
        void batchModifyTasks(const std::function<void(std::shared_ptr<SimpleTask> &)> &modifyFunc);

    private:
        std::map<std::string, std::shared_ptr<SimpleTask>> tasks; ///< The container holding tasks.
        std::mutex mutex;                                               ///< Mutex for thread-safe operations on the container.
    };
} // namespace Lithium

#endif
