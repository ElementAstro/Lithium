/**
 * @file tick.cpp
 * @brief Tick scheduler similar to Minecraft's.
 *
 * This file defines a tick scheduler that operates similarly to the tick
 * system used in Minecraft. The scheduler manages time-based updates or
 * events that occur at regular intervals, facilitating the execution of
 * tasks or processes in a periodic manner.
 *
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#include "tick.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/stopwatcher.hpp"

namespace lithium {

TickScheduler::TickScheduler() {
    schedulerThread = std::jthread([this] { this->taskSchedulerLoop(); });
    pool = GetWeakPtr<TaskPool>("lithium.task.pool");
    stopwatch = std::make_unique<atom::utils::StopWatcher>();
}

TickScheduler::~TickScheduler() { stopScheduler(); }

std::shared_ptr<TickScheduler> TickScheduler::createShared() {
    return std::make_shared<TickScheduler>();
}

bool TickScheduler::cancelTask(std::size_t taskId) {
    std::unique_lock lock(tasksMutex);
    auto it = std::find_if(tasks.begin(), tasks.end(),
                           [taskId](const std::shared_ptr<TickTask>& task) {
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
    std::unique_lock lock(tasksMutex);  // 加锁保护任务队列
    if (delay.has_value()) {            // 确保 delay 有效
        if (taskId.has_value()) {  // 如果提供了 taskId，则仅延迟特定任务
            auto it = std::find_if(
                tasks.begin(), tasks.end(),
                [id = *taskId](const std::shared_ptr<TickTask>& task) {
                    return task->id == id;
                });
            if (it != tasks.end()) {
                (*it)->tick += *delay;  // 延迟该任务的 tick
            }
        } else {  // 如果没有提供 taskId，则延迟所有任务
            for (auto& task : tasks) {
                task->tick += *delay;
            }
        }
    }
}

unsigned long long TickScheduler::getCurrentTick() const {
    return currentTick.load();
}

void TickScheduler::addDependency(const std::shared_ptr<TickTask>& task,
                                  const std::shared_ptr<TickTask>& dependency) {
    std::unique_lock lock(tasksMutex);
    task->dependencies.push_back(dependency);
}

void TickScheduler::setCompletionCallback(const std::shared_ptr<TickTask>& task,
                                          std::function<void()> callback) {
    task->onCompletion = std::move(callback);
}

void TickScheduler::pause() { paused.store(true); }

void TickScheduler::resume() {
    paused.store(false);
    cv.notify_all();
}

void TickScheduler::setMaxConcurrentTasks(std::size_t max) { maxTasks = max; }

void TickScheduler::setTickLength(std::chrono::milliseconds tickLength) {
    this->tickLength.store(static_cast<int>(tickLength.count()));
}

void TickScheduler::setTickLength(unsigned long long tickLength) {
    this->tickLength.store(static_cast<int>(tickLength));
}

int TickScheduler::getTickLength() const { return tickLength.load(); }

void TickScheduler::switchToManualMode() { manualMode.store(true); }

void TickScheduler::switchToAutoMode() {
    manualMode.store(false);
    cv.notify_all();
}

void TickScheduler::triggerTasks() {
    if (manualMode) {
        cv.notify_all();
    }
}

// 新增的功能接口
bool TickScheduler::isPaused() const { return paused.load(); }

std::size_t TickScheduler::getPendingTaskCount() const {
    std::shared_lock lock(tasksMutex);
    return tasks.size();
}

std::vector<std::size_t> TickScheduler::getPendingTaskIds() const {
    std::shared_lock lock(tasksMutex);
    std::vector<std::size_t> ids;
    ids.reserve(tasks.size());
    for (const auto& task : tasks) {
        ids.push_back(task->id);
    }
    return ids;
}

std::optional<std::shared_ptr<TickTask>> TickScheduler::getTaskById(
    std::size_t taskId) const {
    std::shared_lock lock(tasksMutex);
    auto it = std::find_if(tasks.begin(), tasks.end(),
                           [taskId](const std::shared_ptr<TickTask>& task) {
                               return task->id == taskId;
                           });
    if (it != tasks.end()) {
        return *it;
    }
    return std::nullopt;
}

void TickScheduler::taskSchedulerLoop() {
    while (!stop.load()) {
        std::unique_lock lock(tasksMutex);
        if (paused.load()) {
            cv.wait(lock, [this] { return !paused.load() || stop.load(); });
        }

        if (stop.load()) {
            break;
        }

        currentTick++;

        auto now = std::chrono::steady_clock::now();
        tasks.erase(
            std::remove_if(tasks.begin(), tasks.end(),
                           [this, now](const std::shared_ptr<TickTask>& task) {
                               if (task->tick <= currentTick.load()) {
                                   if (task->isRunning.load() ||
                                       task->completed.load()) {
                                       return true;
                                   }

                                   if (allDependenciesMet(task)) {
                                       task->isRunning.store(true);
                                       if (auto poolPtr = pool.lock()) {
                                           poolPtr->enqueue([task, now]() {
                                               task->func();
                                               task->completed.store(true);
                                               task->isRunning.store(false);
                                               if (task->onCompletion) {
                                                   task->onCompletion();
                                               }
                                           });
                                       } else {
                                           task->func();
                                           task->completed.store(true);
                                           if (task->onCompletion) {
                                               task->onCompletion();
                                           }
                                       }
                                       return true;
                                   }
                               }
                               return false;
                           }),
            tasks.end());

        if (!manualMode.load()) {
            cv.wait_for(lock, std::chrono::milliseconds(tickLength.load()));
        } else {
            cv.wait(lock);
        }
    }
}

auto TickScheduler::allDependenciesMet(
    const std::shared_ptr<TickTask>& task) const -> bool {
    return std::all_of(task->dependencies.begin(), task->dependencies.end(),
                       [](const std::shared_ptr<TickTask>& dep) {
                           return dep->completed.load();
                       });
}

void TickScheduler::stopScheduler() {
    stop.store(true);
    cv.notify_all();
    if (schedulerThread.joinable()) {
        schedulerThread.join();
    }
}
}  // namespace lithium
