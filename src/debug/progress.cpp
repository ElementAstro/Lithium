#include "progress.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#define CLEAR_SCREEN "cls"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#else
#define CLEAR_SCREEN "clear"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"
#endif

namespace lithium::debug {

constexpr float PERCENTAGE_MULTIPLIER = 100.0F;
constexpr int MILLISECONDS_IN_A_SECOND = 1000;
constexpr int SECONDS_IN_A_MINUTE = 60;
constexpr int MILLISECONDS_IN_A_MINUTE =
    MILLISECONDS_IN_A_SECOND * SECONDS_IN_A_MINUTE;

class ProgressBar::Impl {
public:
    Impl(int total, int width, char completeChar, char incompleteChar,
         bool showTimeLeft, Color color, int refreshRateMs, bool showPercentage)
        : total_(total),
          width_(width),
          completeChar_(completeChar),
          incompleteChar_(incompleteChar),
          showTimeLeft_(showTimeLeft),
          color_(color),
          current_(0),
          running_(false),
          paused_(false),
          refreshRateMs_(refreshRateMs),
          showPercentage_(showPercentage),
          completionCallback_([]() { /* No-op */ }),
          label_("") {
        if (total_ <= 0) {
            throw std::invalid_argument(
                "Total work must be greater than zero.");
        }
        if (width_ <= 0) {
            throw std::invalid_argument("Width must be greater than zero.");
        }
    }

    ~Impl() {
        stop();
        wait();
    }

    void start() {
        bool expected = false;
        if (!running_.compare_exchange_strong(expected, true)) {
            // Already running
            return;
        }

        paused_ = false;
        current_ = 0;
        startTime_ = std::chrono::steady_clock::now();
        logEvent("Started");

        future_ = std::async(std::launch::async, [this]() {
            while (running_) {
                {
                    std::unique_lock lock(mutex_);
                    cv_.wait(lock, [this]() { return !paused_ || !running_; });

                    if (!running_) {
                        break;
                    }

                    printProgressBar();

                    if (current_ >= total_) {
                        stop();
                        if (completionCallback_) {
                            completionCallback_();
                        }
                        logEvent("Completed");
                    }
                }

                std::this_thread::sleep_for(
                    std::chrono::milliseconds(refreshRateMs_));
            }
        });
    }

    void pause() {
        if (!running_)
            return;

        paused_ = true;
        logEvent("Paused");
    }

    void resume() {
        if (!running_)
            return;

        {
            std::lock_guard lock(mutex_);
            paused_ = false;
        }
        cv_.notify_one();
        logEvent("Resumed");
    }

    void stop() {
        bool expected = true;
        if (!running_.compare_exchange_strong(expected, false)) {
            // Already stopped
            return;
        }

        cv_.notify_one();
        std::cout << SHOW_CURSOR << std::endl;  // Ensure cursor visibility
        logEvent("Stopped");
    }

    void reset() {
        std::lock_guard lock(mutex_);
        current_ = 0;
        paused_ = false;
        running_ = false;
        cv_.notify_one();
        startTime_ = std::chrono::steady_clock::now();
        logEvent("Reset");
    }

    void wait() {
        if (future_.valid()) {
            try {
                future_.wait();
            } catch (const std::exception& e) {
                std::cerr << "Exception in progress bar thread: " << e.what()
                          << std::endl;
            }
        }
    }

    void setCurrent(int value) {
        std::lock_guard lock(mutex_);
        if (value < 0) {
            current_ = 0;
        } else if (value > total_) {
            current_ = total_;
        } else {
            current_ = value;
        }
    }

    void setCompletionCallback(std::function<void()> callback) {
        std::lock_guard lock(mutex_);
        completionCallback_ = std::move(callback);
    }

    void setLabel(const std::string& label) {
        std::lock_guard lock(mutex_);
        label_ = label;
    }

    int getCurrent() const { return current_.load(); }

    bool isRunning() const { return running_.load(); }

