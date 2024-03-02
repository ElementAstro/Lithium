/*
 * tick.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Tick Sheduler, just like Minecraft's

**************************************************/

#include "tick.hpp"

#include "atom/log/loguru.hpp"
#include "atom/server/global_ptr.hpp"
#include "atom/utils/stopwatcher.hpp"


namespace Lithium {
TickScheduler::TickScheduler(size_t threads)
    : currentTick(0), stop(false), tickLength(100) {
#if __cplusplus >= 202002L
    schedulerThread = std::jthread([this] { this->taskSchedulerLoop(); });
#else
    schedulerThread = std::thread([this] { this->taskSchedulerLoop(); });
#endif
    pool = GetPtr<TaskPool>("lithium.task.pool");
    stopwatch = std::make_unique<Atom::Utils::StopWatcher>();
}

TickScheduler::~TickScheduler() { stopScheduler(); }

std::shared_ptr<TickScheduler> TickScheduler::createShared(size_t threads) {
    return std::make_shared<TickScheduler>(threads);
}

bool TickScheduler::cancelTask(std::size_t taskId) {
    std::lock_guard<std::mutex> lock(tasksMutex);
    auto it = std::find_if(tasks.begin(), tasks.end(),
                           [taskId](const std::shared_ptr<TickTask> &task) {
                               return task->id == taskId;
                           });
    if (it != tasks.end()) {
        tasks.erase(it);
        return true;
    }
    return false;
}

void TickScheduler::delayTask(std::optional<std::size_t> taskId,
                              std::optional<unsigned long long> delay) {
    std::lock_guard<std::mutex> lock(tasksMutex);
    if (taskId.has_value()) {
        auto it =
            std::find_if(tasks.begin(), tasks.end(),
                         [id = *taskId](const std::shared_ptr<TickTask> &task) {
                             return task->id == id;
                         });
        if (it != tasks.end()) {
            (*it)->tick += *delay;
        }
    } else {
        for (auto &task : tasks) {
            task->tick += *delay;
        }
    }
}

unsigned long long TickScheduler::getCurrentTick() const {
    return currentTick.load();
}

void TickScheduler::addDependency(const std::shared_ptr<TickTask> &task,
                                  const std::shared_ptr<TickTask> &dependency) {
    task->dependencies.push_back(dependency);
}

void TickScheduler::setCompletionCallback(const std::shared_ptr<TickTask> &task,
                                          std::function<void()> callback) {
    task->onCompletion = callback;
}

void TickScheduler::pause() { isPaused.store(true); }

void TickScheduler::resume() {
    isPaused.store(false);
    cv.notify_all();
}

void TickScheduler::setMaxConcurrentTasks(std::size_t max) { maxTasks = max; }

void TickScheduler::setTickLength(std::chrono::milliseconds tickLength) {
    this->tickLength = tickLength.count();
}

void TickScheduler::setTickLength(unsigned long long tickLength) {
    this->tickLength = tickLength;
}

int TickScheduler::getTickLength() const { return tickLength.load(); }

void TickScheduler::switchToManualMode() { manualMode.store(true); }

void TickScheduler::switchToAutoMode() {
    manualMode.store(false);
    cv.notify_all();  // 从手动模式切换回自动模式时唤醒调度线程
}

void TickScheduler::triggerTasks() {
    if (!manualMode.load()) {
        LOG_F(ERROR, "Scheduler is not in manual mode.");
        return;
    }

    // 手动触发任务执行的逻辑，与 taskSchedulerLoop 中的相似
    std::unique_lock<std::mutex> lock(tasksMutex);
    auto it = tasks.begin();
    while (it != tasks.end()) {
        auto task = *it;
        if (task->tick <= currentTick.load() && allDependenciesMet(task)) {
            pool->enqueue([task]() {
                task->func();
                task->completed.store(true);
                if (task->onCompletion) {
                    task->onCompletion();
                }
            });
            it = tasks.erase(it);  // 移除已触发的任务
        } else {
            ++it;
        }
    }
    currentTick++;  // 手动模式下，每次触发后递增当前时刻
}

void TickScheduler::taskSchedulerLoop() {
    while (!stop.load()) {
        // 记录每个Tick需要的时间
        DLOG_F(INFO, "Tick %llu", currentTick.load());
        stopwatch->start();
        if (manualMode.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(
                tickLength.load()));  // 手动模式下，简单地等待
            stopwatch->stop();
            stopwatch->reset();
            continue;  // 如果处于手动模式，跳过自动执行的逻辑
        }
        {
            std::unique_lock<std::mutex> lock(tasksMutex);
            cv.wait(lock, [this] {
                return stop.load() || !tasks.empty() || isPaused.load();
            });

            if (stop.load())
                break;

            auto it = tasks.begin();
            while (it != tasks.end() &&
                   (maxTasks == 0 || concurrentTasks < maxTasks)) {
                auto task = *it;
                if (task->tick <= currentTick.load() &&
                    allDependenciesMet(task)) {
                    pool->enqueue([this, task]() {
                        task->isRunning.store(true);
                        task->func();
                        task->completed.store(true);
                        if (task->onCompletion) {
                            task->onCompletion();
                        }
                        task->isRunning.store(false);
                        concurrentTasks--;
                    });
                    concurrentTasks++;
                    it = tasks.erase(it);
                } else {
                    ++it;
                }
            }
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(tickLength.load()));  // Simulate a tick
        stopwatch->stop();
        stopwatch->reset();
        DLOG_F(INFO, "Tick %llu took %f ms", currentTick.load(),
               stopwatch->elapsedMilliseconds());
        currentTick++;
    }
}

bool TickScheduler::allDependenciesMet(const std::shared_ptr<TickTask> &task) {
    for (const auto &dep : task->dependencies) {
        if (!dep->completed.load()) {
            return false;
        }
    }
    return true;
}

void TickScheduler::stopScheduler() {
    stop.store(true);
    cv.notify_all();
    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }
}
}  // namespace Lithium
