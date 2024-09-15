/*
 * async.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: A simple but useful async worker manager

**************************************************/

#ifndef ATOM_ASYNC_ASYNC_HPP
#define ATOM_ASYNC_ASYNC_HPP

#include <chrono>
#include <cmath>
#include <functional>
#include <future>
#include <memory>
#include <vector>

#include "atom/error/exception.hpp"

class TimeoutException : public atom::error::RuntimeError {
public:
    using atom::error::RuntimeError::RuntimeError;
};

#define THROW_TIMEOUT_EXCEPTION(...)                                       \
    throw TimeoutException(ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
                           __VA_ARGS__);

namespace atom::async {
/**
 * @brief Class for performing asynchronous tasks.
 *
 * This class allows you to start a task asynchronously and get the result when
 * it's done. It also provides functionality to cancel the task, check if it's
 * done or active, validate the result, set a callback function, and set a
 * timeout.
 *
 * @tparam ResultType The type of the result returned by the task.
 */
template <typename ResultType>
class AsyncWorker {
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
    void startAsync(Func &&func, Args &&...args);

    /**
     * @brief Gets the result of the task.
     *
     * @throw std::runtime_error if the task is not valid.
     * @return The result of the task.
     */
    auto getResult() -> ResultType;

    /**
     * @brief Cancels the task.
     *
     * If the task is valid, this function waits for the task to complete.
     */
    void cancel();

    /**
     * @brief Checks if the task is done.
     *
     * @return True if the task is done, false otherwise.
     */
    [[nodiscard]] auto isDone() const -> bool;

    /**
     * @brief Checks if the task is active.
     *
     * @return True if the task is active, false otherwise.
     */
    [[nodiscard]] auto isActive() const -> bool;

    /**
     * @brief Validates the result of the task using a validator function.
     *
     * @param validator The function used to validate the result.
     * @return True if the result is valid, false otherwise.
     */
    auto validate(std::function<bool(ResultType)> validator) -> bool;

    /**
     * @brief Sets a callback function to be called when the task is done.
     *
     * @param callback The callback function to be set.
     */
    void setCallback(std::function<void(ResultType)> callback);

    /**
     * @brief Sets a timeout for the task.
     *
     * @param timeout The timeout duration.
     */
    void setTimeout(std::chrono::seconds timeout);

    /**
     * @brief Waits for the task to complete.
     *
     * If a timeout is set, this function waits until the task is done or the
     * timeout is reached. If a callback function is set and the task is done,
     * the callback function is called with the result.
     */
    void waitForCompletion();

private:
    std::future<ResultType>
        task_;  ///< The future representing the asynchronous task.
    std::function<void(ResultType)>
        callback_;  ///< The callback function to be called when the task is
                    ///< done.
    std::chrono::seconds timeout_{0};  ///< The timeout duration for the task.
};

/**
 * @brief Class for managing multiple AsyncWorker instances.
 *
 * This class provides functionality to create and manage multiple AsyncWorker
 * instances.
 *
 * @tparam ResultType The type of the result returned by the tasks managed by
 * this class.
 */
template <typename ResultType>
class AsyncWorkerManager {
public:
    /**
     * @brief Default constructor.
     */
    AsyncWorkerManager() = default;

    /**
     * @brief Creates a new AsyncWorker instance and starts the task
     * asynchronously.
     *
     * @tparam Func The type of the function to be executed asynchronously.
     * @tparam Args The types of the arguments to be passed to the function.
     * @param func The function to be executed asynchronously.
     * @param args The arguments to be passed to the function.
     * @return A shared pointer to the created AsyncWorker instance.
     */
    template <typename Func, typename... Args>
    auto createWorker(Func &&func, Args &&...args)
        -> std::shared_ptr<AsyncWorker<ResultType>>;

    /**
     * @brief Cancels all the managed tasks.
     */
    void cancelAll();

    /**
     * @brief Checks if all the managed tasks are done.
     *
     * @return True if all tasks are done, false otherwise.
     */
    auto allDone() const -> bool;

