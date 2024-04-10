/*
 * thread_wrapper.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-13

Description: A simple wrapper of std::jthread

**************************************************/

#ifndef ATOM_ASYNC_THREAD_WRAPPER_HPP
#define ATOM_ASYNC_THREAD_WRAPPER_HPP

#include <condition_variable>
#include <exception>
#include <functional>
#include <future>
#include <mutex>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <utility>

/**
 * @brief A wrapper class for managing a C++20 jthread.
 *
 * This class provides a convenient interface for managing a C++20 jthread,
 * allowing for starting, stopping, and joining threads easily.
 */
class Thread {
public:
    /**
     * @brief Default constructor.
     */
    Thread() = default;

    /**
     * @brief Move constructor.
     * @param other The Thread object to move from.
     */
    Thread(Thread&&) noexcept = default;

    /**
     * @brief Move assignment operator.
     * @param other The Thread object to move from.
     * @return Reference to the assigned Thread object.
     */
    Thread& operator=(Thread&&) noexcept = default;

    /**
     * @brief Deleted copy constructor.
     */
    Thread(const Thread&) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    Thread& operator=(const Thread&) = delete;

    /**
     * @brief Starts a new thread with the specified callable object and
     * arguments.
     *
     * If the callable object is invocable with a std::stop_token and the
     * provided arguments, it will be invoked with a std::stop_token as the
     * first argument. Otherwise, it will be invoked with the provided
     * arguments.
     *
     * @tparam Callable The type of the callable object.
     * @tparam Args The types of the arguments.
     * @param func The callable object to execute in the new thread.
     * @param args The arguments to pass to the callable object.
     */
    template <typename Callable, typename... Args>
    void start(Callable&& func, Args&&... args) {
        thread_ =
            std::jthread([func = std::forward<Callable>(func),
                          ... args = std::forward<Args>(args), this]() mutable {
                if constexpr (std::is_invocable_v<Callable, std::stop_token,
                                                  Args...>) {
                    func(std::stop_token(thread_.get_stop_token()),
                         std::move(args)...);
                } else {
                    func(std::move(args)...);
                }
            });
    }

    /**
     * @brief Requests the thread to stop execution.
     */
    void request_stop() { thread_.request_stop(); }

    /**
     * @brief Waits for the thread to finish execution.
     */
    void join() { thread_.join(); }

    /**
     * @brief Checks if the thread is currently running.
     * @return True if the thread is running, false otherwise.
     */
    [[nodiscard]] bool running() const noexcept { return thread_.joinable(); }

    /**
     * @brief Swaps the content of this Thread object with another Thread
     * object.
     * @param other The Thread object to swap with.
     */
    void swap(Thread& other) noexcept { thread_.swap(other.thread_); }

    /**
     * @brief Gets the underlying std::jthread object.
     * @return Reference to the underlying std::jthread object.
     */
    [[nodiscard]] std::jthread& get_thread() noexcept { return thread_; }

    /**
     * @brief Gets the underlying std::jthread object (const version).
     * @return Constant reference to the underlying std::jthread object.
     */
    [[nodiscard]] const std::jthread& get_thread() const noexcept {
        return thread_;
    }

    /**
     * @brief Gets the ID of the thread.
     * @return The ID of the thread.
     */
    [[nodiscard]] std::thread::id get_id() const noexcept {
        return thread_.get_id();
    }

    /**
     * @brief Gets the underlying std::stop_source object.
     * @return The underlying std::stop_source object.
     */
    [[nodiscard]] std::stop_source get_stop_source() noexcept {
        return thread_.get_stop_source();
    }

    /**
     * @brief Gets the underlying std::stop_token object.
     * @return The underlying std::stop_token object.
     */
    [[nodiscard]] std::stop_token get_stop_token() const noexcept {
        return thread_.get_stop_token();
    }

    /**
     * @brief Default destructor.
     */
    ~Thread() = default;

private:
    std::jthread thread_;  ///< The underlying jthread object.
};

#endif
