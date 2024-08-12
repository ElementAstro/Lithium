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

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <utility>
#include <vector>

#include "pool.hpp"

namespace atom::utils {
class StopWatcher;
}

namespace lithium {

struct TickTask {
    std::function<void()> func;  // 任务函数
    int priority;                // 任务优先级
    unsigned long long tick;     // 任务执行的计划刻
    std::vector<std::shared_ptr<TickTask>> dependencies;  // 任务依赖
    std::function<void()> onCompletion;  // 任务完成时的回调函数
    std::atomic<bool> isRunning{false};  // 任务是否正在执行
    std::atomic<bool> completed{false};  // 任务是否已完成
    std::size_t id;                      // 任务的唯一标识符
    unsigned retryCount{0};              // 重试次数
    std::chrono::milliseconds retryInterval{0};  // 重试间隔
    std::promise<bool> timeoutPromise;  // 任务超时时的回调函数
    int timeout{0};                     // 超时时间

    bool operator<(const TickTask &other) const {
        return std::tie(priority, tick) < std::tie(other.priority, other.tick);
    }

    TickTask(std::function<void()> func, unsigned long long tick,
             std::vector<std::shared_ptr<TickTask>> dependencies = {},
             std::function<void()> onCompletion = nullptr)
        : func(std::move(func)),
          priority(0),
          tick(tick),
          dependencies(std::move(dependencies)),
          onCompletion(std::move(onCompletion)) {}
};

class TickScheduler : public std::enable_shared_from_this<TickScheduler> {
public:
    TickScheduler();
    ~TickScheduler();

    static std::shared_ptr<TickScheduler> createShared();

    template <typename F, typename... Args>
    auto scheduleTask(unsigned long long tick, bool relative,
                      unsigned retryCount = 0,
                      std::chrono::milliseconds retryInterval = {},
                      std::optional<std::size_t> afterTaskId = {},
                      std::optional<unsigned long long> delay = {},
                      std::optional<std::size_t> timeout = {}, F &&f = {},
                      Args &&...args) -> std::shared_ptr<TickTask> {
        static_assert(std::is_invocable_r_v<void, F, Args...>,
                      "Task function must return void");

        auto effectiveTick = relative ? currentTick.load() + tick : tick;
        if (afterTaskId) {
            std::shared_lock lock(tasksMutex);
            auto it = std::find_if(
                tasks.begin(), tasks.end(),
                [id = *afterTaskId](const std::shared_ptr<TickTask> &task) {
                    return task->id == id;
                });
            if (it != tasks.end()) {
                effectiveTick = (*it)->tick;
                while (++it != tasks.end() && (*it)->tick == effectiveTick) {
                }
            }
        }
        if (delay) {
            effectiveTick += *delay;
        }

        auto taskFunc = [this, f = std::forward<F>(f), args..., retryCount,
                         retryInterval, effectiveTick]() mutable {
            try {
                f(std::forward<Args>(args)...);
            } catch (...) {
                if (retryCount > 0) {
                    scheduleTask(retryInterval.count(), true, retryCount - 1,
                                 retryInterval, {}, {}, {}, f,
                                 std::forward<Args>(args)...);
                } else {
                    throw;  // 如果达到重试次数上限，重新抛出异常
                }
            }
        };

        auto task = std::make_shared<TickTask>(taskFunc, effectiveTick);
        task->id = nextTaskId++;
        task->retryCount = retryCount;
        task->retryInterval = retryInterval;
        if (timeout) {
            task->timeout = *timeout;
            task->timeoutPromise.set_value(true);
        }

        {
            std::unique_lock lock(tasksMutex);
            if (afterTaskId) {
                tasks.insert(
                    std::find_if(tasks.begin(), tasks.end(),
                                 [id = *afterTaskId](
                                     const std::shared_ptr<TickTask> &task) {
                                     return task->id == id;
                                 }),
                    task);
            } else {
                tasks.push_back(task);
            }
        }
        cv.notify_one();
        return task;
    }

    bool cancelTask(std::size_t taskId);
    void delayTask(std::optional<std::size_t> taskId,
                   std::optional<unsigned long long> delay);
    unsigned long long getCurrentTick() const;
    void addDependency(const std::shared_ptr<TickTask> &task,
                       const std::shared_ptr<TickTask> &dependency);
    void setCompletionCallback(const std::shared_ptr<TickTask> &task,
                               std::function<void()> callback);
    void pause();
    void resume();
    void setMaxConcurrentTasks(std::size_t max);
    void setTickLength(std::chrono::milliseconds tickLength);
    void setTickLength(unsigned long long tickLength);
    int getTickLength() const;
    void switchToManualMode();
    void switchToAutoMode();
    void triggerTasks();

    // 新增的功能接口
    bool isPaused() const;
    std::size_t getPendingTaskCount() const;
    std::vector<std::size_t> getPendingTaskIds() const;
    std::optional<std::shared_ptr<TickTask>> getTaskById(
        std::size_t taskId) const;

private:
    std::weak_ptr<TaskPool> pool;                  // 线程池对象
    std::vector<std::shared_ptr<TickTask>> tasks;  // 所有待执行的任务
    mutable std::shared_mutex tasksMutex;          // 任务队列的互斥锁
    std::condition_variable_any cv;  // 条件变量，用于暂停和恢复任务调度器的执行
    std::atomic<unsigned long long> currentTick{0};  // 当前的计划刻
    std::atomic_int tickLength{100};                 // 每个刻长的毫秒数
    std::atomic_bool stop{false};            // 停止任务调度器的标志
    std::atomic_bool paused{false};          // 暂停任务调度器的标志
    std::jthread schedulerThread;            // 任务调度器的线程
    std::atomic_bool manualMode{false};      // 手动模式
    std::atomic<std::size_t> nextTaskId{0};  // 用于生成任务的唯一标识符
    std::unique_ptr<atom::utils::StopWatcher> stopwatch;
    std::atomic<std::size_t> concurrentTasks{0};  // 当前正在运行的任务数
    std::size_t maxTasks{0};  // 最大同时运行的任务数，0 表示没有限制

    void taskSchedulerLoop();
    bool allDependenciesMet(const std::shared_ptr<TickTask> &task) const;
    void stopScheduler();
};

}  // namespace lithium

#endif  // LITHIUM_TASK_TICK_HPP
