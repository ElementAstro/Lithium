/*
 * pool.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-13

Description: A very simple thread pool for preload

**************************************************/

#ifndef ATOM_ASYNC_POOL_HPP
#define ATOM_ASYNC_POOL_HPP

#include <algorithm>
#include <atomic>
#include <concepts>
#include <deque>
#include <functional>
#include <future>
#include <mutex>
#include <optional>
#include <semaphore>
#include <thread>
#include <type_traits>
#include <utility>
#include "macro.hpp"
#ifdef __has_include
#if __has_include(<version>)
#include <version>
#endif
#endif

namespace atom::async {
/**
 * @brief Simple concept for the Lockable and Basic Lockable types as defined by
 * the C++ standard.
 * @details See https://en.cppreference.com/w/cpp/named_req/Lockable and
 * https://en.cppreference.com/w/cpp/named_req/BasicLockable for details.
 */
template <typename Lock>
concept is_lockable = requires(Lock&& lock) {
    lock.lock();
    lock.unlock();
    { lock.try_lock() } -> std::convertible_to<bool>;
};

template <typename T, typename Lock = std::mutex>
    requires is_lockable<Lock>
class ThreadSafeQueue {
public:
    using value_type = T;
    using size_type = typename std::deque<T>::size_type;

    ThreadSafeQueue() = default;

    // Copy constructor
    ThreadSafeQueue(const ThreadSafeQueue& other) {
        std::scoped_lock lock(other.mutex_);
        data_ = other.data_;
    }

    // Copy assignment operator
    auto operator=(const ThreadSafeQueue& other) -> ThreadSafeQueue& {
        if (this != &other) {
            std::scoped_lock lockThis(mutex_, std::defer_lock);
            std::scoped_lock lockOther(other.mutex_, std::defer_lock);
            std::lock(lockThis, lockOther);
            data_ = other.data_;
        }
        return *this;
    }

    // Move constructor
    ThreadSafeQueue(ThreadSafeQueue&& other) noexcept {
        std::scoped_lock lock(other.mutex_);
        data_ = std::move(other.data_);
    }

    // Move assignment operator
    auto operator=(ThreadSafeQueue&& other) noexcept -> ThreadSafeQueue& {
        if (this != &other) {
            std::scoped_lock lockThis(mutex_, std::defer_lock);
            std::scoped_lock lockOther(other.mutex_, std::defer_lock);
            std::lock(lockThis, lockOther);
            data_ = std::move(other.data_);
        }
        return *this;
    }

    void pushBack(T&& value) {
        std::scoped_lock lock(mutex_);
        data_.push_back(std::forward<T>(value));
    }

    void pushFront(T&& value) {
        std::scoped_lock lock(mutex_);
        data_.push_front(std::forward<T>(value));
    }

    [[nodiscard]] auto empty() const -> bool {
        std::scoped_lock lock(mutex_);
        return data_.empty();
    }

    [[nodiscard]] auto size() const -> size_type {
        std::scoped_lock lock(mutex_);
        return data_.size();
    }

    [[nodiscard]] auto popFront() -> std::optional<T> {
        std::scoped_lock lock(mutex_);
        if (data_.empty()) {
            return std::nullopt;
        }

        auto front = std::move(data_.front());
        data_.pop_front();
        return front;
    }

    [[nodiscard]] auto popBack() -> std::optional<T> {
        std::scoped_lock lock(mutex_);
        if (data_.empty()) {
            return std::nullopt;
        }

        auto back = std::move(data_.back());
        data_.pop_back();
        return back;
    }

    [[nodiscard]] auto steal() -> std::optional<T> {
        std::scoped_lock lock(mutex_);
        if (data_.empty()) {
            return std::nullopt;
        }

        auto back = std::move(data_.back());
        data_.pop_back();
        return back;
    }

    void rotateToFront(const T& item) {
        std::scoped_lock lock(mutex_);
        auto iter = std::find(data_.begin(), data_.end(), item);

        if (iter != data_.end()) {
            std::ignore = data_.erase(iter);
        }

        data_.push_front(item);
    }

    [[nodiscard]] auto copyFrontAndRotateToBack() -> std::optional<T> {
        std::scoped_lock lock(mutex_);

        if (data_.empty()) {
            return std::nullopt;
        }

        auto front = data_.front();
        data_.pop_front();

        data_.push_back(front);

        return front;
    }

