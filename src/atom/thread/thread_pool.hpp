/*
 * thread_pool.hpp
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

Date: 2023-3-27

Description: Thread Pool

**************************************************/

#pragma once

#include <atomic>
#include <barrier>
#include <concepts>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <semaphore>
#include <thread>
#include <type_traits>
#include <algorithm>
#include <mutex>
#include <optional>
#ifdef __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

template <typename Lock>
concept is_lockable = requires(Lock &&lock) {
    lock.lock();
    lock.unlock();
    {
        lock.try_lock()
    } -> std::convertible_to<bool>;
};

template <typename T, typename Lock = std::mutex>
    requires is_lockable<Lock>
class thread_safe_queue
{
public:
    using value_type = T;
    using size_type = typename std::deque<T>::size_type;

    thread_safe_queue() = default;

    void push_back(T &&value)
    {
        std::scoped_lock lock(mutex_);
        data_.push_back(std::forward<T>(value));
    }

    void push_front(T &&value)
    {
        std::scoped_lock lock(mutex_);
        data_.push_front(std::forward<T>(value));
    }

    [[nodiscard]] bool empty() const
    {
        std::scoped_lock lock(mutex_);
        return data_.empty();
    }

    [[nodiscard]] std::optional<T> pop_front()
    {
        std::scoped_lock lock(mutex_);
        if (data_.empty())
            return std::nullopt;

        auto front = std::move(data_.front());
        data_.pop_front();
        return front;
    }

    [[nodiscard]] std::optional<T> pop_back()
    {
        std::scoped_lock lock(mutex_);
        if (data_.empty())
            return std::nullopt;

        auto back = std::move(data_.back());
        data_.pop_back();
        return back;
    }

    [[nodiscard]] std::optional<T> steal()
    {
        std::scoped_lock lock(mutex_);
        if (data_.empty())
            return std::nullopt;

        auto back = std::move(data_.back());
        data_.pop_back();
        return back;
    }

    void rotate_to_front(const T &item)
    {
        std::scoped_lock lock(mutex_);
        auto iter = std::find(data_.begin(), data_.end(), item);

        if (iter != data_.end())
        {
            std::ignore = data_.erase(iter);
        }

        data_.push_front(item);
    }

    [[nodiscard]] std::optional<T> copy_front_and_rotate_to_back()
    {
        std::scoped_lock lock(mutex_);

        if (data_.empty())
            return std::nullopt;

        auto front = data_.front();
        data_.pop_front();

        data_.push_back(front);

        return front;
    }

private:
    std::deque<T> data_{};
    mutable Lock mutex_{};
};

namespace details
{

#ifdef __cpp_lib_move_only_function
    using default_function_type = std::move_only_function<void()>;
#else
    using default_function_type = std::function<void()>;
#endif
} // namespace details

template <typename FunctionType = details::default_function_type,
          typename ThreadType = std::jthread>
    requires std::invocable<FunctionType> &&
             std::is_same_v<void, std::invoke_result_t<FunctionType>>
class thread_pool
{
public:
    explicit thread_pool(
        const unsigned int &number_of_threads = std::thread::hardware_concurrency())
        : tasks_(number_of_threads)
    {
        std::size_t current_id = 0;
        for (std::size_t i = 0; i < number_of_threads; ++i)
        {
            priority_queue_.push_back(size_t(current_id));
            try
            {
                threads_.emplace_back([&, id = current_id](const std::stop_token &stop_tok)
                                      {
                        do {
                            // wait until signaled
                            tasks_[id].signal.acquire();

                            do {
                                // invoke the task
                                while (auto task = tasks_[id].tasks.pop_front()) {
                                    try {
                                        pending_tasks_.fetch_sub(1, std::memory_order_release);
                                        std::invoke(std::move(task.value()));
                                    } catch (...) {
                                    }
                                }

                                // try to steal a task
                                for (std::size_t j = 1; j < tasks_.size(); ++j) {
                                    const std::size_t index = (id + j) % tasks_.size();
                                    if (auto task = tasks_[index].tasks.steal()) {
                                        // steal a task
                                        pending_tasks_.fetch_sub(1, std::memory_order_release);
                                        std::invoke(std::move(task.value()));
                                        // stop stealing once we have invoked a stolen task
                                        break;
                                    }
                                }

                            } while (pending_tasks_.load(std::memory_order_acquire) > 0);

                            priority_queue_.rotate_to_front(id);

                        } while (!stop_tok.stop_requested()); });
                // increment the thread id
                ++current_id;
            }
            catch (...)
            {
                // catch all

                // remove one item from the tasks
                tasks_.pop_back();

                // remove our thread from the priority queue
                std::ignore = priority_queue_.pop_back();
            }
        }
    }

