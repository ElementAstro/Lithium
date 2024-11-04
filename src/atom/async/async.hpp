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

#include "atom/async/future.hpp"
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
auto asyncRetryImpl(Func &&func, int attemptsLeft,
                    std::chrono::milliseconds initialDelay,
                    BackoffStrategy strategy,
                    std::chrono::milliseconds maxTotalDelay,
                    Callback &&callback, ExceptionHandler &&exceptionHandler,
                    CompleteHandler &&completeHandler, Args &&...args) ->
    typename std::invoke_result_t<Func, Args...> {
    using ReturnType = typename std::invoke_result_t<Func, Args...>;

    auto attempt = std::async(std::launch::async, std::forward<Func>(func),
                              std::forward<Args>(args)...);

    try {
        if constexpr (std::is_same_v<ReturnType, void>) {
            attempt.get();
            callback();
            completeHandler();
            return;
        } else {
            auto result = attempt.get();
            callback();
            completeHandler();
            return result;
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

        return asyncRetryImpl(std::forward<Func>(func), attemptsLeft - 1,
                              initialDelay, strategy, maxTotalDelay,
                              std::forward<Callback>(callback),
                              std::forward<ExceptionHandler>(exceptionHandler),
                              std::forward<CompleteHandler>(completeHandler),
                              std::forward<Args>(args)...);
    }
}

template <typename Func, typename Callback, typename ExceptionHandler,
          typename CompleteHandler, typename... Args>
auto asyncRetry(Func &&func, int attemptsLeft,
                std::chrono::milliseconds initialDelay,
                BackoffStrategy strategy,
                std::chrono::milliseconds maxTotalDelay, Callback &&callback,
                ExceptionHandler &&exceptionHandler,
                CompleteHandler &&completeHandler, Args &&...args)
    -> std::future<typename std::invoke_result_t<Func, Args...>> {

    return std::async(std::launch::async, [=]() mutable {
        return asyncRetryImpl(std::forward<Func>(func), attemptsLeft,
                              initialDelay, strategy, maxTotalDelay,
                              std::forward<Callback>(callback),
                              std::forward<ExceptionHandler>(exceptionHandler),
                              std::forward<CompleteHandler>(completeHandler),
                              std::forward<Args>(args)...);
    });
}

template <typename Func, typename Callback, typename ExceptionHandler,
          typename CompleteHandler, typename... Args>
auto asyncRetryE(Func &&func, int attemptsLeft,
                std::chrono::milliseconds initialDelay,
                BackoffStrategy strategy,
                std::chrono::milliseconds maxTotalDelay, Callback &&callback,
                ExceptionHandler &&exceptionHandler,
                CompleteHandler &&completeHandler, Args &&...args)
    -> EnhancedFuture<typename std::invoke_result_t<Func, Args...>> {
    using ReturnType = typename std::invoke_result_t<Func, Args...>;

    auto future =
        std::async(std::launch::async, [=]() mutable {
            return asyncRetryImpl(
                std::forward<Func>(func), attemptsLeft, initialDelay, strategy,
                maxTotalDelay, std::forward<Callback>(callback),
                std::forward<ExceptionHandler>(exceptionHandler),
                std::forward<CompleteHandler>(completeHandler),
                std::forward<Args>(args)...);
        }).share();

    if constexpr (std::is_same_v<ReturnType, void>) {
        return EnhancedFuture<void>(std::shared_future<void>(future));
    } else {
        return EnhancedFuture<ReturnType>(
            std::shared_future<ReturnType>(future));
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
}  // namespace atom::async
#endif
