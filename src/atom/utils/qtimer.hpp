#ifndef ATOM_UTILS_QTIMER_HPP
#define ATOM_UTILS_QTIMER_HPP

#include <chrono>
#include <cstdint>
#include <optional>

namespace atom::utils {
/**
 * @brief Class to measure elapsed time using std::chrono.
 *
 * This class provides functionality to measure elapsed time in various units
 * (nanoseconds, microseconds, milliseconds, seconds, minutes, hours). It uses
 * std::chrono for precise time measurements.
 */
class ElapsedTimer {
public:
    using Clock = std::chrono::steady_clock;
    using Nanoseconds = std::chrono::nanoseconds;
    using Microseconds = std::chrono::microseconds;
    using Milliseconds = std::chrono::milliseconds;
    using Seconds = std::chrono::seconds;
    using Minutes = std::chrono::minutes;
    using Hours = std::chrono::hours;

    /**
     * @brief Default constructor.
     *
     * Initializes the timer. The timer is initially not started.
     */
    ElapsedTimer();

    /**
     * @brief Start or restart the timer.
     *
     * Sets the start time of the timer to the current time.
     */
    void start();

    /**
     * @brief Invalidate the timer.
     *
     * Resets the start time of the timer to an invalid state.
     */
    void invalidate();

    /**
     * @brief Check if the timer has been started and is valid.
     *
     * @return true if the timer is valid and started, false otherwise.
     */
    [[nodiscard]] auto isValid() const -> bool;

    /**
     * @brief Get elapsed time in nanoseconds.
     *
     * @return Elapsed time in nanoseconds since the timer was started.
     *         Returns 0 if the timer is not valid.
     */
    [[nodiscard]] auto elapsedNs() const -> int64_t;

    /**
     * @brief Get elapsed time in microseconds.
     *
     * @return Elapsed time in microseconds since the timer was started.
     *         Returns 0 if the timer is not valid.
     */
    [[nodiscard]] auto elapsedUs() const -> int64_t;

    /**
     * @brief Get elapsed time in milliseconds.
     *
     * @return Elapsed time in milliseconds since the timer was started.
     *         Returns 0 if the timer is not valid.
     */
    [[nodiscard]] auto elapsedMs() const -> int64_t;

    /**
     * @brief Get elapsed time in seconds.
     *
     * @return Elapsed time in seconds since the timer was started.
     *         Returns 0 if the timer is not valid.
     */
    [[nodiscard]] auto elapsedSec() const -> int64_t;

    /**
     * @brief Get elapsed time in minutes.
     *
     * @return Elapsed time in minutes since the timer was started.
     *         Returns 0 if the timer is not valid.
     */
    [[nodiscard]] auto elapsedMin() const -> int64_t;

    /**
     * @brief Get elapsed time in hours.
     *
     * @return Elapsed time in hours since the timer was started.
     *         Returns 0 if the timer is not valid.
     */
    [[nodiscard]] auto elapsedHrs() const -> int64_t;

    /**
     * @brief Get elapsed time in milliseconds (same as elapsedMs).
     *
     * @return Elapsed time in milliseconds since the timer was started.
     *         Returns 0 if the timer is not valid.
     */
    [[nodiscard]] auto elapsed() const -> int64_t;

    /**
     * @brief Check if a specified duration (in milliseconds) has passed.
     *
     * @param ms Duration in milliseconds to check against elapsed time.
     * @return true if the specified duration has passed, false otherwise.
     */
    [[nodiscard]] auto hasExpired(int64_t ms) const -> bool;

    /**
     * @brief Get the remaining time until the specified duration (in
     * milliseconds) has passed.
     *
     * @param ms Duration in milliseconds to check against elapsed time.
     * @return Remaining time in milliseconds until the specified duration
     * passes. Returns 0 if the duration has already passed or the timer is
     * invalid.
     */
    [[nodiscard]] auto remainingTimeMs(int64_t ms) const -> int64_t;

    /**
     * @brief Get the current absolute time in milliseconds since epoch.
     *
     * @return Current time in milliseconds since epoch.
     */
    static auto currentTimeMs() -> int64_t;

    /**
     * @brief Compare two ElapsedTimer objects.
     *
     * @param other Another ElapsedTimer object to compare against.
     * @return true if this timer's start time is earlier than the other timer's
     * start time. false otherwise.
     */
    auto operator<(const ElapsedTimer& other) const -> bool;

    /**
     * @brief Compare two ElapsedTimer objects.
     *
     * @param other Another ElapsedTimer object to compare against.
     * @return true if this timer's start time is later than the other timer's
     * start time. false otherwise.
     */
    auto operator>(const ElapsedTimer& other) const -> bool;

    /**
     * @brief Compare two ElapsedTimer objects.
     *
     * @param other Another ElapsedTimer object to compare against.
     * @return true if this timer's start time is earlier than or equal to the
     * other timer's start time. false otherwise.
     */
    auto operator<=(const ElapsedTimer& other) const -> bool;

    /**
     * @brief Compare two ElapsedTimer objects.
     *
     * @param other Another ElapsedTimer object to compare against.
     * @return true if this timer's start time is later than or equal to the
     * other timer's start time. false otherwise.
     */
    auto operator>=(const ElapsedTimer& other) const -> bool;

    /**
     * @brief Compare two ElapsedTimer objects for equality.
     *
     * @param other Another ElapsedTimer object to compare against.
     * @return true if this timer's start time is equal to the other timer's
     * start time. false otherwise.
     */
    auto operator==(const ElapsedTimer& other) const -> bool;

    /**
     * @brief Compare two ElapsedTimer objects for inequality.
     *
     * @param other Another ElapsedTimer object to compare against.
     * @return true if this timer's start time is not equal to the other timer's
     * start time. false otherwise.
     */
    auto operator!=(const ElapsedTimer& other) const -> bool;

private:
    std::optional<Clock::time_point> start_time_;  ///< Start time of the timer.
};
}  // namespace atom::utils

#endif  // ATOM_UTILS_QTIMER_HPP
