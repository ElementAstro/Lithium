/*
 * stopwatcher.cpp
 *
 * Optimized for C++20 and Pimpl pattern.
 */

#include "stopwatcher.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace atom::utils {
class StopWatcher::Impl {
public:
    Impl()= default;

    void start() {
        if (!running) {
            startTime_ = Clock::now();
            running = true;
            paused = false;
            intervals.clear();
            intervals.push_back(startTime_);
        }
    }

    void stop() {
        if (running && !paused) {
            auto stopTime = Clock::now();
            endTime_ = stopTime;
            running = false;
            intervals.push_back(stopTime);
            checkCallbacks(stopTime);
        }
    }

    void pause() {
        if (running && !paused) {
            pauseTime_ = Clock::now();
            paused = true;
            intervals.push_back(pauseTime_);
        }
    }

    void resume() {
        if (running && paused) {
            auto resumeTime = Clock::now();
            startTime_ += resumeTime - pauseTime_;
            paused = false;
            intervals.push_back(resumeTime);
        }
    }

    void reset() {
        running = false;
        paused = false;
        intervals.clear();
        callbacks.clear();
    }

    [[nodiscard]] double elapsedMilliseconds() const {
        auto endTimePoint =
            running ? (paused ? pauseTime_ : Clock::now()) : endTime_;
        return std::chrono::duration<double, std::milli>(endTimePoint -
                                                         startTime_)
            .count();
    }

    [[nodiscard]] double elapsedSeconds() const {
        return elapsedMilliseconds() / 1000.0;
    }

    [[nodiscard]] std::string elapsedFormatted() const {
        auto totalSeconds = static_cast<int>(elapsedSeconds());
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;

        std::ostringstream ss;
        ss << std::setw(2) << std::setfill('0') << hours << ":" << std::setw(2)
           << std::setfill('0') << minutes << ":" << std::setw(2)
           << std::setfill('0') << seconds;
        return ss.str();
    }

    void registerCallback(std::function<void()> callback, int milliseconds) {
        callbacks.emplace_back(std::move(callback), milliseconds);
    }

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint startTime_{}, endTime_{}, pauseTime_{};
    bool running{}, paused{};
    std::vector<TimePoint> intervals;
    std::vector<std::pair<std::function<void()>, int>> callbacks;

    void checkCallbacks(const TimePoint& currentTime) {
        for (const auto& [callback, interval] : callbacks) {
            auto targetTime = startTime_ + std::chrono::milliseconds(interval);
            if (currentTime >= targetTime) {
                callback();
            }
        }
    }
};

// StopWatcher public interface methods

StopWatcher::StopWatcher() : pImpl(std::make_unique<Impl>()) {}
StopWatcher::~StopWatcher() = default;

StopWatcher::StopWatcher(StopWatcher&&) noexcept = default;
StopWatcher& StopWatcher::operator=(StopWatcher&&) noexcept = default;

void StopWatcher::start() { pImpl->start(); }
void StopWatcher::stop() { pImpl->stop(); }
void StopWatcher::pause() { pImpl->pause(); }
void StopWatcher::resume() { pImpl->resume(); }
void StopWatcher::reset() { pImpl->reset(); }

double StopWatcher::elapsedMilliseconds() const {
    return pImpl->elapsedMilliseconds();
}
double StopWatcher::elapsedSeconds() const { return pImpl->elapsedSeconds(); }
std::string StopWatcher::elapsedFormatted() const {
    return pImpl->elapsedFormatted();
}

void StopWatcher::registerCallback(std::function<void()> callback,
                                   int milliseconds) {
    pImpl->registerCallback(std::move(callback), milliseconds);
}

}  // namespace atom::utils
