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
 *
 * @brief 表示可以由线程池执行的任务。
 */
struct Task {
    std::function<void()> func;

    /**
     * @brief Constructor for creating a task.
     * @param func The function that this task should execute.
     *
     * @brief 创建任务的构造函数。
     * @param func 该任务应执行的函数。
     */
    explicit Task(std::function<void()> func) : func(std::move(func)) {}
};

/**
 * @class WorkerQueue
 * @brief A queue of tasks specific to a worker thread in the pool.
 *
 * @brief 线程池中特定工作线程的任务队列。
 */
class WorkerQueue {
public:
    std::deque<std::shared_ptr<Task>> queue;
    std::mutex mutex;

    /**
     * @brief Attempts to pop a task from the queue.
     * @param task The task that was popped from the queue.
     *
     * @brief 尝试从队列中弹出一个任务。
     * @param task 从队列中弹出的任务。
     */
    bool tryPop(std::shared_ptr<Task> &task);

    /**
     * @brief Attempts to steal a task from another worker thread.
     * @param task The task that was stolen from another worker thread.
     *
     * @brief 尝试从另一个工作线程偷取一个任务。
     * @param task 从另一个工作线程偷取的任务。
     */
    bool trySteal(std::shared_ptr<Task> &task);

    /**
     * @brief Pushes a task onto the queue.
     * @param task The task to push onto the queue.
     *
     * @brief 将任务推入队列。
     * @param task 要推入队列的任务。
     */
    void push(std::shared_ptr<Task> task);

    /**
     * @brief Attempts to pop a task from the queue.
     * @param task The task that was popped from the queue.
     *
     * @brief 尝试从队列中弹出一个任务。
     * @param task 从队列中弹出的任务。
     */
    std::optional<std::shared_ptr<Task>> tryPop();

    /**
     * @brief Attempts to steal a task from another worker thread.
     * @param task The task that was stolen from another worker thread.
     *
     * @brief 尝试从另一个工作线程偷取一个任务。
     * @param task 从另一个工作线程偷取的任务。
     */
    std::optional<std::shared_ptr<Task>> trySteal();
};

/**
 * @class TaskPool
 * @brief A thread pool for executing tasks asynchronously.
 *
 * TaskPool allows you to enqueue tasks which are functions to be executed
 * by a pool of worker threads. It supports task stealing for load balancing
 * between threads.
 *
 * @brief 一个用于异步执行任务的线程池。
 *
 * TaskPool
 * 允许你将任务（即要被工作线程池执行的函数）加入队列。它支持任务窃取，以实现线程间的负载平衡。
 */
class TaskPool {
private:
    std::atomic<bool> m_stop{false};
    std::vector<std::thread> m_workers;
    std::vector<std::unique_ptr<WorkerQueue>> m_queues;
    std::condition_variable m_condition;
    std::mutex m_conditionMutex;
    size_t m_defaultThreadCount;

    static thread_local WorkerQueue *t_localQueue;
    static thread_local size_t t_index;

public:
    /**
     * @brief Constructor that initializes the thread pool with a specified
     * number of threads.
     * @param threads The number of worker threads to create.
     *
     * @brief 用指定数量的线程初始化线程池的构造函数。
     * @param threads 要创建的工作线程数量。
     */
    explicit TaskPool(size_t threads);

    /**
     * @brief Destructor that stops all worker threads and cleans up resources.
     *
     * @brief 析构函数，停止所有工作线程并清理资源。
     */
    ~TaskPool();

    static std::shared_ptr<TaskPool> createShared(size_t threads);

    /**
     * @brief Enqueues a task for execution by the thread pool.
     * @param f The function to execute.
     * @param args Arguments to pass to the function.
     * @return A future representing the result of the task.
     *
     * @brief 将一个任务入队以供线程池执行。
     * @param f 要执行的函数。
     * @param args 传递给函数的参数。
     * @return 表示任务结果的未来值。
     */
    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        static_assert(std::is_invocable_v<F, Args...>,
                      "Function must be invocable with arguments");

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
    /**
     * @brief The main worker thread function.
     * @param index The index of the worker thread.
     *
     * @brief 主工作线程函数。
     * @param index 工作线程的索引。
     */
    void workerThread(size_t index);

    /**
     * @brief Tries to steal a task from another worker thread.
     * @param task The task to steal.
     *
     * @brief 尝试从另一个工作线程窃取一个任务。
     * @param task 要窃取的任务。
     */
    bool tryStealing(std::shared_ptr<Task> &task);

    /**
     * @brief Starts all worker threads.
     *
     * @brief 启动所有工作线程。
     */
    void start();

    /**
     * @brief Stops all worker threads.
     *
     * @brief 停止所有工作线程。
     */
    void stop();
};
}  // namespace lithium

#endif
