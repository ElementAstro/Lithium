/**
 * @file progress.hpp
 * @brief A robust and feature-rich progress bar for tracking task progress.
 */

#ifndef LITHIUM_DEBUG_PROGRESS_HPP
#define LITHIUM_DEBUG_PROGRESS_HPP

#include <functional>
#include <memory>
#include <string>

namespace lithium::debug {

/**
 * @brief Enum class representing different colors for the progress bar.
 */
enum class Color { RED, GREEN, YELLOW, BLUE, CYAN, MAGENTA, DEFAULT };

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
     * @brief Destructor to ensure proper resource cleanup.
     */
    ~ProgressBar();

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
    [[nodiscard]] int getCurrent() const;

    /**
     * @brief Checks if the progress bar is running.
     *
     * @return True if the progress bar is running, false otherwise.
     */
    [[nodiscard]] bool isRunning() const;

    /**
     * @brief Checks if the progress bar is paused.
     *
     * @return True if the progress bar is paused, false otherwise.
     */
    [[nodiscard]] bool isPaused() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium::debug

#endif