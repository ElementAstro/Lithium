#include "progress.hpp"

#include <iostream>

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

constexpr float PERCENTAGE_MULTIPLIER = 100.0f;
constexpr int MILLISECONDS_IN_A_SECOND = 1000;
constexpr int SECONDS_IN_A_MINUTE = 60;
constexpr int MILLISECONDS_IN_A_MINUTE =
    MILLISECONDS_IN_A_SECOND * SECONDS_IN_A_MINUTE;

auto getColorCode(Color color) -> std::string {
    switch (color) {
        case Color::RED:
            return "\033[31m";
        case Color::GREEN:
            return "\033[32m";
        case Color::YELLOW:
            return "\033[33m";
        case Color::BLUE:
            return "\033[34m";
        default:
            return "\033[0m";
    }
}

ProgressBar::ProgressBar(int total, int width, char completeChar,
                         char incompleteChar, bool showTimeLeft, Color color,
                         int refreshRateMs, bool showPercentage)
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
      label_("") {}

void ProgressBar::printProgressBar() {
    std::scoped_lock lock(mutex_);

    float progress = static_cast<float>(current_) / static_cast<float>(total_);
    auto pos = static_cast<int>(progress * width_);

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
        std::cout << static_cast<int>(progress * PERCENTAGE_MULTIPLIER) << " %";
    }

    if (!label_.empty()) {
        std::cout << " " << label_;
    }

    if (showTimeLeft_ && current_ > 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - startTime_)
                           .count();
        auto remaining =
            static_cast<int>((elapsed * total_) / current_ - elapsed);
        std::cout << " (ETA: " << remaining / MILLISECONDS_IN_A_MINUTE << "m "
                  << (remaining / MILLISECONDS_IN_A_SECOND) %
                         SECONDS_IN_A_MINUTE
                  << "s)";
    }

    std::cout << "\033[0m" << std::endl;  // Reset color
    std::cout << SHOW_CURSOR;
}

std::string ProgressBar::selectColorBasedOnProgress(float progress) const {
    if (progress < 0.33f) {
        return getColorCode(Color::RED);
    }
    if (progress < 0.66f) {
        return getColorCode(Color::YELLOW);
    }
    return getColorCode(Color::GREEN);
}

void ProgressBar::logEvent(const std::string& event) const {
    std::cout << "[" << event << "] at: "
              << std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::steady_clock::now().time_since_epoch())
                     .count()
              << "s" << std::endl;
}

void ProgressBar::start() {
    running_ = true;
    paused_ = false;
    startTime_ = std::chrono::steady_clock::now();
    logEvent("Started");

    future_ = std::async(std::launch::async, [this]() {
        while (running_) {
            {
                std::unique_lock lock(mutex_);
                cv_.wait(lock, [this]() { return !paused_ || !running_; });

                if (!running_)
                    break;

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

void ProgressBar::pause() {
    std::scoped_lock lock(mutex_);
    paused_ = true;
    logEvent("Paused");
}

void ProgressBar::resume() {
    {
        std::scoped_lock lock(mutex_);
        paused_ = false;
    }
    cv_.notify_one();
    logEvent("Resumed");
}

void ProgressBar::stop() {
    {
        std::scoped_lock lock(mutex_);
        running_ = false;
    }
    cv_.notify_one();
    std::cout << SHOW_CURSOR << std::endl;  // Ensure cursor visibility
    logEvent("Stopped");
}

void ProgressBar::reset() {
    std::scoped_lock lock(mutex_);
    current_ = 0;
    paused_ = false;
    running_ = true;
    cv_.notify_one();
    startTime_ = std::chrono::steady_clock::now();
    logEvent("Reset");
}

void ProgressBar::wait() {
    if (future_.valid()) {
        future_.wait();
    }
}

void ProgressBar::setCurrent(int value) {
    std::scoped_lock lock(mutex_);
    if (value <= total_) {
        current_ = value;
    }
}

void ProgressBar::setCompletionCallback(std::function<void()> callback) {
    std::scoped_lock lock(mutex_);
    completionCallback_ = std::move(callback);
}

void ProgressBar::setLabel(const std::string& label) {
    std::scoped_lock lock(mutex_);
    label_ = label;
}

auto ProgressBar::getCurrent() const -> int { return current_; }

auto ProgressBar::isRunning() const -> bool { return running_; }

auto ProgressBar::isPaused() const -> bool { return paused_; }

}  // namespace lithium::debug
