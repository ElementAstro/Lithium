#ifndef ATOM_ASYNC_LIMITER_HPP
#define ATOM_ASYNC_LIMITER_HPP

#include <chrono>
#include <coroutine>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>

namespace atom::async {
/**
 * @brief A rate limiter class to control the rate of function executions.
 */
class RateLimiter {
public:
    /**
     * @brief Settings for the rate limiter.
     */
    struct Settings {
        size_t maxRequests;  ///< Maximum number of requests allowed in the time
                             ///< window.
        std::chrono::seconds
            timeWindow;  ///< The time window in which maxRequests are allowed.

        /**
         * @brief Constructor for Settings.
         * @param max_requests Maximum number of requests.
         * @param time_window Duration of the time window.
         */
        explicit Settings(
            size_t max_requests = 5,
            std::chrono::seconds time_window = std::chrono::seconds(1));
    };

    /**
     * @brief Constructor for RateLimiter.
     */
    RateLimiter();

    /**
     * @brief Awaiter class for handling coroutines.
     */
    class Awaiter {
    public:
        /**
         * @brief Constructor for Awaiter.
         * @param limiter Reference to the rate limiter.
         * @param function_name Name of the function to be rate-limited.
         */
        Awaiter(RateLimiter& limiter, const std::string& function_name);

        /**
         * @brief Checks if the awaiter is ready.
         * @return Always returns false.
         */
        auto await_ready() -> bool;

        /**
         * @brief Suspends the coroutine.
         * @param handle Coroutine handle.
         */
        void await_suspend(std::coroutine_handle<> handle);

        /**
         * @brief Resumes the coroutine.
         */
        void await_resume();

    private:
        RateLimiter& limiter_;
        std::string function_name_;
    };

    /**
     * @brief Acquires the rate limiter for a specific function.
     * @param function_name Name of the function to be rate-limited.
     * @return An Awaiter object.
     */
    Awaiter acquire(const std::string& function_name);

    /**
     * @brief Sets the rate limit for a specific function.
     * @param function_name Name of the function to be rate-limited.
     * @param max_requests Maximum number of requests allowed.
     * @param time_window Duration of the time window.
     */
    void setFunctionLimit(const std::string& function_name, size_t max_requests,
                          std::chrono::seconds time_window);

    /**
     * @brief Pauses the rate limiter.
     */
    void pause();

    /**
     * @brief Resumes the rate limiter.
     */
    void resume();

    /**
     * @brief Prints the log of requests.
     */
    void printLog();

    /**
     * @brief Gets the number of rejected requests for a specific function.
     * @param function_name Name of the function.
     * @return Number of rejected requests.
     */
    auto getRejectedRequests(const std::string& function_name) -> size_t;

#if !defined(TEST_F) && !defined(TEST)
private:
#endif
    /**
     * @brief Cleans up old requests outside the time window.
     * @param function_name Name of the function.
     * @param time_window Duration of the time window.
     */
    void cleanup(const std::string& function_name,
                 const std::chrono::seconds& time_window);

    /**
     * @brief Processes waiting coroutines.
     */
    void processWaiters();

    std::unordered_map<std::string, Settings> settings_;
    std::unordered_map<std::string,
                       std::deque<std::chrono::steady_clock::time_point>>
        requests_;
    std::unordered_map<std::string, std::deque<std::coroutine_handle<>>>
        waiters_;
    std::unordered_map<std::string,
                       std::deque<std::chrono::steady_clock::time_point>>
        log_;
    std::unordered_map<std::string, size_t> rejected_requests_;
    bool paused_ = false;
    std::mutex mutex_;
};

/**
 * @class Debounce
 * @brief A class that implements a debouncing mechanism for function calls.
 *
 * The `Debounce` class ensures that the given function is not invoked more
 * frequently than a specified delay interval. It postpones the function call
 * until the delay has elapsed since the last call. If a new call occurs before
 * the delay expires, the previous call is canceled and the delay starts over.
 * This is useful for situations where you want to limit the rate of function
 * invocations, such as handling user input events.
 */
class Debounce {
public:
    /**
     * @brief Constructs a Debounce object.
     *
     * @param func The function to be debounced.
     * @param delay The time delay to wait before invoking the function.
     * @param leading If true, the function will be invoked immediately on the
     * first call and then debounced for subsequent calls. If false, the
     * function will be debounced and invoked only after the delay has passed
     * since the last call.
     * @param maxWait Optional maximum wait time before invoking the function if
     * it has been called frequently. If not provided, there is no maximum wait
     * time.
     */
    Debounce(std::function<void()> func, std::chrono::milliseconds delay,
             bool leading = false,
             std::optional<std::chrono::milliseconds> maxWait = std::nullopt);

