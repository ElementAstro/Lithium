/*
 * timer.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-14

Description: Timer class for C++

**************************************************/

#ifndef ATOM_ASYNC_TIMER_HPP
#define ATOM_ASYNC_TIMER_HPP

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace atom::async {
/**
 * @brief Represents a task to be scheduled and executed by the Timer.
 */
class TimerTask {
public:
    /**
     * @brief Constructor for TimerTask.
     *
     * @param func The function to be executed when the task runs.
     * @param delay The delay in milliseconds before the first execution.
     * @param repeatCount The number of times the task should be repeated. -1
     * for infinite repetition.
     * @param priority The priority of the task.
     */
    explicit TimerTask(std::function<void()> func, unsigned int delay,
                       int repeatCount, int priority);

    /**
     * @brief Comparison operator for comparing two TimerTask objects based on
     * their next execution time.
     *
     * @param other Another TimerTask object to compare to.
     * @return True if this task's next execution time is earlier than the other
     * task's next execution time.
     */
    auto operator<(const TimerTask &other) const -> bool;

    /**
     * @brief Executes the task's associated function.
     */
    void run();

    /**
     * @brief Get the next scheduled execution time of the task.
     *
     * @return The steady clock time point representing the next execution time.
     */
    auto getNextExecutionTime() const -> std::chrono::steady_clock::time_point;

    std::function<void()> m_func;  ///< The function to be executed.
    unsigned int m_delay;          ///< The delay before the first execution.
    int m_repeatCount;             ///< The number of repetitions remaining.
    int m_priority;                ///< The priority of the task.
    std::chrono::steady_clock::time_point
        m_nextExecutionTime;  ///< The next execution time.
};

/**
 * @brief Represents a timer for scheduling and executing tasks.
 */
class Timer {
public:
    /**
     * @brief Constructor for Timer.
     */
    Timer();

    /**
     * @brief Destructor for Timer.
     */
    ~Timer();

    /**
     * @brief Schedules a task to be executed once after a specified delay.
     *
     * @tparam Function The type of the function to be executed.
     * @tparam Args The types of the arguments to be passed to the function.
     * @param func The function to be executed.
     * @param delay The delay in milliseconds before the function is executed.
     * @param args The arguments to be passed to the function.
     * @return A future representing the result of the function execution.
     */
    template <typename Function, typename... Args>
    [[nodiscard]] auto setTimeout(Function &&func, unsigned int delay,
                                  Args &&...args)
        -> std::future<typename std::result_of<Function(Args...)>::type>;

    /**
     * @brief Schedules a task to be executed repeatedly at a specified
     * interval.
     *
     * @tparam Function The type of the function to be executed.
     * @tparam Args The types of the arguments to be passed to the function.
     * @param func The function to be executed.
     * @param interval The interval in milliseconds between executions.
     * @param repeatCount The number of times the function should be repeated.
     * -1 for infinite repetition.
     * @param priority The priority of the task.
     * @param args The arguments to be passed to the function.
     */
    template <typename Function, typename... Args>
    void setInterval(Function &&func, unsigned int interval, int repeatCount,
                     int priority, Args &&...args);

    [[nodiscard]] auto now() const -> std::chrono::steady_clock::time_point;

    /**
     * @brief Cancels all scheduled tasks.
     */
    void cancelAllTasks();

    /**
     * @brief Pauses the execution of scheduled tasks.
     */
    void pause();

    /**
     * @brief Resumes the execution of scheduled tasks after pausing.
     */
    void resume();

    /**
     * @brief Stops the timer and cancels all tasks.
     */
    void stop();

    /**
     * @brief Sets a callback function to be called when a task is executed.
     *
     * @tparam Function The type of the callback function.
     * @param func The callback function to be set.
     */
    template <typename Function>
    void setCallback(Function &&func);

    [[nodiscard]] auto getTaskCount() const -> size_t;

private:
    /**
     * @brief Adds a task to the task queue.
     *
     * @tparam Function The type of the function to be executed.
     * @tparam Args The types of the arguments to be passed to the function.
     * @param func The function to be executed.
     * @param delay The delay in milliseconds before the function is executed.
     * @param repeatCount The number of repetitions remaining.
     * @param priority The priority of the task.
     * @param args The arguments to be passed to the function.
     * @return A future representing the result of the function execution.
     */
    template <typename Function, typename... Args>
    auto addTask(Function &&func, unsigned int delay, int repeatCount,
                 int priority, Args &&...args)
        -> std::future<typename std::result_of<Function(Args...)>::type>;

    /**
     * @brief Main execution loop for processing and running tasks.
     */
    void run();

#if _cplusplus >= 202203L
    std::jthread
        m_thread;  ///< The thread for running the timer loop (C++20 or later).
#else
    std::thread m_thread;  ///< The thread for running the timer loop.
#endif
    std::priority_queue<TimerTask>
        m_taskQueue;             ///< The priority queue for scheduled tasks.
    mutable std::mutex m_mutex;  ///< Mutex for thread synchronization.
    std::condition_variable
        m_cond;  ///< Condition variable for thread synchronization.
    std::function<void()> m_callback;  ///< The callback function to be called
                                       ///< when a task is executed.
    bool m_stop;    ///< Flag indicating whether the timer should stop.
    bool m_paused;  ///< Flag indicating whether the timer is paused.
};

template <typename Function, typename... Args>
auto Timer::setTimeout(Function &&func, unsigned int delay, Args &&...args)
    -> std::future<typename std::result_of<Function(Args...)>::type> {
    using ReturnType = typename std::result_of<Function(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    std::unique_lock lock(m_mutex);
    m_taskQueue.emplace([task]() { (*task)(); }, delay, 1, 0);
    m_cond.notify_all();
    return result;
}

template <typename Function, typename... Args>
void Timer::setInterval(Function &&func, unsigned int interval, int repeatCount,
                        int priority, Args &&...args) {
    addTask(std::forward<Function>(func), interval, repeatCount, priority,
            std::forward<Args>(args)...);
}

template <typename Function, typename... Args>
std::future<typename std::result_of<Function(Args...)>::type> Timer::addTask(
    Function &&func, unsigned int delay, int repeatCount, int priority,
    Args &&...args) {
    using ReturnType = typename std::result_of<Function(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(
        std::bind(std::forward<Function>(func), std::forward<Args>(args)...));
    std::future<ReturnType> result = task->get_future();
    std::unique_lock lock(m_mutex);
    m_taskQueue.emplace([task]() { (*task)(); }, delay, repeatCount, priority);
    m_cond.notify_all();
    return result;
}

template <typename Function>
void Timer::setCallback(Function &&func) {
    m_callback = std::forward<Function>(func);
}
}  // namespace atom::async

#endif
