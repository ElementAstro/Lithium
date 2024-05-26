/*
 * pool.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Specialized task pool

**************************************************/

#ifndef LITHIUM_TASK_POOL_HPP
#define LITHIUM_TASK_POOL_HPP

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <tuple>
#include <type_traits>
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
};

/**
 * @class WorkerQueue
 * @brief A queue of tasks specific to a worker thread in the pool.
 */
class WorkerQueue {
public:
    std::deque<std::shared_ptr<Task>> queue;
    std::mutex mutex;

    bool tryPop(std::shared_ptr<Task>& task);

    bool trySteal(std::shared_ptr<Task>& task);

    void push(std::shared_ptr<Task> task);
};

/**
 * @class TaskPool
 * @brief A thread pool for executing tasks asynchronously.
 */
class TaskPool {
private:
    std::atomic<bool> m_stop{false};
    std::vector<std::thread> m_workers;
    std::vector<std::unique_ptr<WorkerQueue>> m_queues;
    std::condition_variable m_condition;
    std::mutex m_conditionMutex;
    size_t m_defaultThreadCount;

    static thread_local WorkerQueue* t_localQueue;
    static thread_local size_t t_index;

public:
    explicit TaskPool(size_t threads = std::thread::hardware_concurrency());

    ~TaskPool();

    static std::shared_ptr<TaskPool> createShared(size_t threads);

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        auto wrappedTask = std::make_shared<Task>([task]() { (*task)(); });

        if (t_localQueue) {
            t_localQueue->push(wrappedTask);
        } else {
            m_queues[0]->push(wrappedTask);
        }
        m_condition.notify_one();
        return res;
    }

private:
    void workerThread(size_t index);

    bool tryStealing(std::shared_ptr<Task>& task);

    void start();

    void stop();
};

}  // namespace lithium

#endif
