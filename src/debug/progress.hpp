#ifndef LITHIUM_DEBUG_PROGRESS_HPP
#define LITHIUM_DEBUG_PROGRESS_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>
#include <string>

namespace lithium::debug {

/**
 * @enum Color
 * @brief Specifies colors for the progress bar.
 */
enum class Color {
    Default,  ///< Default color
    Red,      ///< Red color
    Green,    ///< Green color
    Yellow,   ///< Yellow color
    Blue      ///< Blue color
};

/**
 * @brief Get the color code for the specified color.
 * @param color The color enum value.
 * @return The corresponding color code as a string.
 */
std::string getColorCode(Color color);

/**
 * @brief Print a progress bar to the console.
 *
 * @param current The current progress value.
 * @param total The total value for completion.
 * @param width The width of the progress bar.
 * @param completeChar The character representing completed progress.
 * @param incompleteChar The character representing incomplete progress.
 * @param showTimeLeft Flag to show the estimated time left.
 * @param start The start time point.
 * @param color The color of the progress bar.
 */
void printProgressBar(int current, int total, int width, char completeChar,
                      char incompleteChar, bool showTimeLeft,
                      std::chrono::time_point<std::chrono::steady_clock> start,
                      Color color);

/**
 * @class ProgressBar
 * @brief A class to manage and display a progress bar.
 */
class ProgressBar {
public:
    /**
     * @brief Construct a new ProgressBar object.
     *
     * @param total The total value for completion.
     * @param width The width of the progress bar.
     * @param completeChar The character representing completed progress.
     * @param incompleteChar The character representing incomplete progress.
     * @param showTimeLeft Flag to show the estimated time left.
     * @param color The color of the progress bar.
     */
    ProgressBar(int total, int width, char completeChar, char incompleteChar,
                bool showTimeLeft, Color color);

    /**
     * @brief Start the progress bar.
     */
    void start();

    /**
     * @brief Pause the progress bar.
     */
    void pause();

    /**
     * @brief Resume the progress bar.
     */
    void resume();

    /**
     * @brief Stop the progress bar.
     */
    void stop();

    /**
     * @brief Reset the progress bar.
     */
    void reset();

    /**
     * @brief Wait for the progress bar to complete.
     */
    void wait();

private:
    int total;            ///< The total value for completion.
    int width;            ///< The width of the progress bar.
    char completeChar;    ///< The character representing completed progress.
    char incompleteChar;  ///< The character representing incomplete progress.
    bool showTimeLeft;    ///< Flag to show the estimated time left.
    Color color;          ///< The color of the progress bar.
    std::atomic<int> current;  ///< The current progress value.
    std::atomic<bool>
        running;  ///< Flag to indicate if the progress bar is running.
    std::atomic<bool>
        paused;  ///< Flag to indicate if the progress bar is paused.
    std::atomic<bool>
        interrupted;  ///< Flag to indicate if the progress bar is interrupted.
    std::chrono::time_point<std::chrono::steady_clock>
        start_time;            ///< The start time point.
    std::future<void> future;  ///< The future object for asynchronous progress
                               ///< bar management.
    std::mutex mutex;          ///< The mutex for thread safety.
    std::condition_variable
        cv;  ///< The condition variable for thread synchronization.
};

}  // namespace lithium::debug

#endif