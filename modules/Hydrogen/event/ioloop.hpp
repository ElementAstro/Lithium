/*
 * ioloop.hpp
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

#ifndef IO_LOOP_H
#define IO_LOOP_H

#include <functional>
#include <queue>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>

class IOLoop
{
public:
    using Task = std::function<void()>;

    IOLoop();

    ~IOLoop();

    void addAsync(const Task &task, int delay = 0);

    void cancelTask(const Task &task);

    void pause();

    void resume();

    void stop();

private:
    struct TimeTask
    {
        std::chrono::system_clock::time_point time;
        Task task;
        bool cancelled;

        bool operator>(const TimeTask &other) const
        {
            return time > other.time;
        }
    };

    void run();

    std::priority_queue<TimeTask, std::vector<TimeTask>, std::greater<TimeTask>> tasks;
    std::mutex queueMutex;
    std::condition_variable taskCondition;
    std::atomic_bool running;
    std::thread loopThread;
};

#endif // IO_LOOP_H
