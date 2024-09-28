#include "lodash.hpp"

#include <thread>

#include "atom/log/loguru.hpp"

// Throttle类实现
Throttle::Throttle(std::function<void()> func,
                   std::chrono::milliseconds interval, bool leading,
                   bool trailing)
    : func_(std::move(func)),
      interval_(interval),
      leading_(leading),
      trailing_(trailing),
      lastCall_(std::chrono::steady_clock::now() - interval) {}

void Throttle::operator()() {
    auto now = std::chrono::steady_clock::now();
    std::lock_guard lock(mutex_);
    handleThrottle(now);
}

void Throttle::reset() {
    std::lock_guard lock(mutex_);
    lastCall_ = std::chrono::steady_clock::now() - interval_;
    isScheduled_ = false;
    LOG_F(INFO, "Throttle reset.");
}

void Throttle::setInterval(std::chrono::milliseconds newInterval) {
    std::lock_guard lock(mutex_);
    interval_ = newInterval;
    LOG_F(INFO, "Throttle interval updated to {} milliseconds.",
          newInterval.count());
}

void Throttle::cancel() {
    std::lock_guard lock(mutex_);
    isScheduled_ = false;
    LOG_F(INFO, "Throttle function call canceled.");
}

void Throttle::handleThrottle(std::chrono::steady_clock::time_point now) {
    if (leading_ && !hasExecutedFirstCall_) {
        func_();
        lastCall_ = now;
        hasExecutedFirstCall_ = true;
    } else if (now - lastCall_ >= interval_) {
        func_();
        lastCall_ = now;
        if (trailing_) {
            isScheduled_ = false;
        }
    } else if (trailing_ && !isScheduled_) {
        isScheduled_ = true;
        auto delay = interval_ - (now - lastCall_);
        std::thread([this, delay]() {
            std::this_thread::sleep_for(delay);
            std::lock_guard lock(this->mutex_);
            if (this->isScheduled_) {
                this->func_();
                this->isScheduled_ = false;
                this->lastCall_ = std::chrono::steady_clock::now();
            }
        }).detach();
    } else {
        LOG_F(INFO, "Throttled: Function call skipped.");
    }
}

// Debounce类实现
Debounce::Debounce(std::function<void()> func,
                   std::chrono::milliseconds interval)
    : func_(std::move(func)), interval_(interval) {}

void Debounce::operator()() {
    std::lock_guard lock(mutex_);
    handleDebounce();
}

void Debounce::reset() {
    std::lock_guard lock(mutex_);
    isScheduled_ = false;
    LOG_F(INFO, "Debounce reset.");
}

void Debounce::setInterval(std::chrono::milliseconds newInterval) {
    std::lock_guard lock(mutex_);
    interval_ = newInterval;
    LOG_F(INFO, "Debounce interval updated to {} milliseconds.",
          newInterval.count());
}

void Debounce::cancel() {
    std::lock_guard lock(mutex_);
    isScheduled_ = false;
    LOG_F(INFO, "Debounce function call canceled.");
}

void Debounce::handleDebounce() {
    if (isScheduled_) {
        cancel();
    }

    isScheduled_ = true;
    std::thread([this]() {
        std::this_thread::sleep_for(interval_);
        std::lock_guard lock(this->mutex_);
        if (this->isScheduled_) {
            this->func_();
            this->isScheduled_ = false;
        }
    }).detach();
}
