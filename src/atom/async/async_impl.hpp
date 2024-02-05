/*
 * async_impl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: A simple but useful async worker manager

**************************************************/

#ifndef ATOM_ASYNC_ASYNC_IMPL_HPP
#define ATOM_ASYNC_ASYNC_IMPL_HPP

namespace Atom::Async
{
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
} // namespace Atom::Async

#endif