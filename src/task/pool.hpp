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

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <stack>
#include <optional>
#include <tuple>
#include <memory>
#include <chrono>

namespace Lithium
{

    enum class TaskSchedulingStrategy
    {
        FIFO, // First-In-First-Out, 默认行为，使用priority_queue
        LIFO  // Last-In-First-Out, 使用stack
    };

    struct Task
    {
        int priority;
        std::function<void()> func;

        Task(int priority, std::function<void()> func) : priority(priority), func(std::move(func)) {}

        bool operator<(const Task &other) const
        {
            return priority < other.priority; // Lower integer value means higher priority
        }
    };

    /**
     * @class DynamicThreadPool
     * @brief A dynamic thread pool that can adjust the number of threads according to the number of tasks
     *        一个可以根据任务数量动态调整线程数量的线程池
     */
    class DynamicThreadPool
    {
    public:
        /**
         * @brief Constructor
         * @param threads Initial number of threads 初始线程数量
         * @param maxThreads Maximum number of threads 最大线程数量
         * @param strategy Task scheduling strategy 任务调度策略
         */
        DynamicThreadPool(size_t threads, size_t maxThreads, TaskSchedulingStrategy strategy);

        /**
         * @brief Destructor
         */
        ~DynamicThreadPool();

        /**
         * @brief Add a task to the thread pool and return a future
         *        将一个任务添加到线程池并返回一个future
         * @param priority Priority of the task 任务的优先级
         * @param f Function to be executed 函数
         * @param args Arguments for the function 函数的参数
         * @return std::future Future for retrieving the result 结果的future
         */
        template <class F, class... Args, typename = std::enable_if_t<std::is_invocable<F, Args...>::value>>
        auto enqueue(int priority, F &&f, Args &&...args)
            -> std::future<typename std::invoke_result<F, Args...>::type>;

        /**
         * @brief Add a task to the thread pool without returning a future
         *        将一个任务添加到线程池但不返回future
         * @param priority Priority of the task 任务的优先级
         * @param f Function to be executed 函数
         * @param args Arguments for the function 函数的参数
         */
        template <class F, class... Args, typename = std::enable_if_t<std::is_invocable<F, Args...>::value>>
        void enqueueDetach(int priority, F &&f, Args &&...args);

        /**
         * @brief Sleep for a given duration
         *        睡眠一段时间
         * @param duration Duration to sleep 睡眠的时长
         */
        void sleepFor(std::chrono::milliseconds duration);

        /**
         * @brief Adjust the number of threads in the pool
         *        调整线程池中的线程数量
         * @param newSize New number of threads 新的线程数量
         */
        void adjustThreadCount(size_t newSize);

        /**
         * @brief Wait for all tasks to complete
         *        等待所有任务完成
         */
        void wait();

        /**
         * @brief Determine whether to reduce the number of threads
         *        判断是否需要减少线程数量
         * @return bool Whether to reduce the number of threads 是否需要减少线程数量
         */
        bool shouldReduceThreads();

    private:
        // 添加用于存储LIFO策略任务的堆栈
        std::stack<std::shared_ptr<Task>> m_taskStack;
        TaskSchedulingStrategy m_schedulingStrategy = TaskSchedulingStrategy::FIFO;
        std::priority_queue<std::shared_ptr<Task>> m_tasks;
#if __cplusplus >= 202002L
        std::vector<std::jthread> m_workers;
#else
        std::vector<std::thread> m_workers;
#endif
        
        std::mutex m_queueMutex;
        std::condition_variable m_condition;
        std::atomic<bool> m_stop;
        std::atomic<bool> m_sleep;
        size_t m_defaultThreadCount;
        size_t m_maxThreadCount;
        std::atomic<size_t> m_activeThreads;
    };
}
#endif
