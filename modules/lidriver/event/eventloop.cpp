/*
 * eventloop.cpp
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

Description: EventLoop

**************************************************/

#include "eventloop.hpp"

void EventTrigger::triggerEvents()
{
    std::lock_guard<std::mutex> lock(mutex);
    while (!eventQueue.empty())
    {
        auto event = eventQueue.top();
        eventQueue.pop();
        event.handler();
    }
}

void EventLoop::start()
{
    if (running)
        return;

    running = true;
    eventLoopThread = std::jthread(&EventLoop::eventLoopThreadFunc, this); // 创建并启动EventLoop的线程
}

void EventLoop::eventLoopThreadFunc()
{
    while (running)
    {
        processEvents();     // 处理事件
        processAsyncTasks(); // 处理异步任务
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void EventLoop::stop()
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!running)
        return;
    running = false;
    eventLoopThread.request_stop();
    eventLoopThread.join(); // 等待EventLoop的线程结束
}

void EventLoop::processEvents()
{
    eventTrigger.triggerEvents();
}

void EventLoop::processAsyncTasks()
{
    std::lock_guard<std::mutex> lock(mutex);

    if (!asyncTasks.empty())
    {
        auto it = asyncTasks.begin();
        while (it != asyncTasks.end())
        {
            try
            {
                if (it->wait_for(std::chrono::seconds(0)) == std::future_status::ready)
                {
                    it->get();
                    it = asyncTasks.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            catch (const std::exception &e)
            {
                // 异常处理
                // std::cout << "Exception caught: " << e.what() << std::endl;
                it = asyncTasks.erase(it);
            }
        }
    }
}