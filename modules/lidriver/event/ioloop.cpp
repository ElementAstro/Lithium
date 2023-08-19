/*
 * ioloop.cpp
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

Date: 2023-3-29

Description: IOLoop

**************************************************/


#include "ioloop.hpp"

IOLoop::IOLoop() : running(true), loopThread(&IOLoop::run, this) {}

IOLoop::~IOLoop()
{
    stop();
    loopThread.join();
}

void IOLoop::addAsync(const Task &task, int delay)
{
    std::lock_guard<std::mutex> lock(queueMutex);
    TimeTask timeTask{std::chrono::system_clock::now() + std::chrono::milliseconds(delay), task, false};
    tasks.push(timeTask);
    taskCondition.notify_one();
}

void IOLoop::cancelTask(const Task &task)
{
    std::lock_guard<std::mutex> lock(queueMutex);

    std::priority_queue<TimeTask, std::vector<TimeTask>, std::greater<TimeTask>> tempTasks;

    while (!tasks.empty())
    {
        TimeTask currentTask = tasks.top();
        tasks.pop();

        if (currentTask.task.target<void()>() == task.target<void()>())
        {
            currentTask.cancelled = true;
        }

        tempTasks.push(currentTask);
    }

    tasks = std::move(tempTasks);
}

void IOLoop::pause()
{
    running = false;
}

void IOLoop::resume()
{
    running = true;
    taskCondition.notify_one();
}

void IOLoop::stop()
{
    pause();
    taskCondition.notify_one();
}

void IOLoop::run()
{
    while (running)
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        taskCondition.wait(lock, [this]
                           { return !tasks.empty() || !running; });

        if (!running)
        {
            break;
        }

        TimeTask nextTask = tasks.top();
        tasks.pop();

        if (nextTask.cancelled)
        {
            continue;
        }

        lock.unlock();

        try
        {
            nextTask.task();
        }
        catch (const std::exception &ex)
        {
        }
    }
}
