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
    Impl() = default;

    void start() {
        if (!running_) {
            startTime_ = Clock::now();
            running_ = true;
            paused_ = false;
            intervals_.clear();
            intervals_.push_back(startTime_);
        }
    }

    void stop() {
        if (running_ && !paused_) {
            auto stopTime = Clock::now();
            endTime_ = stopTime;
            running_ = false;
            intervals_.push_back(stopTime);
            checkCallbacks(stopTime);
        }
    }

    void pause() {
        if (running_ && !paused_) {
            pauseTime_ = Clock::now();
            paused_ = true;
            intervals_.push_back(pauseTime_);
        }
    }

    void resume() {
        if (running_ && paused_) {
            auto resumeTime = Clock::now();
            startTime_ += resumeTime - pauseTime_;
            paused_ = false;
            intervals_.push_back(resumeTime);
        }
    }

    void reset() {
        running_ = false;
        paused_ = false;
        intervals_.clear();
        callbacks_.clear();
    }

    [[nodiscard]] auto elapsedMilliseconds() const -> double {
        auto endTimePoint =
            running_ ? (paused_ ? pauseTime_ : Clock::now()) : endTime_;
        return std::chrono::duration<double, std::milli>(endTimePoint -
                                                         startTime_)
            .count();
    }

    [[nodiscard]] auto elapsedSeconds() const -> double {
        return elapsedMilliseconds() / K_MILLISECONDS_PER_SECOND;
    }

    [[nodiscard]] auto elapsedFormatted() const -> std::string {
        auto totalSeconds = static_cast<int>(elapsedSeconds());
        int hours = totalSeconds / K_SECONDS_PER_HOUR;
        int minutes =
            (totalSeconds % K_SECONDS_PER_HOUR) / K_SECONDS_PER_MINUTE;
        int seconds = totalSeconds % K_SECONDS_PER_MINUTE;

        std::ostringstream stream;
        stream << std::setw(2) << std::setfill('0') << hours << ":"
               << std::setw(2) << std::setfill('0') << minutes << ":"
               << std::setw(2) << std::setfill('0') << seconds;
        return stream.str();
    }

    void registerCallback(std::function<void()> callback, int milliseconds) {
        callbacks_.emplace_back(std::move(callback), milliseconds);
    }

private:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    TimePoint startTime_, endTime_, pauseTime_;
    bool running_{}, paused_{};
    std::vector<TimePoint> intervals_;
    std::vector<std::pair<std::function<void()>, int>> callbacks_;

    static constexpr int K_MILLISECONDS_PER_SECOND = 1000;
    static constexpr int K_SECONDS_PER_MINUTE = 60;
    static constexpr int K_SECONDS_PER_HOUR = 3600;

    void checkCallbacks(const TimePoint& currentTime) {
        for (const auto& [callback, interval] : callbacks_) {
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
auto StopWatcher::operator=(StopWatcher&&) noexcept -> StopWatcher& = default;

auto StopWatcher::start() -> void { pImpl->start(); }
auto StopWatcher::stop() -> void { pImpl->stop(); }
auto StopWatcher::pause() -> void { pImpl->pause(); }
auto StopWatcher::resume() -> void { pImpl->resume(); }
auto StopWatcher::reset() -> void { pImpl->reset(); }

auto StopWatcher::elapsedMilliseconds() const -> double {
    return pImpl->elapsedMilliseconds();
}
auto StopWatcher::elapsedSeconds() const -> double {
    return pImpl->elapsedSeconds();
}
auto StopWatcher::elapsedFormatted() const -> std::string {
    return pImpl->elapsedFormatted();
}

auto StopWatcher::registerCallback(std::function<void()> callback,
                                   int milliseconds) -> void {
    pImpl->registerCallback(std::move(callback), milliseconds);
}

}  // namespace atom::utils