    bool isPaused() const { return paused_.load(); }

private:
    int total_;                 ///< The total amount of work to be done.
    int width_;                 ///< The width of the progress bar.
    char completeChar_;         ///< The character representing completed work.
    char incompleteChar_;       ///< The character representing incomplete work.
    bool showTimeLeft_;         ///< Whether to show the estimated time left.
    Color color_;               ///< The color of the progress bar.
    std::atomic<int> current_;  ///< The current progress value.
    std::atomic<bool> running_;  ///< Whether the progress bar is running.
    std::atomic<bool> paused_;   ///< Whether the progress bar is paused.
    int refreshRateMs_;          ///< The refresh rate in milliseconds.
    bool showPercentage_;        ///< Whether to show the percentage completed.
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
    void printProgressBar() {
        std::lock_guard lock(mutex_);

        float progress =
            static_cast<float>(current_) / static_cast<float>(total_);
        progress = progress > 1.0f ? 1.0f : progress;
        int pos = static_cast<int>(progress * width_);

        std::cout << HIDE_CURSOR << "\033[2J\033[1;1H";
        std::cout << selectColorBasedOnProgress(progress) << "[";

        for (int i = 0; i < width_; ++i) {
            if (i < pos) {
                std::cout << completeChar_;
            } else if (i == pos) {
                std::cout << ">";
            } else {
                std::cout << incompleteChar_;
            }
        }

        std::cout << "] ";

        if (showPercentage_) {
            std::cout << std::fixed << std::setprecision(1)
                      << (progress * PERCENTAGE_MULTIPLIER) << " %";
        }

        if (!label_.empty()) {
            std::cout << " " << label_;
        }

        if (showTimeLeft_ && current_ > 0) {
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - startTime_)
                    .count();
            int remaining =
                static_cast<int>((elapsed * total_) / current_ - elapsed);
            remaining = remaining < 0 ? 0 : remaining;
            std::cout << " (ETA: " << (remaining / MILLISECONDS_IN_A_MINUTE)
                      << "m "
                      << (remaining / MILLISECONDS_IN_A_SECOND) %
                             SECONDS_IN_A_MINUTE
                      << "s)";
        }

        std::cout << "\033[0m" << std::endl;  // Reset color
        std::cout << SHOW_CURSOR;
    }

    /**
     * @brief Selects the color based on the progress.
     *
     * @param progress The current progress as a float.
     * @return The color code as a string.
     */
    std::string selectColorBasedOnProgress(float progress) const {
        if (progress < 0.33f) {
            return getColorCode(Color::RED);
        }
        if (progress < 0.66f) {
            return getColorCode(Color::YELLOW);
        }
        return getColorCode(Color::GREEN);
    }

    /**
     * @brief Logs an event.
     *
     * @param event The event to be logged.
     */
    void logEvent(const std::string& event) {
        std::lock_guard lock(mutex_);
        std::cout << "[" << event << "] at: "
                  << std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::steady_clock::now().time_since_epoch())
                         .count()
                  << "s" << std::endl;
    }

    /**
     * @brief Gets the ANSI color code for a given color enum.
     *
     * @param color The color enum.
     * @return The ANSI color code as a string.
     */
    std::string getColorCode(Color color) const {
        switch (color) {
            case Color::RED:
                return "\033[31m";
            case Color::GREEN:
                return "\033[32m";
            case Color::YELLOW:
                return "\033[33m";
            case Color::BLUE:
                return "\033[34m";
            case Color::CYAN:
                return "\033[36m";
            case Color::MAGENTA:
                return "\033[35m";
            default:
                return "\033[0m";
        }
    }
};

ProgressBar::ProgressBar(int total, int width, char completeChar,
                         char incompleteChar, bool showTimeLeft, Color color,
                         int refreshRateMs, bool showPercentage)
    : impl_(std::make_unique<Impl>(total, width, completeChar, incompleteChar,
                                   showTimeLeft, color, refreshRateMs,
                                   showPercentage)) {}

ProgressBar::~ProgressBar() = default;

void ProgressBar::start() { impl_->start(); }

void ProgressBar::pause() { impl_->pause(); }

void ProgressBar::resume() { impl_->resume(); }

void ProgressBar::stop() { impl_->stop(); }

void ProgressBar::reset() { impl_->reset(); }

void ProgressBar::wait() { impl_->wait(); }

void ProgressBar::setCurrent(int value) { impl_->setCurrent(value); }

void ProgressBar::setLabel(const std::string& label) { impl_->setLabel(label); }

void ProgressBar::setCompletionCallback(std::function<void()> callback) {
    impl_->setCompletionCallback(std::move(callback));
}

int ProgressBar::getCurrent() const { return impl_->getCurrent(); }

bool ProgressBar::isRunning() const { return impl_->isRunning(); }

bool ProgressBar::isPaused() const { return impl_->isPaused(); }

}  // namespace lithium::debug
