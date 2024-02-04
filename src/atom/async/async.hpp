/*
 * async.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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
#include <type_traits>

#include "atom/utils/exception.hpp"

namespace Atom::Async
{
    /**
     * @brief Class for performing asynchronous tasks.
     *
     * This class allows you to start a task asynchronously and get the result when it's done.
     * It also provides functionality to cancel the task, check if it's done or active, validate the result,
     * set a callback function, and set a timeout.
     *
     * @tparam ResultType The type of the result returned by the task.
     */
    template <typename ResultType>
    class AsyncWorker
    {
    public:
        /**
         * @brief Starts the task asynchronously.
         *
         * @tparam Func The type of the function to be executed asynchronously.
         * @tparam Args The types of the arguments to be passed to the function.
         * @param func The function to be executed asynchronously.
         * @param args The arguments to be passed to the function.
         */
        template <typename Func, typename... Args>
        void StartAsync(Func &&func, Args &&...args);

        /**
         * @brief Gets the result of the task.
         *
         * @throw std::runtime_error if the task is not valid.
         * @return The result of the task.
         */
        ResultType GetResult();

        /**
         * @brief Cancels the task.
         *
         * If the task is valid, this function waits for the task to complete.
         */
        void Cancel();

        /**
         * @brief Checks if the task is done.
         *
         * @return True if the task is done, false otherwise.
         */
        bool IsDone() const;

        /**
         * @brief Checks if the task is active.
         *
         * @return True if the task is active, false otherwise.
         */
        bool IsActive() const;

        /**
         * @brief Validates the result of the task using a validator function.
         *
         * @param validator The function used to validate the result.
         * @return True if the result is valid, false otherwise.
         */
        bool Validate(std::function<bool(ResultType)> validator);

        /**
         * @brief Sets a callback function to be called when the task is done.
         *
         * @param callback The callback function to be set.
         */
        void SetCallback(std::function<void(ResultType)> callback);

        /**
         * @brief Sets a timeout for the task.
         *
         * @param timeout The timeout duration.
         */
        void SetTimeout(std::chrono::seconds timeout);

        /**
         * @brief Waits for the task to complete.
         *
         * If a timeout is set, this function waits until the task is done or the timeout is reached.
         * If a callback function is set and the task is done, the callback function is called with the result.
         */
        void WaitForCompletion();

    private:
        std::future<ResultType> task_;             ///< The future representing the asynchronous task.
        std::function<void(ResultType)> callback_; ///< The callback function to be called when the task is done.
        std::chrono::seconds timeout_{0};          ///< The timeout duration for the task.
    };

    /**
     * @brief Class for managing multiple AsyncWorker instances.
     *
     * This class provides functionality to create and manage multiple AsyncWorker instances.
     *
     * @tparam ResultType The type of the result returned by the tasks managed by this class.
     */
    template <typename ResultType>
    class AsyncWorkerManager
    {
    public:
        /**
         * @brief Default constructor.
         */
        AsyncWorkerManager() = default;

        /**
         * @brief Creates a new AsyncWorker instance and starts the task asynchronously.
         *
         * @tparam Func The type of the function to be executed asynchronously.
         * @tparam Args The types of the arguments to be passed to the function.
         * @param func The function to be executed asynchronously.
         * @param args The arguments to be passed to the function.
         * @return A shared pointer to the created AsyncWorker instance.
         */
        template <typename Func, typename... Args>
        std::shared_ptr<AsyncWorker<ResultType>> CreateWorker(Func &&func, Args &&...args);

        /**
         * @brief Cancels all the managed tasks.
         */
        void CancelAll();

        /**
         * @brief Checks if all the managed tasks are done.
         *
         * @return True if all tasks are done, false otherwise.
         */
        bool AllDone() const;

        /**
         * @brief Waits for all the managed tasks to complete.
         */
        void WaitForAll();

        /**
         * @brief Checks if a specific task is done.
         *
         * @param worker The AsyncWorker instance to check.
         * @return True if the task is done, false otherwise.
         */
        bool IsDone(std::shared_ptr<AsyncWorker<ResultType>> worker) const;

        /**
         * @brief Cancels a specific task.
         *
         * @param worker The AsyncWorker instance to cancel.
         */
        void Cancel(std::shared_ptr<AsyncWorker<ResultType>> worker);

    private:
        std::vector<std::shared_ptr<AsyncWorker<ResultType>>> workers_; ///< The list of managed AsyncWorker instances.
    };

    template <typename ResultType>
    template <typename Func, typename... Args>
    void AsyncWorker<ResultType>::StartAsync(Func &&func, Args &&...args)
    {
        static_assert(std::is_invocable_r_v<ResultType, Func, Args...>, "Function must return a result");
        task_ = std::async(std::launch::async, std::forward<Func>(func), std::forward<Args>(args)...);
    }

    template <typename ResultType>
    ResultType AsyncWorker<ResultType>::GetResult()
    {
        if (!task_.valid())
        {
            throw Utils::Exception::InvalidArgument_Error("Task is not valid");
        }
        return task_.get();
    }

    template <typename ResultType>
    void AsyncWorker<ResultType>::Cancel()
    {
        if (task_.valid())
        {
            task_.wait(); // 等待任务完成
        }
    }

    template <typename ResultType>
    bool AsyncWorker<ResultType>::IsDone() const
    {
        return task_.valid() && (task_.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
    }

    template <typename ResultType>
    bool AsyncWorker<ResultType>::IsActive() const
    {
        return task_.valid() && (task_.wait_for(std::chrono::seconds(0)) == std::future_status::timeout);
    }

    template <typename ResultType>
    bool AsyncWorker<ResultType>::Validate(std::function<bool(ResultType)> validator)
    {
        if (!IsDone())
        {
        }
        ResultType result = GetResult();
        return validator(result);
    }

    template <typename ResultType>
    void AsyncWorker<ResultType>::SetCallback(std::function<void(ResultType)> callback)
    {
        callback_ = callback;
    }

    template <typename ResultType>
    void AsyncWorker<ResultType>::SetTimeout(std::chrono::seconds timeout)
    {
        timeout_ = timeout;
    }

    template <typename ResultType>
    void AsyncWorker<ResultType>::WaitForCompletion()
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

    template <typename ResultType>
    template <typename Func, typename... Args>
    std::shared_ptr<AsyncWorker<ResultType>> AsyncWorkerManager<ResultType>::CreateWorker(Func &&func, Args &&...args)
    {
        auto worker = std::make_shared<AsyncWorker<ResultType>>();
        workers_.push_back(worker);
        worker->StartAsync(std::forward<Func>(func), std::forward<Args>(args)...);
        return worker;
    }

    template <typename ResultType>
    void AsyncWorkerManager<ResultType>::CancelAll()
    {
        for (auto &worker : workers_)
        {
            worker->Cancel();
        }
    }

    template <typename ResultType>
    bool AsyncWorkerManager<ResultType>::AllDone() const
    {
        return std::all_of(workers_.begin(), workers_.end(), [](const auto &worker)
                           { return worker->IsDone(); });
    }

    template <typename ResultType>
    void AsyncWorkerManager<ResultType>::WaitForAll()
    {
        while (!AllDone())
        {
            // 可以加入一些逻辑来检查任务完成情况
        }
    }

    template <typename ResultType>
    bool AsyncWorkerManager<ResultType>::IsDone(std::shared_ptr<AsyncWorker<ResultType>> worker) const
    {
        return worker->IsDone();
    }

    template <typename ResultType>
    void AsyncWorkerManager<ResultType>::Cancel(std::shared_ptr<AsyncWorker<ResultType>> worker)
    {
        worker->Cancel();
    }
}

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