    /**
     * @brief Invokes the debounced function if the delay has elapsed since the
     * last call.
     *
     * This method schedules the function call if the delay period has passed
     * since the last call. If the leading flag is set, the function will be
     * called immediately on the first call. Subsequent calls will reset the
     * delay timer.
     */
    void operator()();

    /**
     * @brief Cancels any pending function calls.
     *
     * This method cancels any pending invocation of the function that is
     * scheduled to occur based on the debouncing mechanism.
     */
    void cancel();

    /**
     * @brief Immediately invokes the function if it is scheduled to be called.
     *
     * This method flushes any pending function calls, ensuring the function is
     * called immediately.
     */
    void flush();

    /**
     * @brief Resets the debouncer, clearing any pending function call and
     * timer.
     *
     * This method resets the internal state of the debouncer, allowing it to
     * start fresh and schedule new function calls based on the debounce delay.
     */
    void reset();

    /**
     * @brief Returns the number of times the function has been invoked.
     *
     * @return The count of function invocations.
     */
    size_t callCount() const;

private:
    /**
     * @brief Runs the function in a separate thread after the debounce delay.
     *
     * This method is used internally to handle the scheduling and execution of
     * the function after the specified delay.
     */
    void run();

    std::function<void()> func_;  ///< The function to be debounced.
    std::chrono::milliseconds
        delay_;  ///< The time delay before invoking the function.
    std::optional<std::chrono::steady_clock::time_point>
        last_call_;        ///< The timestamp of the last call.
    std::jthread thread_;  ///< A thread used to handle delayed function calls.
    mutable std::mutex
        mutex_;     ///< Mutex to protect concurrent access to internal state.
    bool leading_;  ///< Indicates if the function should be called immediately
                    ///< upon the first call.
    bool scheduled_ =
        false;  ///< Flag to track if the function is scheduled for execution.
    std::optional<std::chrono::milliseconds>
        maxWait_;  ///< Optional maximum wait time before invocation.
    size_t
        call_count_{};  ///< Counter to keep track of function call invocations.
};

/**
 * @class Throttle
 * @brief A class that provides throttling for function calls, ensuring they are
 * not invoked more frequently than a specified interval.
 *
 * This class is useful for rate-limiting function calls. It ensures that the
 * given function is not called more frequently than the specified interval.
 * Additionally, it can be configured to either throttle function calls to be
 * executed at most once per interval or to execute the function immediately
 * upon the first call and then throttle subsequent calls.
 */
class Throttle {
public:
    /**
     * @brief Constructs a Throttle object.
     *
     * @param func The function to be throttled.
     * @param interval The minimum time interval between calls to the function.
     * @param leading If true, the function will be called immediately upon the
     * first call, then throttled. If false, the function will be throttled and
     * called at most once per interval.
     * @param maxWait Optional maximum wait time before invoking the function if
     * it has been called frequently. If not provided, there is no maximum wait
     * time.
     */
    Throttle(std::function<void()> func, std::chrono::milliseconds interval,
             bool leading = false,
             std::optional<std::chrono::milliseconds> maxWait = std::nullopt);

    /**
     * @brief Invokes the throttled function if the interval has elapsed.
     *
     * This method will check if enough time has passed since the last function
     * call. If so, it will invoke the function and update the last call
     * timestamp. If the function is being invoked immediately as per the
     * leading configuration, it will be executed at once, and subsequent calls
     * will be throttled.
     */
    void operator()();

    /**
     * @brief Cancels any pending function calls.
     *
     * This method cancels any pending function invocations that are scheduled
     * to occur based on the throttling mechanism.
     */
    void cancel();

    /**
     * @brief Resets the throttle, clearing the last call timestamp and allowing
     *        the function to be invoked immediately if required.
     *
     * This method can be used to reset the throttle state, allowing the
     * function to be called immediately if the leading flag is set or to reset
     * the interval for subsequent function calls.
     */
    void reset();

    /**
     * @brief Returns the number of times the function has been called.
     *
     * @return The count of function invocations.
     */
    auto callCount() const -> size_t;

private:
    std::function<void()> func_;  ///< The function to be throttled.
    std::chrono::milliseconds
        interval_;  ///< The time interval between allowed function calls.
    std::chrono::steady_clock::time_point
        last_call_;  ///< The timestamp of the last function call.
    mutable std::mutex
        mutex_;     ///< Mutex to protect concurrent access to internal state.
    bool leading_;  ///< Indicates if the function should be called immediately
                    ///< upon first call.
    bool called_ = false;  ///< Flag to track if the function has been called.
    std::optional<std::chrono::milliseconds>
        maxWait_;  ///< Optional maximum wait time before invocation.
    size_t
        call_count_{};  ///< Counter to keep track of function call invocations.
};

}  // namespace atom::async

#endif
