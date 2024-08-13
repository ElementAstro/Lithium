/**
 * @file pool.hpp
 * @brief Specialized task pool for managing tasks.
 * 
 * This file defines a specialized task pool designed to efficiently manage
 * and execute tasks. The pool provides mechanisms for task scheduling, 
 * execution, and resource management, optimizing task processing based on
 * specific criteria or requirements.
 * 
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef LITHIUM_TASK_POOL_HPP
#define LITHIUM_TASK_POOL_HPP

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <ranges>
#include <shared_mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace lithium {

/**
 * @struct Task
 * @brief Represents a task that can be executed by the thread pool.
 */
struct Task {
    std::function<void()> func;

    /**
     * @brief Constructor for creating a task.
     * @param func The function that this task should execute.
     */
    explicit Task(std::function<void()> func) : func(std::move(func)) {}

    Task() = default;
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&&) noexcept = default;
    Task& operator=(Task&&) noexcept = default;
};

/**
 * @class WorkerQueue
 * @brief A queue of tasks specific to a worker thread in the pool.
 */
class WorkerQueue {
public:
    std::deque<std::shared_ptr<Task>> queue;
    mutable std::shared_mutex mutex;

    bool tryPop(std::shared_ptr<Task>& task);
    bool trySteal(std::shared_ptr<Task>& task);
    void push(std::shared_ptr<Task> task);
    [[nodiscard]] bool empty() const;
};

/**
 * @class TaskPool
 * @brief A thread pool for executing tasks asynchronously.
 */
class TaskPool : public std::enable_shared_from_this<TaskPool> {
public:
    explicit TaskPool(size_t threads = std::thread::hardware_concurrency());
    ~TaskPool();

    static std::shared_ptr<TaskPool> createShared(size_t threads);

    template <class F, class... Args>
        requires std::invocable<F, Args...>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;

        if (!m_acceptTasks) {
            throw std::runtime_error("TaskPool is not accepting new tasks.");
        }

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        auto wrappedTask = std::make_shared<Task>([task]() { (*task)(); });

        {
            std::shared_lock lock(m_conditionMutex);
            if (m_acceptTasks) {
                if (t_localQueue) {
                    t_localQueue->push(wrappedTask);
                } else {
                    m_queues.front()->push(wrappedTask);
                }
                m_condition.notify_one();
            }
        }
        return res;
    }

    void resize(size_t newThreadCount);
    size_t getThreadCount() const;
    void stopAcceptingTasks();

private:
    void workerThread(size_t index);
    bool tryStealing(std::shared_ptr<Task>& task);
    void start(size_t threads);
    void stop();

    std::atomic<bool> m_stop{false};
    std::atomic<bool> m_acceptTasks{true};
    std::vector<std::jthread> m_workers;
    std::vector<std::unique_ptr<WorkerQueue>> m_queues;
    std::condition_variable_any m_condition;
    mutable std::shared_mutex m_conditionMutex;
    size_t m_defaultThreadCount;

    static thread_local WorkerQueue* t_localQueue;
    static thread_local size_t t_index;

};

}  // namespace lithium

#endif
