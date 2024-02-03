/*
 * pool.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Specialized task pool

**************************************************/

#include "pool.hpp"

namespace Lithium
{
    DynamicThreadPool::DynamicThreadPool(size_t threads = std::thread::hardware_concurrency(), size_t maxThreads = std::numeric_limits<size_t>::max(), TaskSchedulingStrategy strategy = TaskSchedulingStrategy::FIFO)
        : m_stop(false), m_sleep(false), m_defaultThreadCount(threads), m_maxThreadCount(maxThreads), m_activeThreads(0), m_schedulingStrategy(strategy)
    {
        assert(threads > 0 && maxThreads >= threads);
        adjustThreadCount(threads);
    }

    DynamicThreadPool::~DynamicThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_stop = true;
        }
        m_condition.notify_all();
        for (std::thread &worker : m_workers)
            worker.join();
    }

    void DynamicThreadPool::sleepFor(std::chrono::milliseconds duration)
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_sleep = true;
        }
        std::this_thread::sleep_for(duration);
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_sleep = false;
        }
    }

    void DynamicThreadPool::adjustThreadCount(size_t newSize)
    {
        newSize = std::min(newSize, m_maxThreadCount);
        while (m_workers.size() < newSize)
        {
            m_workers.emplace_back([this]
                                   {
                for (;;) {
                    std::shared_ptr<Task> task;
                    {
                        std::unique_lock<std::mutex> lock(this->m_queueMutex);
                        this->m_condition.wait(lock, [this] { return this->m_stop || !this->m_tasks.empty(); });
                        if (this->m_stop && this->m_tasks.empty())
                            return;
                        task = std::move(this->m_tasks.top());
                        this->m_tasks.pop();
                        ++m_activeThreads;
                    }
                    task->func();
                    {
                        std::lock_guard<std::mutex> lock(this->m_queueMutex);
                        --m_activeThreads;
                        if (this->shouldReduceThreads()) {
                            break;
                        }
                    }
                } });
        }
    }

    void DynamicThreadPool::wait()
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_condition.wait(lock, [this]
                         { return this->m_tasks.empty(); });
    }

    bool DynamicThreadPool::shouldReduceThreads()
    {
        return m_workers.size() > m_defaultThreadCount && m_tasks.empty();
    }
}
