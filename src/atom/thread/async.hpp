/*
 * async.hpp
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

Date: 2023-11-10

Description: A simple but useful async worker manager

**************************************************/

#pragma once

#include <future>
#include <chrono>
#include <thread>
#include <functional>
#include <stdexcept>
#include <vector>
#include <memory>
#include <algorithm>

template <typename ResultType>
class AsyncWorker
{
public:
    template <typename Func, typename... Args>
    void StartAsync(Func &&func, Args &&...args)
    {
        task_ = std::async(std::launch::async, std::forward<Func>(func), std::forward<Args>(args)...);
    }

    ResultType GetResult()
    {
        if (!task_.valid())
        {
            throw std::runtime_error("Task is not valid");
        }
        return task_.get();
    }

    void Cancel()
    {
        if (task_.valid())
        {
            task_.wait(); // 等待任务完成
        }
    }

    bool IsDone() const
    {
        return task_.valid() && (task_.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
    }

    bool IsActive() const
    {
        return task_.valid() && (task_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout);
    }

    bool Validate(std::function<bool(ResultType)> validator)
    {
        if (!IsDone())
        {
        }
        ResultType result = GetResult();
        return validator(result);
    }

    void SetCallback(std::function<void(ResultType)> callback)
    {
        callback_ = callback;
    }

    void SetTimeout(std::chrono::seconds timeout)
    {
        timeout_ = timeout;
    }

    void WaitForCompletion()
    {
        if (timeout_ != std::chrono::seconds(0))
        {
            auto start_time = std::chrono::steady_clock::now();
            while (!IsDone())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (std::chrono::steady_clock::now() - start_time > timeout_)
                {
                    Cancel();
                    break;
                }
            }
        }
        else
        {
            while (!IsDone())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        if (callback_ && IsDone())
        {
            callback_(GetResult());
        }
    }

private:
    std::future<ResultType> task_;
    std::function<void(ResultType)> callback_;
    std::chrono::seconds timeout_{0};
};

template <typename ResultType>
class AsyncWorkerManager
{
public:
    AsyncWorkerManager() = default;

    template <typename Func, typename... Args>
    std::shared_ptr<AsyncWorker<ResultType>> CreateWorker(Func &&func, Args &&...args)
    {
        auto worker = std::make_shared<AsyncWorker<ResultType>>();
        workers_.push_back(worker);
        worker->StartAsync(std::forward<Func>(func), std::forward<Args>(args)...);
        return worker;
    }

    void CancelAll()
    {
        for (auto &worker : workers_)
        {
            worker->Cancel();
        }
    }

    bool AllDone() const
    {
        return std::all_of(workers_.begin(), workers_.end(), [](const auto &worker)
                           { return worker->IsDone(); });
    }

    void WaitForAll()
    {
        while (!AllDone())
        {
            // 可以加入一些逻辑来检查任务完成情况
        }
    }

    bool IsDone(std::shared_ptr<AsyncWorker<ResultType>> worker) const
    {
        return worker->IsDone();
    }

    void Cancel(std::shared_ptr<AsyncWorker<ResultType>> worker)
    {
        worker->Cancel();
    }

private:
    std::vector<std::shared_ptr<AsyncWorker<ResultType>>> workers_;
};

/*
int main()
{
    AsyncWorkerManager<int> manager;

    auto worker1 = manager.CreateWorker([]()
                                        {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        return 42; });

    auto worker2 = manager.CreateWorker([]()
                                        {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 100; });

    manager.WaitForAll();

    std::cout << "All workers are done." << std::endl;

    if (manager.IsDone(worker1))
    {
        std::cout << "Worker 1 is done." << std::endl;
    }

    manager.Cancel(worker2);
    std::cout << "Worker 2 is cancelled." << std::endl;

    return 0;
}
*/
