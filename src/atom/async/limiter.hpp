#ifndef ATOM_ASYNC_LIMITER_HPP
#define ATOM_ASYNC_LIMITER_HPP

#include <chrono>
#include <coroutine>
#include <deque>
#include <mutex>
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
    auto get_rejected_requests(const std::string& function_name) -> size_t;

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
    void process_waiters();

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
}  // namespace atom::async

#endif