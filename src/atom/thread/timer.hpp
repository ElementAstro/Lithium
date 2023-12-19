/*
 * timer.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-14

Description: Timer class for C++

**************************************************/

#pragma once

#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class TimerTask
{
public:
    TimerTask(std::function<void()> func, unsigned int delay, int repeatCount, int priority);

    bool operator<(const TimerTask &other) const;

    void run();

    std::chrono::steady_clock::time_point getNextExecutionTime() const;

    std::function<void()> m_func;
    unsigned int m_delay;
    int m_repeatCount;
    int m_priority;
    std::chrono::steady_clock::time_point m_nextExecutionTime;
};

class Timer
{
public:
    Timer();

    ~Timer();

    template <typename Function, typename... Args>
    std::future<typename std::result_of<Function(Args...)>::type> setTimeout(Function &&func, unsigned int delay, Args &&...args);

    template <typename Function, typename... Args>
    void setInterval(Function &&func, unsigned int interval, int repeatCount, int priority, Args &&...args);

    void cancelAllTasks();

    void pause();

    void resume();

    void stop();

    template <typename Function>
    void setCallback(Function &&func);

private:
    template <typename Function, typename... Args>
    std::future<typename std::result_of<Function(Args...)>::type> addTask(Function &&func, unsigned int delay, int repeatCount, int priority, Args &&...args);

    void run();

private:
#if _cplusplus >= 202203L
    std::jthread m_thread;
#else
    std::thread m_thread;
#endif
    std::priority_queue<TimerTask> m_taskQueue;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    std::function<void()> m_callback;
    bool m_stop;
    bool m_paused;
};

template <typename Function, typename... Args>
std::future<typename std::result_of<Function(Args...)>::type> Timer::setTimeout(Function &&func, unsigned int delay, Args &&...args)
{
    using ReturnType = typename std::result_of<Function(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    std::unique_lock<std::mutex> lock(m_mutex);
    m_taskQueue.emplace([task]()
                        { (*task)(); },
                        delay, 1, 0);
    m_cond.notify_all();
    return result;
}

template <typename Function, typename... Args>
void Timer::setInterval(Function &&func, unsigned int interval, int repeatCount, int priority, Args &&...args)
{
    addTask(std::forward<Function>(func), interval, repeatCount, priority, std::forward<Args>(args)...);
}

template <typename Function, typename... Args>
std::future<typename std::result_of<Function(Args...)>::type> Timer::addTask(Function &&func, unsigned int delay, int repeatCount, int priority, Args &&...args)
{
    using ReturnType = typename std::result_of<Function(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    std::unique_lock<std::mutex> lock(m_mutex);
    m_taskQueue.emplace([task]()
                        { (*task)(); },
                        delay, repeatCount, priority);
    m_cond.notify_all();
    return result;
}

template <typename Function>
void Timer::setCallback(Function &&func)
{
    m_callback = std::forward<Function>(func);
}