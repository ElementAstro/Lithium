/*
 * stopwatcher.hpp
 *
 * Optimized for C++20 and Pimpl pattern.
 */

#ifndef ATOM_UTILS_STOPWATCHER_HPP
#define ATOM_UTILS_STOPWATCHER_HPP

#include <functional>
#include <memory>
#include <string>

namespace atom::utils {
/**
 * @brief A class for measuring elapsed time.
 *
 * The `StopWatcher` class is used to measure the execution time of code
 * segments. It provides functionalities such as starting, stopping, pausing,
 * resuming, and resetting the timer. Additionally, it can calculate elapsed
 * time in milliseconds or seconds and can format the elapsed time as a string.
 * The class also supports registering callbacks that are triggered after a
 * specified time interval.
 */
class StopWatcher {
public:
    /**
     * @brief Default constructor for `StopWatcher`.
     *
     * Initializes the `StopWatcher` instance and prepares it for timing
     * operations.
     */
    StopWatcher();

    /**
     * @brief Destructor for `StopWatcher`.
     *
     * Cleans up resources used by the `StopWatcher` instance.
     */
    ~StopWatcher();

    /**
     * @brief Copy constructor is deleted.
     *
     * Copying `StopWatcher` objects is disallowed to prevent issues with shared
     * state.
     */
    StopWatcher(const StopWatcher&) = delete;

    /**
     * @brief Copy assignment operator is deleted.
     *
     * Copy assignment of `StopWatcher` objects is disallowed to prevent issues
     * with shared state.
     */
    auto operator=(const StopWatcher&) -> StopWatcher& = delete;

    /**
     * @brief Move constructor.
     *
     * @param other The `StopWatcher` instance to move from.
     *
     * Moves the resources from another `StopWatcher` instance to this one. The
     * other instance is left in a valid but unspecified state.
     */
    StopWatcher(StopWatcher&&) noexcept;

    /**
     * @brief Move assignment operator.
     *
     * @param other The `StopWatcher` instance to move from.
     *
     * Moves the resources from another `StopWatcher` instance to this one,
     * replacing any existing resources. The other instance is left in a valid
     * but unspecified state.
     *
     * @return A reference to this `StopWatcher` instance.
     */
    auto operator=(StopWatcher&&) noexcept -> StopWatcher&;

    /**
     * @brief Starts the timer.
     *
     * Resets the timer and begins measuring elapsed time. The timer can be
     * stopped, paused, or reset while running.
     */
    void start();

    /**
     * @brief Stops the timer.
     *
     * Stops the timer and calculates the total elapsed time. The timer can be
     * restarted using the `start` method.
     */
    void stop();

    /**
     * @brief Pauses the timer.
     *
     * Pauses the timer without resetting the elapsed time. The timer can be
     * resumed from where it was paused using the `resume` method.
     */
    void pause();

    /**
     * @brief Resumes the timer.
     *
     * Resumes the timer from the paused state. The elapsed time continues to be
     * measured from where it was paused.
     */
    void resume();

    /**
     * @brief Resets the timer.
     *
     * Resets the timer to zero. Any elapsed time is discarded, and the timer is
     * ready to be started again from scratch.
     */
    void reset();

    /**
     * @brief Gets the elapsed time in milliseconds.
     *
     * @return The elapsed time in milliseconds as a double.
     *
     * This method returns the total elapsed time since the timer was started,
     * stopped, or resumed, in milliseconds.
     */
    [[nodiscard]] auto elapsedMilliseconds() const -> double;

    /**
     * @brief Gets the elapsed time in seconds.
     *
     * @return The elapsed time in seconds as a double.
     *
     * This method returns the total elapsed time since the timer was started,
     * stopped, or resumed, in seconds.
     */
    [[nodiscard]] auto elapsedSeconds() const -> double;

    /**
     * @brief Gets the elapsed time as a formatted string.
     *
     * @return A string representation of the elapsed time.
     *
     * This method returns the total elapsed time in a human-readable format,
     * such as "hh:mm:ss".
     */
    [[nodiscard]] auto elapsedFormatted() const -> std::string;

    /**
     * @brief Registers a callback to be invoked after a specified interval.
     *
     * @param callback The function to be called after the specified interval.
     * @param milliseconds The time interval after which the callback should be
     * invoked.
     *
     * This method allows registering a callback function that will be triggered
     * after the specified number of milliseconds have passed.
     */
    void registerCallback(std::function<void()> callback, int milliseconds);

private:
    class Impl;  ///< Forward declaration of the implementation class.
    std::unique_ptr<Impl> pImpl;  ///< Pointer to the implementation details.
};
}  // namespace atom::utils

#endif  // ATOM_UTILS_STOPWATCHER_HPP
