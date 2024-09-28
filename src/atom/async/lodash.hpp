#ifndef ATOM_ASYNC_DEBOUNCE_HPP
#define ATOM_ASYNC_DEBOUNCE_HPP

#include <chrono>
#include <functional>
#include <mutex>

/**
 * @class Throttle
 * @brief A class that ensures a function is not called more often than a
 * specified interval.
 */
class Throttle {
public:
    /**
     * @brief Constructs a Throttle object.
     * @param func The function to be throttled.
     * @param interval The minimum interval between function calls.
     * @param leading If true, the function is called immediately on the leading
     * edge.
     * @param trailing If true, the function is called on the trailing edge if
     * it was called during the interval.
     */
    Throttle(std::function<void()> func, std::chrono::milliseconds interval,
             bool leading = true, bool trailing = false);

    /**
     * @brief Calls the throttled function if the interval has passed.
     */
    void operator()();

    /**
     * @brief Resets the throttle, allowing the function to be called
     * immediately.
     */
    void reset();

    /**
     * @brief Sets a new interval for the throttle.
     * @param newInterval The new interval.
     */
    void setInterval(std::chrono::milliseconds newInterval);

    /**
     * @brief Cancels any scheduled function calls.
     */
    void cancel();

private:
    /**
     * @brief Handles the throttling logic.
     * @param now The current time point.
     */
    void handleThrottle(std::chrono::steady_clock::time_point now);

    std::function<void()> func_;  ///< The function to be throttled.
    std::chrono::milliseconds
        interval_;  ///< The minimum interval between function calls.
    bool leading_;  ///< Whether to call the function immediately on the leading
                    ///< edge.
    bool trailing_;  ///< Whether to call the function on the trailing edge if
                     ///< it was called during the interval.
    bool isScheduled_ = false;  ///< Whether a function call is scheduled.
    bool hasExecutedFirstCall_ =
        false;  ///< Whether the function has been called at least once.
    std::chrono::steady_clock::time_point
        lastCall_;      ///< The time point of the last function call.
    std::mutex mutex_;  ///< Mutex to protect shared data.
};

/**
 * @class Debounce
 * @brief A class that ensures a function is only called after a specified
 * interval has passed since the last call.
 */
class Debounce {
public:
    /**
     * @brief Constructs a Debounce object.
     * @param func The function to be debounced.
     * @param interval The interval to wait before calling the function.
     */
    Debounce(std::function<void()> func, std::chrono::milliseconds interval);

    /**
     * @brief Calls the debounced function if the interval has passed since the
     * last call.
     */
    void operator()();

    /**
     * @brief Resets the debounce, allowing the function to be called
     * immediately.
     */
    void reset();

    /**
     * @brief Sets a new interval for the debounce.
     * @param newInterval The new interval.
     */
    void setInterval(std::chrono::milliseconds newInterval);

    /**
     * @brief Cancels any scheduled function calls.
     */
    void cancel();

private:
    /**
     * @brief Handles the debouncing logic.
     */
    void handleDebounce();

    std::function<void()> func_;  ///< The function to be debounced.
    std::chrono::milliseconds
        interval_;  ///< The interval to wait before calling the function.
    bool isScheduled_ = false;  ///< Whether a function call is scheduled.
    std::mutex mutex_;          ///< Mutex to protect shared data.
};

#endif  // ATOM_ASYNC_DEBOUNCE_HPP