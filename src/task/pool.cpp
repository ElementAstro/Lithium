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

namespace lithium {

// Initialize static thread_local variables
thread_local WorkerQueue* TaskPool::t_localQueue = nullptr;
thread_local size_t TaskPool::t_index = 0;

bool WorkerQueue::tryPop(std::shared_ptr<Task>& task) {
    std::unique_lock lock(mutex);
    if (queue.empty()) {
        return false;
    }
    task = std::move(queue.front());
    queue.pop_front();
    return true;
}

bool WorkerQueue::trySteal(std::shared_ptr<Task>& task) {
    std::unique_lock lock(mutex);
    if (queue.empty()) {
        return false;
    }
    task = std::move(queue.back());
    queue.pop_back();
    return true;
}

void WorkerQueue::push(std::shared_ptr<Task> task) {
    std::unique_lock lock(mutex);
    queue.push_front(std::move(task));
}

TaskPool::TaskPool(size_t threads) : m_defaultThreadCount(threads) {
    m_queues.resize(threads);
    for (size_t i = 0; i < threads; ++i) {
        m_queues[i] = std::make_unique<WorkerQueue>();
    }
    start(threads);
}

TaskPool::~TaskPool() { stop(); }

std::shared_ptr<TaskPool> TaskPool::createShared(size_t threads) {
    return std::make_shared<TaskPool>(threads);
}

void TaskPool::workerThread(size_t index) {
    t_index = index;
    t_localQueue = m_queues[t_index].get();

    while (!m_stop) {
        std::shared_ptr<Task> task;
        for (size_t i = 0; i < m_queues.size(); ++i) {
            if (m_queues[(t_index + i) % m_queues.size()]->tryPop(task)) {
                break;
            }
        }
        if (!task) {
            std::unique_lock lock(m_conditionMutex);
            m_condition.wait_for(
                lock, std::chrono::milliseconds(1),
                [this, &task] { return m_stop || tryStealing(task); });
        }
        if (task) {
            task->func();
        }
    }
}

bool TaskPool::tryStealing(std::shared_ptr<Task>& task) {
    for (size_t i = 0; i < m_queues.size(); ++i) {
        if (m_queues[(t_index + i + 1) % m_queues.size()]->trySteal(task)) {
            return true;
        }
    }
    return false;
}

void TaskPool::start(size_t threads) {
    m_queues.resize(threads);
    for (size_t i = 0; i < threads; ++i) {
        m_queues[i] = std::make_unique<WorkerQueue>();
        m_workers.emplace_back([this, i] { workerThread(i); });
    }
}

void TaskPool::stop() {
    m_stop = true;
    m_acceptTasks = false;
    m_condition.notify_all();
    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_workers.clear();
    m_queues.clear();
}

void TaskPool::resize(size_t newThreadCount) {
    {
        std::unique_lock lock(m_conditionMutex);
        m_acceptTasks = false;
    }
    stop();
    {
        std::unique_lock lock(m_conditionMutex);
        m_stop = false;
        m_acceptTasks = true;
    }
    start(newThreadCount);
    m_defaultThreadCount = newThreadCount;
}

size_t TaskPool::getThreadCount() const { return m_defaultThreadCount; }

}  // namespace lithium