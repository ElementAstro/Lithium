/**
 * @file pool.cpp
 * @brief Specialized task pool for managing tasks.
 * 
 * This file defines a specialized task pool designed to efficiently manage
 * and execute tasks. The pool provides mechanisms for task scheduling, 
 * execution, and resource management, optimizing task processing based on
 * specific criteria or requirements.
 * 
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#include "pool.hpp"

#include "atom/log/loguru.hpp"

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

bool WorkerQueue::empty() const {
    std::shared_lock lock(mutex);
    return queue.empty();
}

TaskPool::TaskPool(size_t threads) : m_defaultThreadCount(threads) {
    if (threads == 0) {
        throw std::invalid_argument("Thread count cannot be zero.");
    }

    m_queues.resize(threads);
    for (size_t i : std::views::iota(0u, threads)) {
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
        for (size_t i : std::views::iota(0u, m_queues.size())) {
            if (m_queues[(t_index + i) % m_queues.size()]->tryPop(task)) {
                break;
            }
        }
        if (!task) {
            std::unique_lock lock(m_conditionMutex);
            m_condition.wait(
                lock, [this, &task] { return m_stop || tryStealing(task); });
        }
        if (task) {
            try {
                task->func();
            } catch (const std::exception& e) {
                // Handle task-specific exceptions
                LOG_F(ERROR, "Exception in task: %s", e.what());
            }
        }
    }
}

bool TaskPool::tryStealing(std::shared_ptr<Task>& task) {
    for (size_t i : std::views::iota(1u, m_queues.size())) {
        if (m_queues[(t_index + i) % m_queues.size()]->trySteal(task)) {
            return true;
        }
    }
    return false;
}

void TaskPool::start(size_t threads) {
    m_queues.resize(threads);
    for (size_t i : std::views::iota(0u, threads)) {
        m_queues[i] = std::make_unique<WorkerQueue>();
        m_workers.emplace_back(&TaskPool::workerThread, this, i);
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

void TaskPool::stopAcceptingTasks() {
    std::unique_lock lock(m_conditionMutex);
    m_acceptTasks = false;
}

}  // namespace lithium
