/*
 * timer.cpp
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

#include "timer.hpp"

namespace Atom::Async
{
    TimerTask::TimerTask(std::function<void()> func, unsigned int delay, int repeatCount, int priority)
        : m_func(func), m_delay(delay), m_repeatCount(repeatCount), m_priority(priority)
    {
        m_nextExecutionTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(m_delay);
    }

    bool TimerTask::operator<(const TimerTask &other) const
    {
        if (m_priority != other.m_priority)
        {
            return m_priority > other.m_priority;
        }
        else
        {
            return m_nextExecutionTime > other.m_nextExecutionTime;
        }
    }

    void TimerTask::run()
    {
        try
        {
            m_func();
        }
        catch (const std::exception &e)
        {
            std::throw_with_nested(std::runtime_error("Failed to run timer task"));
        }
        if (m_repeatCount > 0)
        {
            --m_repeatCount;
            if (m_repeatCount > 0)
            {
                m_nextExecutionTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(m_delay);
            }
        }
    }

    std::chrono::steady_clock::time_point TimerTask::getNextExecutionTime() const
    {
        return m_nextExecutionTime;
    }

    Timer::Timer()
        : m_stop(false), m_paused(false)
    {
        m_thread = std::thread(&Timer::run, this);
    }

    Timer::~Timer()
    {
        stop();
        if (m_thread.joinable())
        {
            m_thread.join();
        }
    }

    void Timer::cancelAllTasks()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_taskQueue = std::priority_queue<TimerTask>();
        m_cond.notify_all();
    }

    void Timer::pause()
    {
        m_paused = true;
    }

    void Timer::resume()
    {
        m_paused = false;
        m_cond.notify_all();
    }

    void Timer::stop()
    {
        m_stop = true;
        m_cond.notify_all();
    }

    std::chrono::steady_clock::time_point Timer::now() const
    {
        return std::chrono::steady_clock::now();
    }

    void Timer::run()
    {
        while (!m_stop)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            while (!m_stop && m_paused && m_taskQueue.empty())
            {
                m_cond.wait(lock);
            }
            if (m_stop)
            {
                break;
            }
            if (!m_taskQueue.empty())
            {
                TimerTask task = m_taskQueue.top();
                if (std::chrono::steady_clock::now() >= task.getNextExecutionTime())
                {
                    m_taskQueue.pop();
                    lock.unlock();
                    task.run();
                    if (task.m_repeatCount > 0)
                    {
                        std::unique_lock<std::mutex> lock(m_mutex);
                        m_taskQueue.emplace(task.m_func, task.m_delay, task.m_repeatCount, task.m_priority);
                    }
                    if (m_callback)
                    {
                        m_callback();
                    }
                }
                else
                {
                    m_cond.wait_until(lock, task.getNextExecutionTime());
                }
            }
        }
    }

    int Timer::getTaskCount() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_taskQueue.size();
    }
}

/*

// 用法示例
int main()
{
    Timer timer;

    // 可取消的定时器任务
    std::future<void> future = timer.setTimeout([]()
                                                { std::cout << "Timeout!" << std::endl; },
                                                1000);
    timer.setTimeout([]()
                     { std::cout << "This should not be printed!" << std::endl; },
                     2000);
    timer.cancelAllTasks();

    // 任务函数支持返回值
    std::future<int> result = timer.setTimeout([]()
                                               { return 42; },
                                               2000);
    std::cout << "Result: " << result.get() << std::endl;

    // 定时器线程池
    for (int i = 0; i < 5; ++i)
    {
        timer.setInterval([i]()
                          { std::cout << "Interval from thread " << i << "!" << std::endl; },
                          500, 10 - i, i);
    }

    // 暂停定时器任务
    timer.pause();
    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 恢复定时器任务
    timer.resume();

    timer.setTimeout([]() -> void
                     { std::cout << "High priority task!" << std::endl; },
                     1000);
    timer.setTimeout([]() -> void
                     { std::cout << "Low priority task!" << std::endl; },
                     1000);

    // 添加回调函数
    timer.setCallback([]()
                      { std::cout << "Task completed!" << std::endl; });

    std::this_thread::sleep_for(std::chrono::seconds(10));
    timer.stop();

    return 0;
}

*/