    void clear() {
        std::scoped_lock lock(mutex_);
        data_.clear();
    }

private:
    std::deque<T> data_;
    mutable Lock mutex_;
};

namespace details {
#ifdef __cpp_lib_move_only_function
using default_function_type = std::move_only_function<void()>;
#else
using default_function_type = std::function<void()>;
#endif
}  // namespace details

template <typename FunctionType = details::default_function_type,
          typename ThreadType = std::jthread>
    requires std::invocable<FunctionType> &&
             std::is_same_v<void, std::invoke_result_t<FunctionType>>
class ThreadPool {
public:
    template <
        typename InitializationFunction = std::function<void(std::size_t)>>
        requires std::invocable<InitializationFunction, std::size_t> &&
                 std::is_same_v<void, std::invoke_result_t<
                                          InitializationFunction, std::size_t>>
    explicit ThreadPool(
        const unsigned int& number_of_threads =
            std::thread::hardware_concurrency(),
        InitializationFunction init = [](std::size_t) {})
        : tasks_(number_of_threads) {
        std::size_t currentId = 0;
        for (std::size_t i = 0; i < number_of_threads; ++i) {
            priority_queue_.pushBack(std::move(currentId));
            try {
                threads_.emplace_back([&, threadId = currentId,
                                       init](const std::stop_token& stop_tok) {
                    try {
                        std::invoke(init, threadId);
                    } catch (...) {
                    }

                    do {
                        tasks_[threadId].signal.acquire();

                        do {
                            while (auto task =
                                       tasks_[threadId].tasks.popFront()) {
                                unassigned_tasks_.fetch_sub(
                                    1, std::memory_order_release);
                                std::invoke(std::move(task.value()));
                                in_flight_tasks_.fetch_sub(
                                    1, std::memory_order_release);
                            }

                            for (std::size_t j = 1; j < tasks_.size(); ++j) {
                                const std::size_t INDEX =
                                    (threadId + j) % tasks_.size();
                                if (auto task = tasks_[INDEX].tasks.steal()) {
                                    unassigned_tasks_.fetch_sub(
                                        1, std::memory_order_release);
                                    std::invoke(std::move(task.value()));
                                    in_flight_tasks_.fetch_sub(
                                        1, std::memory_order_release);
                                    break;
                                }
                            }
                        } while (unassigned_tasks_.load(
                                     std::memory_order_acquire) > 0);

                        priority_queue_.rotateToFront(threadId);

                        if (in_flight_tasks_.load(std::memory_order_acquire) ==
                            0) {
                            threads_complete_signal_.store(
                                true, std::memory_order_release);
                            threads_complete_signal_.notify_one();
                        }

                    } while (!stop_tok.stop_requested());
                });
                ++currentId;

            } catch (...) {
                tasks_.pop_back();
                std::ignore = priority_queue_.popBack();
            }
        }
    }

    ~ThreadPool() {
        waitForTasks();

        for (auto& thread : threads_) {
            thread.request_stop();
        }

        for (auto& task : tasks_) {
            task.signal.release();
        }

        for (auto& thread : threads_) {
            thread.join();
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    auto operator=(const ThreadPool&) -> ThreadPool& = delete;

    // Define move constructor and move assignment operator
    ThreadPool(ThreadPool&& other) noexcept = default;
    auto operator=(ThreadPool&& other) noexcept -> ThreadPool& = default;

    template <typename Function, typename... Args,
              typename ReturnType = std::invoke_result_t<Function&&, Args&&...>>
        requires std::invocable<Function, Args...>
    [[nodiscard]] auto enqueue(Function func,
                               Args... args) -> std::future<ReturnType> {
#ifdef __cpp_lib_move_only_function
        std::promise<ReturnType> promise;
        auto future = promise.get_future();
        auto task = [func = std::move(func), ... largs = std::move(args),
                     promise = std::move(promise)]() mutable {
            try {
                if constexpr (std::is_same_v<ReturnType, void>) {
                    func(largs...);
                    promise.set_value();
                } else {
                    promise.set_value(func(largs...));
                }
            } catch (...) {
                promise.set_exception(std::current_exception());
            }
        };
        enqueueTask(std::move(task));
        return future;
#else
        auto shared_promise = std::make_shared<std::promise<ReturnType>>();
        auto task = [func = std::move(func), ... largs = std::move(args),
                     promise = shared_promise]() {
            try {
                if constexpr (std::is_same_v<ReturnType, void>) {
                    func(largs...);
                    promise->set_value();
                } else {
                    promise->set_value(func(largs...));
                }
            } catch (...) {
                promise->set_exception(std::current_exception());
            }
        };

        auto future = shared_promise->get_future();
        enqueue_task(std::move(task));
        return future;
#endif
    }

    template <typename Function, typename... Args>
        requires std::invocable<Function, Args...>
    void enqueueDetach(Function&& func, Args&&... args) {
        enqueueTask([func = std::forward<Function>(func),
                     ... largs = std::forward<Args>(args)]() mutable {
            try {
                if constexpr (std::is_same_v<void,
                                             std::invoke_result_t<Function&&,
                                                                  Args&&...>>) {
                    std::invoke(func, largs...);
                } else {
                    std::ignore = std::invoke(func, largs...);
                }
            } catch (...) {
            }
        });
    }

    [[nodiscard]] auto size() const -> std::size_t { return threads_.size(); }

    void waitForTasks() {
        if (in_flight_tasks_.load(std::memory_order_acquire) > 0) {
            threads_complete_signal_.wait(false);
        }
    }

private:
    template <typename Function>
    void enqueueTask(Function&& func) {
        auto iOpt = priority_queue_.copyFrontAndRotateToBack();
        if (!iOpt.has_value()) {
            return;
        }
        auto index = *(iOpt);

        unassigned_tasks_.fetch_add(1, std::memory_order_release);
        const auto PREV_IN_FLIGHT =
            in_flight_tasks_.fetch_add(1, std::memory_order_release);

        if (PREV_IN_FLIGHT == 0) {
            threads_complete_signal_.store(false, std::memory_order_release);
        }

        tasks_[index].tasks.pushBack(std::forward<Function>(func));
        tasks_[index].signal.release();
    }

    struct TaskItem {
        atom::async::ThreadSafeQueue<FunctionType> tasks{};
        std::binary_semaphore signal{0};
    } ATOM_ALIGNAS(128);

    std::vector<ThreadType> threads_;
    std::deque<TaskItem> tasks_;
    atom::async::ThreadSafeQueue<std::size_t> priority_queue_;
    std::atomic_int_fast64_t unassigned_tasks_{0}, in_flight_tasks_{0};
    std::atomic_bool threads_complete_signal_{false};
};
}  // namespace atom::async

#endif  // ATOM_ASYNC_POOL_HPP