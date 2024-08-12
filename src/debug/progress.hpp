#ifndef LITHIUM_DEBUG_PROGRESS_HPP
#define LITHIUM_DEBUG_PROGRESS_HPP

#include <chrono>
#include <condition_variable>
#include <future>
#include <mutex>

namespace lithium::debug {

enum class Color { Red, Green, Yellow, Blue, Default };

class ProgressBar {
public:
    ProgressBar(int total, int width, char completeChar, char incompleteChar,
                bool showTimeLeft, Color color);

    void start();
    void pause();
    void resume();
    void stop();
    void reset();
    void wait();
    [[nodiscard]] auto getCurrent() const -> int;

private:
    int total;
    int width;
    char completeChar;
    char incompleteChar;
    bool showTimeLeft;
    Color color;
    int current;
    bool running;
    bool paused;
    std::chrono::time_point<std::chrono::steady_clock> start_time;
    std::future<void> future;
    std::mutex mutex;
    std::condition_variable cv;

    void printProgressBar();
};

}  // namespace lithium::debug

#endif