    /**
     * @brief Waits for all the managed tasks to complete.
     */
    void waitForAll();

    /**
     * @brief Checks if a specific task is done.
     *
     * @param worker The AsyncWorker instance to check.
     * @return True if the task is done, false otherwise.
     */
    bool isDone(std::shared_ptr<AsyncWorker<ResultType>> worker) const;

    /**
     * @brief Cancels a specific task.
     *
     * @param worker The AsyncWorker instance to cancel.
     */
    void cancel(std::shared_ptr<AsyncWorker<ResultType>> worker);

private:
    std::vector<std::shared_ptr<AsyncWorker<ResultType>>>
        workers_;  ///< The list of managed AsyncWorker instances.
};

/**
 * @brief Gets the result of the task with a timeout.
 *
 * @param future The future representing the asynchronous task.
 * @param timeout The timeout duration.
 * @return The result of the task.
 */
template <typename ReturnType>
auto getWithTimeout(std::future<ReturnType> &future,
                    std::chrono::milliseconds timeout) -> ReturnType;

template <typename ResultType>
template <typename Func, typename... Args>
void AsyncWorker<ResultType>::startAsync(Func &&func, Args &&...args) {
    static_assert(std::is_invocable_r_v<ResultType, Func, Args...>,
                  "Function must return a result");
    task_ = std::async(std::launch::async, std::forward<Func>(func),
                       std::forward<Args>(args)...);
}

template <typename ResultType>
[[nodiscard]] auto AsyncWorker<ResultType>::getResult() -> ResultType {
    if (!task_.valid()) {
        throw std::invalid_argument("Task is not valid");
    }
    return task_.get();
}

template <typename ResultType>
void AsyncWorker<ResultType>::cancel() {
    if (task_.valid()) {
        task_.wait();  // 等待任务完成
    }
}

template <typename ResultType>
auto AsyncWorker<ResultType>::isDone() const -> bool {
    return task_.valid() && (task_.wait_for(std::chrono::seconds(0)) ==
                             std::future_status::ready);
}

template <typename ResultType>
auto AsyncWorker<ResultType>::isActive() const -> bool {
    return task_.valid() && (task_.wait_for(std::chrono::seconds(0)) ==
                             std::future_status::timeout);
}

template <typename ResultType>
auto AsyncWorker<ResultType>::validate(
    std::function<bool(ResultType)> validator) -> bool {
    if (!isDone()) {
    }
    ResultType result = getResult();
    return validator(result);
}

template <typename ResultType>
void AsyncWorker<ResultType>::setCallback(
    std::function<void(ResultType)> callback) {
    callback_ = callback;
}

template <typename ResultType>
void AsyncWorker<ResultType>::setTimeout(std::chrono::seconds timeout) {
    timeout_ = timeout;
}

template <typename ResultType>
void AsyncWorker<ResultType>::waitForCompletion() {
    if (timeout_ != std::chrono::seconds(0)) {
        auto startTime = std::chrono::steady_clock::now();
        while (!isDone()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (std::chrono::steady_clock::now() - startTime > timeout_) {
                cancel();
                break;
            }
        }
    } else {
        while (!isDone()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    if (callback_ && isDone()) {
        callback_(getResult());
    }
}

template <typename ResultType>
template <typename Func, typename... Args>
[[nodiscard]] auto AsyncWorkerManager<ResultType>::createWorker(
    Func &&func, Args &&...args) -> std::shared_ptr<AsyncWorker<ResultType>> {
    auto worker = std::make_shared<AsyncWorker<ResultType>>();
    workers_.push_back(worker);
    worker->StartAsync(std::forward<Func>(func), std::forward<Args>(args)...);
    return worker;
}

template <typename ResultType>
void AsyncWorkerManager<ResultType>::cancelAll() {
    for (auto &worker : workers_) {
        worker->Cancel();
    }
}

template <typename ResultType>
auto AsyncWorkerManager<ResultType>::allDone() const -> bool {
    return std::all_of(workers_.begin(), workers_.end(),
                       [](const auto &worker) { return worker->IsDone(); });
}

template <typename ResultType>
void AsyncWorkerManager<ResultType>::waitForAll() {
    while (!allDone()) {
    }
}

template <typename ResultType>
auto AsyncWorkerManager<ResultType>::isDone(
    std::shared_ptr<AsyncWorker<ResultType>> worker) const -> bool {
    return worker->IsDone();
}

template <typename ResultType>
void AsyncWorkerManager<ResultType>::cancel(
    std::shared_ptr<AsyncWorker<ResultType>> worker) {
    worker->Cancel();
}

template <typename T>
using EnableIfNotVoid = typename std::enable_if_t<!std::is_void_v<T>, T>;

// Retry strategy enum for different backoff strategies
enum class BackoffStrategy { FIXED, LINEAR, EXPONENTIAL };

/**
 * @brief Async execution with retry.
 *
 * @tparam Func The type of the function to be executed asynchronously.
 * @tparam Args The types of the arguments to be passed to the function.
 * @param func The function to be executed asynchronously.
 * @param args The arguments to be passed to the function.
 * @return A shared pointer to the created AsyncWorker instance.
 */
template <typename Func, typename Callback, typename ExceptionHandler,
          typename CompleteHandler, typename... Args>
auto asyncRetry(Func &&func, int attemptsLeft,
                std::chrono::milliseconds initialDelay,
                BackoffStrategy strategy,
                std::chrono::milliseconds maxTotalDelay, Callback &&callback,
                ExceptionHandler &&exceptionHandler,
                CompleteHandler &&completeHandler, Args &&...args)
    -> std::future<typename std::invoke_result_t<Func, Args...>> {
    using ReturnType = typename std::invoke_result_t<Func, Args...>;

    auto attempt = std::async(std::launch::async, std::forward<Func>(func),
                              std::forward<Args>(args)...);

    try {
        if constexpr (std::is_same_v<ReturnType, void>) {
            attempt.get();
            callback();
            completeHandler();
            return std::async(std::launch::async, [] {});
        } else {
            auto result = attempt.get();
            callback();
            completeHandler();
            return std::async(std::launch::async,
                              [result = std::move(result)]() mutable {
                                  return std::move(result);
                              });
        }
    } catch (const std::exception &e) {
        exceptionHandler(e);  // Call custom exception handler

        if (attemptsLeft <= 1 || maxTotalDelay.count() <= 0) {
            completeHandler();  // Invoke complete handler on final failure
            throw;
        }

        switch (strategy) {
            case BackoffStrategy::LINEAR:
                initialDelay *= 2;
                break;
            case BackoffStrategy::EXPONENTIAL:
                initialDelay = std::chrono::milliseconds(static_cast<int>(
                    initialDelay.count() * std::pow(2, (5 - attemptsLeft))));
                break;
            default:
                break;
        }

        std::this_thread::sleep_for(initialDelay);

        // Decrease the maximum total delay by the time spent in the last
        // attempt
        maxTotalDelay -= initialDelay;

        return asyncRetry(std::forward<Func>(func), attemptsLeft - 1,
                          initialDelay, strategy, maxTotalDelay,
                          std::forward<Callback>(callback),
                          std::forward<ExceptionHandler>(exceptionHandler),
                          std::forward<CompleteHandler>(completeHandler),
                          std::forward<Args>(args)...);
    }
}

// getWithTimeout function for C++17
template <typename T, typename Duration>
auto getWithTimeout(std::future<T> &future,
                    Duration timeout) -> EnableIfNotVoid<T> {
    if (future.wait_for(timeout) == std::future_status::ready) {
        return future.get();
    }
    THROW_TIMEOUT_EXCEPTION("Timeout occurred while waiting for future result");
}

// Helper function to get a future for a range of futures
template <typename InputIt>
auto whenAll(InputIt first, InputIt last,
             std::optional<std::chrono::milliseconds> timeout = std::nullopt)
    -> std::future<
        std::vector<typename std::iterator_traits<InputIt>::value_type>> {
    using FutureType = typename std::iterator_traits<InputIt>::value_type;
    using ResultType = std::vector<FutureType>;

    std::promise<ResultType> promise;
    std::future<ResultType> resultFuture = promise.get_future();

    // Launch an async task to wait for all the futures
    auto asyncTask = std::async([promise = std::move(promise), first, last,
                                 timeout]() mutable {
        ResultType results;
        try {
            for (auto it = first; it != last; ++it) {
                if (timeout) {
                    // Check each future with timeout (if specified)
                    if (it->wait_for(*timeout) == std::future_status::timeout) {
                        THROW_INVALID_ARGUMENT(
                            "Timeout while waiting for a future.");
                    }
                }
                results.push_back(std::move(*it));
            }
            promise.set_value(std::move(results));
        } catch (const std::exception &e) {
            promise.set_exception(
                std::current_exception());  // Pass the exception to the future
        }
    });

    // Optionally, store the future or use it if needed
    asyncTask.wait();  // Wait for the async task to finish

    return resultFuture;
}

// Helper to get the return type of a future
template <typename T>
using future_value_t = decltype(std::declval<T>().get());

// Helper function for a variadic template version (when_all for futures as
// arguments)
template <typename... Futures>
auto whenAll(Futures &&...futures)
    -> std::future<std::tuple<future_value_t<Futures>...>> {
    std::promise<std::tuple<future_value_t<Futures>...>> promise;
    std::future<std::tuple<future_value_t<Futures>...>> resultFuture =
        promise.get_future();

    // Use async to wait for all futures and gather results
    auto asyncTask =
        std::async([promise = std::move(promise),
                    futures = std::make_tuple(
                        std::forward<Futures>(futures)...)]() mutable {
            try {
                auto results = std::apply(
                    [](auto &&...fs) {
                        return std::make_tuple(
                            fs.get()...);  // Wait for each future and collect
                                           // the results
                    },
                    futures);
                promise.set_value(std::move(results));
            } catch (const std::exception &e) {
                promise.set_exception(std::current_exception());
            }
        });

    asyncTask.wait();  // Wait for the async task to finish

    return resultFuture;
}

// Primary template for EnhancedFuture
template <typename T>
class EnhancedFuture {
public:
    explicit EnhancedFuture(std::shared_future<T> &&fut)
        : future_(std::move(fut)), cancelled_(false) {}

    // Chaining: call another operation after the future is done
    template <typename F>
    auto then(F &&func) {
        using ResultType = std::invoke_result_t<F, T>;
        return EnhancedFuture<ResultType>(
            std::async(std::launch::async, [fut = future_,
                                            func = std::forward<F>(
                                                func)]() mutable {
                if (fut.valid()) {
                    return func(fut.get());
                }
                throw std::runtime_error("Future is invalid or cancelled");
            }).share());
    }

    // Wait with timeout and auto cancel
    auto waitFor(std::chrono::milliseconds timeout) -> std::optional<T> {
        if (future_.wait_for(timeout) == std::future_status::ready &&
            !cancelled_) {
            return future_.get();
        }
        cancel();
        return std::nullopt;
    }

    // Check if the future is done
    [[nodiscard]] auto isDone() const -> bool {
        return future_.wait_for(std::chrono::milliseconds(0)) ==
               std::future_status::ready;
    }

    // Set a completion callback, allows multiple callbacks
    template <typename F>
    void onComplete(F &&func) {
        if (!cancelled_) {
            callbacks_.emplace_back(std::forward<F>(func));
            std::async(std::launch::async, [this]() {
                try {
                    if (future_.valid()) {
                        auto result = future_.get();
                        for (auto &callback : callbacks_) {
                            callback(result);
                        }
                    }
                } catch (const std::exception &e) {
                }
            }).get();
        }
    }

    // Synchronous wait
    auto wait() -> T {
        if (cancelled_) {
            THROW_OBJ_NOT_EXIST("Future has been cancelled");
        }
        return future_.get();
    }

    // Support cancellation
    void cancel() { cancelled_ = true; }

    // Check if cancelled
    [[nodiscard]] auto isCancelled() const -> bool { return cancelled_; }

    // Exception handling
    auto getException() -> std::exception_ptr {
        try {
            future_.get();
        } catch (...) {
            return std::current_exception();
        }
        return nullptr;
    }

    // Retry mechanism
    template <typename F>
    auto retry(F &&func, int max_retries) {
        using ResultType = std::invoke_result_t<F, T>;
        return EnhancedFuture<ResultType>(
            std::async(std::launch::async, [fut = future_,
                                            func = std::forward<F>(func),
                                            max_retries]() mutable {
                for (int attempt = 0; attempt < max_retries; ++attempt) {
                    if (fut.valid()) {
                        try {
                            return func(fut.get());
                        } catch (const std::exception &e) {
                            if (attempt == max_retries - 1) {
                                throw;
                            }
                        }
                    } else {
                        THROW_UNLAWFUL_OPERATION(
                            "Future is invalid or cancelled");
                    }
                }
            }).share());
    }

protected:
    std::shared_future<T> future_;
    std::vector<std::function<void(T)>> callbacks_;
    std::atomic<bool> cancelled_;
};

// Specialization for void type
template <>
class EnhancedFuture<void> {
public:
    explicit EnhancedFuture(std::shared_future<void> &&fut)
        : future_(std::move(fut)), cancelled_(false) {}

    template <typename F>
    auto then(F &&func) {
        using ResultType = std::invoke_result_t<F>;
        return EnhancedFuture<ResultType>(
            std::async(std::launch::async, [fut = future_,
                                            func = std::forward<F>(
                                                func)]() mutable {
                if (fut.valid()) {
                    fut.get();
                    return func();
                }
                THROW_UNLAWFUL_OPERATION("Future is invalid or cancelled");
            }).share());
    }

    auto waitFor(std::chrono::milliseconds timeout) -> bool {
        if (future_.wait_for(timeout) == std::future_status::ready &&
            !cancelled_) {
            future_.get();
            return true;
        }
        cancel();
        return false;
    }

    [[nodiscard]] auto isDone() const -> bool {
        return future_.wait_for(std::chrono::milliseconds(0)) ==
               std::future_status::ready;
    }

    template <typename F>
    void onComplete(F &&func) {
        if (!cancelled_) {
            callbacks_.emplace_back(std::forward<F>(func));
            std::async(std::launch::async, [this]() {
                try {
                    if (future_.valid()) {
                        future_.get();
                        for (auto &callback : callbacks_) {
                            callback();
                        }
                    }
                } catch (const std::exception &e) {
                }
            }).get();
        }
    }

    void wait() {
        if (cancelled_) {
            THROW_OBJ_NOT_EXIST("Future has been cancelled");
        }
        future_.get();
    }

    void cancel() { cancelled_ = true; }

    [[nodiscard]] auto isCancelled() const -> bool { return cancelled_; }

    auto getException() -> std::exception_ptr {
        try {
            future_.get();
        } catch (...) {
            return std::current_exception();
        }
        return nullptr;
    }

protected:
    std::shared_future<void> future_;
    std::vector<std::function<void()>> callbacks_;
    std::atomic<bool> cancelled_;
};

// Helper function to create EnhancedFuture
template <typename F, typename... Args>
auto makeEnhancedFuture(F &&f, Args &&...args) {
    using result_type = std::invoke_result_t<F, Args...>;
    return EnhancedFuture<result_type>(std::async(std::launch::async,
                                                  std::forward<F>(f),
                                                  std::forward<Args>(args)...)
                                           .share());
}

}  // namespace atom::async
#endif
