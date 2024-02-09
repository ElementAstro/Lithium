/*
 * tick.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Tick Sheduler, just like Minecraft's

**************************************************/

#ifndef LITHIUM_TASK_TICK_HPP
#define LITHIUM_TASK_TICK_HPP

#include <vector>
#include <functional>
#include <queue>
#include <chrono>
#include <thread>
#include <future>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <atomic>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <memory>

#include "pool.hpp"

namespace Lithium
{
    struct TickTask
    {
        /**
         * @brief 任务结构体，表示一个待执行的任务
         */
        std::function<void()> func;                          // 任务函数
        int priority;                                        // 任务优先级
        unsigned long long tick;                             // 任务执行的计划刻
        std::vector<std::shared_ptr<TickTask>> dependencies; // 任务依赖
        std::function<void()> onCompletion;                  // 任务完成时的回调函数
        std::atomic_bool isRunning = false;                 // 任务是否正在执行的标志
        std::atomic_bool completed = false;                  // 任务是否已完成的标志

        /**
         * @brief 比较函数，用于优先级队列的排序
         * @param other 另一个TickTask对象
         * @return 当前任务是否小于另一个任务
         */
        bool operator<(const TickTask &other) const
        {
            return std::tie(priority, tick) < std::tie(other.priority, other.tick);
        }

        /**
         * @brief 构造函数，初始化任务对象
         * @param func 任务函数
         * @param tick 任务执行的计划刻
         * @param dependencies 任务依赖
         * @param onCompletion 任务完成时的回调函数
         */
        TickTask(std::function<void()> func, unsigned long long tick, std::vector<std::shared_ptr<TickTask>> dependencies,
                 std::function<void()> onCompletion)
            : func(std::move(func)), priority(0), tick(tick), dependencies(std::move(dependencies)),
              onCompletion(std::move(onCompletion))
        {
        }
    };

    class TickScheduler
    {
    public:
        /**
         * @brief 构造函数，初始化TickScheduler对象
         * @param threads 线程池中的线程数量
         */
        TickScheduler(size_t threads);

        /**
         * @brief 析构函数，释放资源
         */
        ~TickScheduler();

        static std::shared_ptr<TickScheduler> createShared(size_t threads);

        /**
         * @brief 调度一个任务，并返回一个指向该任务的智能指针
         * @param tick 任务执行的计划刻
         * @param f 任务函数
         * @param args 任务函数的参数
         * @return 指向调度的任务的智能指针
         */
        template <typename F, typename... Args>
        auto scheduleTask(unsigned long long tick, F &&f, Args &&...args) -> std::shared_ptr<TickTask>
        {
            auto taskFunc = [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() mutable
            {
                f(std::forward<Args>(args)...);
            };
            std::vector<std::shared_ptr<TickTask>> task_dependencies;
            auto task = std::make_shared<TickTask>(taskFunc, tick, task_dependencies, nullptr);
            {
                std::lock_guard<std::mutex> lock(tasksMutex);
                tasks.push_back(task);
            }
            cv.notify_one();
            return task;
        }

        /**
         * @brief 添加一个任务依赖关系
         * @param task 被依赖的任务
         * @param dependency 依赖的任务
         */
        void addDependency(const std::shared_ptr<TickTask> &task, const std::shared_ptr<TickTask> &dependency);

        /**
         * @brief 设置任务完成时的回调函数
         * @param task 需要设置回调函数的任务
         * @param callback 回调函数
         */
        void setCompletionCallback(const std::shared_ptr<TickTask> &task, std::function<void()> callback);

        /**
         * @brief 暂停任务调度器的执行
         */
        void pause();

        /**
         * @brief 恢复任务调度器的执行
         */
        void resume();

    private:
        std::shared_ptr<TaskPool> pool;               // 线程池对象
        std::vector<std::shared_ptr<TickTask>> tasks; // 所有待执行的任务
        std::mutex tasksMutex;                        // 任务队列的互斥锁
        std::condition_variable cv;                   // 条件变量，用于暂停和恢复任务调度器的执行
        std::atomic<unsigned long long> currentTick;  // 当前的计划刻
        std::atomic<bool> stop;                       // 停止任务调度器的标志
        std::atomic<bool> isPaused;                   // 暂停任务调度器的标志
#if __cplusplus >= 202002L
        std::jthread schedulerThread; // 任务调度器的线程
#else
        std::thread schedulerThread; // 任务调度器的线程
#endif

        /**
         * @brief 任务调度循环函数，在单独的线程中运行
         */
        void taskSchedulerLoop();

        /**
         * @brief 检查一个任务的所有依赖是否已满足
         * @param task 待检查的任务
         * @return 是否所有依赖都已满足
         */
        bool allDependenciesMet(const std::shared_ptr<TickTask> &task);

        /**
         * @brief 停止任务调度器的执行
         */
        void stopScheduler();
    };
}

#endif