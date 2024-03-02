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

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <vector>

#include "atom/error/exception.hpp"

namespace Atom::Async {
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
     * If a timeout is set, this function waits until the task is done or the
     * timeout is reached. If a callback function is set and the task is done,
     * the callback function is called with the result.
     */
    void WaitForCompletion();

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
    std::shared_ptr<AsyncWorker<ResultType>> CreateWorker(Func &&func,
                                                          Args &&...args);

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
    std::vector<std::shared_ptr<AsyncWorker<ResultType>>>
        workers_;  ///< The list of managed AsyncWorker instances.
};

/**
 * @brief Async execution with retry.
 *
 * @tparam Func The type of the function to be executed asynchronously.
 * @tparam Args The types of the arguments to be passed to the function.
 * @param func The function to be executed asynchronously.
 * @param args The arguments to be passed to the function.
 * @return A shared pointer to the created AsyncWorker instance.
 */
template <typename Func, typename... Args>
std::future<decltype(std::declval<Func>()(std::declval<Args>()...))> asyncRetry(
    Func &&func, int attemptsLeft, std::chrono::milliseconds delay,
    Args &&...args);

/**
 * @brief Gets the result of the task with a timeout.
 *
 * @param future The future representing the asynchronous task.
 * @param timeout The timeout duration.
 * @return The result of the task.
 */
template <typename ReturnType>
ReturnType getWithTimeout(std::future<ReturnType> &future,
                          std::chrono::milliseconds timeout);
}  // namespace Atom::Async

#include "async_impl.hpp"

#endif
