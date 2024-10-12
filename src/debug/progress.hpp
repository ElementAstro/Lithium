#ifndef LITHIUM_DEBUG_PROGRESS_HPP
#define LITHIUM_DEBUG_PROGRESS_HPP

#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <string>

namespace lithium::debug {

/**
 * @brief Enum class representing different colors for the progress bar.
 */
enum class Color { RED, GREEN, YELLOW, BLUE, DEFAULT };

/**
 * @brief A class representing a progress bar for tracking progress of tasks.
 */
class ProgressBar {
public:
    /**
     * @brief Default refresh rate in milliseconds.
     */
    static constexpr int DEFAULT_REFRESH_RATE_MS = 100;

    /**
     * @brief Constructs a ProgressBar object.
     *
     * @param total The total amount of work to be done.
     * @param width The width of the progress bar.
     * @param completeChar The character representing completed work.
     * @param incompleteChar The character representing incomplete work.
     * @param showTimeLeft Whether to show the estimated time left.
     * @param color The color of the progress bar.
     * @param refreshRateMs The refresh rate in milliseconds.
     * @param showPercentage Whether to show the percentage completed.
     */
    ProgressBar(int total, int width, char completeChar = '=',
                char incompleteChar = '-', bool showTimeLeft = true,
                Color color = Color::DEFAULT,
                int refreshRateMs = DEFAULT_REFRESH_RATE_MS,
                bool showPercentage = true);

    /**
     * @brief Starts the progress bar.
     */
    void start();

    /**
     * @brief Pauses the progress bar.
     */
    void pause();

    /**
     * @brief Resumes the progress bar.
     */
    void resume();

    /**
     * @brief Stops the progress bar.
     */
    void stop();

    /**
     * @brief Resets the progress bar.
     */
    void reset();

    /**
     * @brief Waits for the progress bar to complete.
     */
    void wait();

    /**
     * @brief Sets the current progress value.
     *
     * @param value The current progress value.
     */
    void setCurrent(int value);

    /**
     * @brief Sets the label for the progress bar.
     *
     * @param label The label to be displayed.
     */
    void setLabel(const std::string& label);

    /**
     * @brief Sets the callback function to be called upon completion.
     *
     * @param callback The callback function.
     */
    void setCompletionCallback(std::function<void()> callback);

    /**
     * @brief Gets the current progress value.
     *
     * @return The current progress value.
     */
    [[nodiscard]] auto getCurrent() const -> int;

    /**
     * @brief Checks if the progress bar is running.
     *
     * @return True if the progress bar is running, false otherwise.
     */
    [[nodiscard]] auto isRunning() const -> bool;

    /**
     * @brief Checks if the progress bar is paused.
     *
     * @return True if the progress bar is paused, false otherwise.
     */
    [[nodiscard]] auto isPaused() const -> bool;

private:
    int total_;            ///< The total amount of work to be done.
    int width_;            ///< The width of the progress bar.
    char completeChar_;    ///< The character representing completed work.
    char incompleteChar_;  ///< The character representing incomplete work.
    bool showTimeLeft_;    ///< Whether to show the estimated time left.
    Color color_;          ///< The color of the progress bar.
    int current_;          ///< The current progress value.
    bool running_;         ///< Whether the progress bar is running.
    bool paused_;          ///< Whether the progress bar is paused.
    int refreshRateMs_;    ///< The refresh rate in milliseconds.
    bool showPercentage_;  ///< Whether to show the percentage completed.
    std::chrono::time_point<std::chrono::steady_clock>
        startTime_;  ///< The start time of the progress bar.
    std::future<void>
        future_;        ///< The future object for asynchronous operations.
    std::mutex mutex_;  ///< The mutex for thread safety.
    std::condition_variable
        cv_;  ///< The condition variable for synchronization.
    std::function<void()> completionCallback_;  ///< The callback function to be
                                                ///< called upon completion.
    std::string label_;  ///< The label for the progress bar.

    /**
     * @brief Prints the progress bar.
     */
    void printProgressBar();

    /**
     * @brief Selects the color based on the progress.
     *
     * @param progress The current progress as a float.
     * @return The color as a string.
     */
    [[nodiscard]] auto selectColorBasedOnProgress(float progress) const
        -> std::string;

    /**
     * @brief Logs an event.
     *
     * @param event The event to be logged.
     */
    void logEvent(const std::string& event) const;
};

}  // namespace lithium::debug

#endif
