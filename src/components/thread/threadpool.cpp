/*
 * threadpool.cpp
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

Date: 2023-7-2

Description: Simple Thread Pool

**************************************************/

#include "threadpool.hpp"

namespace OpenAPT::Thread
{
    ThreadPool::ThreadPool(size_t num_threads)
        : stop_(false)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers_.emplace_back([this]()
                                  {
            while (true)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
                    if (stop_ && tasks_.empty())
                    {
                        return;
                    }
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
                task();
            } });
        }
    }

    ThreadPool::~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto &worker : workers_)
        {
            worker.join();
        }
    }
    
    void ThreadPool::wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this]()
                        { return tasks_.empty(); });
    }
}
