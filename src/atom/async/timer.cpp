/*
 * timer.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-14

Description: Timer class for C++

**************************************************/

#include "timer.hpp"
#include <cstddef>
#include "error/exception.hpp"

namespace atom::async {
TimerTask::TimerTask(std::function<void()> func, unsigned int delay,
                     int repeatCount, int priority)
    : m_func(func),
      m_delay(delay),
      m_repeatCount(repeatCount),
      m_priority(priority) {
    m_nextExecutionTime =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(m_delay);
}

auto TimerTask::operator<(const TimerTask &other) const -> bool {
    if (m_priority != other.m_priority) {
        return m_priority > other.m_priority;
    }
    return m_nextExecutionTime > other.m_nextExecutionTime;
}

void TimerTask::run() {
    try {
        m_func();
    } catch (const std::exception &e) {
        THROW_RUNTIME_ERROR("Failed to run timer task: ", e.what());
    }
    if (m_repeatCount > 0) {
        --m_repeatCount;
        if (m_repeatCount > 0) {
            m_nextExecutionTime = std::chrono::steady_clock::now() +
                                  std::chrono::milliseconds(m_delay);
        }
    }
}

std::chrono::steady_clock::time_point TimerTask::getNextExecutionTime() const {
    return m_nextExecutionTime;
}

Timer::Timer() : m_stop(false), m_paused(false) {
    m_thread = std::thread(&Timer::run, this);
}

Timer::~Timer() {
    stop();
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void Timer::cancelAllTasks() {
    std::unique_lock lock(m_mutex);
    m_taskQueue = std::priority_queue<TimerTask>();
    m_cond.notify_all();
}

void Timer::pause() { m_paused = true; }

void Timer::resume() {
    m_paused = false;
    m_cond.notify_all();
}

void Timer::stop() {
    m_stop = true;
    m_cond.notify_all();
}

auto Timer::now() const -> std::chrono::steady_clock::time_point {
    return std::chrono::steady_clock::now();
}

void Timer::run() {
    while (!m_stop) {
        std::unique_lock lock(m_mutex);
        while (!m_stop && m_paused && m_taskQueue.empty()) {
            m_cond.wait(lock, [&]() {
                return m_stop || !m_paused || !m_taskQueue.empty();
            });
        }
        if (m_stop) {
            break;
        }
        if (!m_taskQueue.empty()) {
            TimerTask task = m_taskQueue.top();
            if (std::chrono::steady_clock::now() >=
                task.getNextExecutionTime()) {
                m_taskQueue.pop();
                lock.unlock();
                task.run();
                if (task.m_repeatCount > 0) {
                    std::unique_lock lock(m_mutex);
                    m_taskQueue.emplace(task.m_func, task.m_delay,
                                        task.m_repeatCount, task.m_priority);
                }
                if (m_callback) {
                    m_callback();
                }
            } else {
                m_cond.wait_until(lock, task.getNextExecutionTime());
            }
        }
    }
}

auto Timer::getTaskCount() const -> size_t {
    std::unique_lock lock(m_mutex);
    return m_taskQueue.size();
}

void Timer::wait() {
    std::unique_lock lock(m_mutex);
    m_cond.wait(lock, [&]() { return m_taskQueue.empty(); });
}
}  // namespace atom::async
