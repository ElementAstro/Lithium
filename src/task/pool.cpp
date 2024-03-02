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

namespace Lithium {
// Initialize static thread_local variables
thread_local WorkerQueue *TaskPool::t_localQueue = nullptr;
thread_local size_t TaskPool::t_index = 0;

bool WorkerQueue::tryPop(std::shared_ptr<Task> &task) {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.empty()) {
        return false;
    }
    task = std::move(queue.front());
    queue.pop_front();
    return true;
}

bool WorkerQueue::trySteal(std::shared_ptr<Task> &task) {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.empty()) {
        return false;
    }
    task = std::move(queue.back());
    queue.pop_back();
    return true;
}

void WorkerQueue::push(std::shared_ptr<Task> task) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.push_front(std::move(task));
}

std::optional<std::shared_ptr<Task>> WorkerQueue::tryPop() {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.empty()) {
        return {};
    }
    auto task = std::move(queue.front());
    queue.pop_front();
    return task;
}

std::optional<std::shared_ptr<Task>> WorkerQueue::trySteal() {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.empty()) {
        return {};
    }
    auto task = std::move(queue.back());
    queue.pop_back();
    return task;
}

TaskPool::TaskPool(size_t threads = std::thread::hardware_concurrency())
    : m_defaultThreadCount(threads) {
    for (size_t i = 0; i < m_defaultThreadCount; ++i) {
        m_queues.emplace_back(std::make_unique<WorkerQueue>());
    }
    start();
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
        if (!task && !tryStealing(task)) {
            std::unique_lock<std::mutex> lock(m_conditionMutex);
            m_condition.wait(
                lock, [this, &task] { return m_stop || tryStealing(task); });
            if (task) {
                task->func();
            }
        } else if (task) {
            task->func();
        }
    }
}

bool TaskPool::tryStealing(std::shared_ptr<Task> &task) {
    for (size_t i = 0; i < m_queues.size(); ++i) {
        if (m_queues[(t_index + i + 1) % m_queues.size()]->trySteal(task)) {
            return true;
        }
    }
    return false;
}

void TaskPool::start() {
    for (size_t i = 0; i < m_defaultThreadCount; ++i) {
        m_workers.emplace_back([this, i] { workerThread(i); });
    }
}

void TaskPool::stop() {
    m_stop = true;
    m_condition.notify_all();
    for (auto &worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}
}  // namespace Lithium
