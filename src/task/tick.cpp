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

#include "atom/server/global_ptr.hpp"

namespace Lithium
{
    TickScheduler::TickScheduler(size_t threads): currentTick(0), stop(false)
    {
#if __cplusplus >= 202002L
        schedulerThread = std::jthread([this]
                                       { this->taskSchedulerLoop(); });
#else
        schedulerThread = std::thread([this]
                                      { this->taskSchedulerLoop(); });
#endif
        pool = GetPtr<TaskPool>("lithium.task.pool");
    }

    TickScheduler::~TickScheduler()
    {
        stopScheduler();
    }

    std::shared_ptr<TickScheduler> TickScheduler::createShared(size_t threads)
    {
        return std::make_shared<TickScheduler>(threads);
    }

    void TickScheduler::addDependency(const std::shared_ptr<TickTask> &task, const std::shared_ptr<TickTask> &dependency)
    {
        task->dependencies.push_back(dependency);
    }

    void TickScheduler::setCompletionCallback(const std::shared_ptr<TickTask> &task, std::function<void()> callback)
    {
        task->onCompletion = callback;
    }

    void TickScheduler::pause()
    {
        isPaused.store(true);
    }

    void TickScheduler::resume()
    {
        isPaused.store(false);
        cv.notify_all();
    }

    void TickScheduler::taskSchedulerLoop()
    {
        while (!stop.load())
        {
            {
                std::unique_lock<std::mutex> lock(tasksMutex);
                cv.wait(lock, [this]
                        { return stop.load() || !tasks.empty() || isPaused.load(); });

                if (stop.load())
                    break;

                auto it = tasks.begin();
                while (it != tasks.end())
                {
                    auto task = *it;
                    // Check if the task's tick has come and dependencies are met
                    if (task->tick <= currentTick.load() && allDependenciesMet(task))
                    {
                        pool->enqueue([task]()
                                      {
                            task->func();
                            task->completed.store(true);
                            if (task->onCompletion) {
                                task->onCompletion();
                            } });
                        it = tasks.erase(it); // Remove the task from the list
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate a tick
            currentTick++;
        }
    }

    bool TickScheduler::allDependenciesMet(const std::shared_ptr<TickTask> &task)
    {
        for (const auto &dep : task->dependencies)
        {
            if (!dep->completed.load())
            {
                return false;
            }
        }
        return true;
    }

    void TickScheduler::stopScheduler()
    {
        stop.store(true);
        cv.notify_all();
        if (schedulerThread.joinable())
        {
            schedulerThread.join();
        }
    }
}