    ~thread_pool()
    {
        // stop all threads
        for (std::size_t i = 0; i < threads_.size(); ++i)
        {
            threads_[i].request_stop();
            tasks_[i].signal.release();
            threads_[i].join();
        }
    }

    /// thread pool is non-copyable
    thread_pool(const thread_pool &) = delete;
    thread_pool &operator=(const thread_pool &) = delete;

    /**
     * @brief Enqueue a task into the thread pool that returns a result.
     * @details Note that task execution begins once the task is enqueued.
     * @tparam Function An invokable type.
     * @tparam Args Argument parameter pack
     * @tparam ReturnType The return type of the Function
     * @param f The callable function
     * @param args The parameters that will be passed (copied) to the function.
     * @return A std::future<ReturnType> that can be used to retrieve the returned value.
     */
    template <typename Function, typename... Args,
              typename ReturnType = std::invoke_result_t<Function &&, Args &&...>>
        requires std::invocable<Function, Args...>
    [[nodiscard]] std::future<ReturnType> enqueue(Function f, Args... args)
    {
#if __cpp_lib_move_only_function
        // we can do this in C++23 because we now have support for move only functions
        std::promise<ReturnType> promise;
        auto future = promise.get_future();
        auto task = [func = std::move(f), ... largs = std::move(args),
                     promise = std::move(promise)]() mutable
        {
            try
            {
                if constexpr (std::is_same_v<ReturnType, void>)
                {
                    func(largs...);
                    promise.set_value();
                }
                else
                {
                    promise.set_value(func(largs...));
                }
            }
            catch (...)
            {
                promise.set_exception(std::current_exception());
            }
        };
        enqueue_task(std::move(task));
        return future;
#else
        /*
         * use shared promise here so that we don't break the promise later (until C++23)
         *
         * with C++23 we can do the following:
         *
         * std::promise<ReturnType> promise;
         * auto future = promise.get_future();
         * auto task = [func = std::move(f), ...largs = std::move(args),
                          promise = std::move(promise)]() mutable {...};
         */
        auto shared_promise = std::make_shared<std::promise<ReturnType>>();
        auto task = [func = std::move(f), ... largs = std::move(args),
                     promise = shared_promise]()
        {
            try
            {
                if constexpr (std::is_same_v<ReturnType, void>)
                {
                    func(largs...);
                    promise->set_value();
                }
                else
                {
                    promise->set_value(func(largs...));
                }
            }
            catch (...)
            {
                promise->set_exception(std::current_exception());
            }
        };

        // get the future before enqueuing the task
        auto future = shared_promise->get_future();
        // enqueue the task
        enqueue_task(std::move(task));
        return future;
#endif
    }

    /**
     * @brief Enqueue a task to be executed in the thread pool that returns void.
     * @tparam Function An invokable type.
     * @tparam Args Argument parameter pack for Function
     * @param func The callable to be executed
     * @param args Arguments that will be passed to the function.
     */
    template <typename Function, typename... Args>
        requires std::invocable<Function, Args...> &&
                 std::is_same_v<void, std::invoke_result_t<Function &&, Args &&...>>
    void enqueue_detach(Function &&func, Args &&...args)
    {
        enqueue_task(
            std::move([f = std::forward<Function>(func),
                       ... largs = std::forward<Args>(args)]() mutable -> decltype(auto)
                      {
                    // suppress exceptions
                    try {
                        std::invoke(f, largs...);
                    } catch (...) {
                    } }));
    }

    [[nodiscard]] auto size() const { return threads_.size(); }

private:
    template <typename Function>
    void enqueue_task(Function &&f)
    {
        auto i_opt = priority_queue_.copy_front_and_rotate_to_back();
        if (!i_opt.has_value())
        {
            // would only be a problem if there are zero threads
            return;
        }
        auto i = *(i_opt);
        pending_tasks_.fetch_add(1, std::memory_order_relaxed);
        tasks_[i].tasks.push_back(std::forward<Function>(f));
        tasks_[i].signal.release();
    }

    struct task_item
    {
        thread_safe_queue<FunctionType> tasks{};
        std::binary_semaphore signal{0};
    };

    std::vector<ThreadType> threads_;
    std::deque<task_item> tasks_;
    thread_safe_queue<std::size_t> priority_queue_;
    std::atomic_int_fast64_t pending_tasks_{};
};
