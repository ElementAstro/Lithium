#include "exposure_timer.hpp"

#include <asio/steady_timer.hpp>
#include <chrono>
#include <mutex>

class ExposureTimer::Impl {
public:
    Impl(asio::io_context& io_context)
        : timer_(io_context),
          total_exposure_time_(0),
          remaining_time_(0),
          delay_time_(0),
          is_running_(false),
          last_tick_time_(std::chrono::high_resolution_clock::now()) {}

    void start(std::chrono::milliseconds exposure_time,
               std::function<void()> on_complete, std::function<void()> on_tick,
               std::chrono::milliseconds delay,
               std::function<void()> on_start) {
        std::lock_guard<std::mutex> lock(mutex_);
        total_exposure_time_ = exposure_time;
        remaining_time_ = exposure_time;
        delay_time_ = delay;
        on_complete_ = std::move(on_complete);
        on_tick_ = std::move(on_tick);
        on_start_ = std::move(on_start);
        is_running_ = true;
        last_tick_time_ = std::chrono::high_resolution_clock::now();
        if (on_start_) {
            on_start_();
        }
        if (delay_time_.count() > 0) {
            startDelay();
        } else {
            runTimer();
        }
    }

    void pause() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (is_running_) {
            asio::error_code ec;
            timer_.cancel(ec);
            if (ec) {
                // Handle error
                return;
            }
            is_running_ = false;
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    now - last_tick_time_);
            remaining_time_ -= elapsed;
            if (on_pause_) {
                on_pause_();
            }
        }
    }

    void resume() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!is_running_ && remaining_time_.count() > 0) {
            is_running_ = true;
            last_tick_time_ = std::chrono::high_resolution_clock::now();
            if (on_resume_) {
                on_resume_();
            }
            runTimer();
        }
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex_);
        asio::error_code ec;
        timer_.cancel(ec);
        if (ec) {
            // Handle error
            return;
        }
        is_running_ = false;
        remaining_time_ = std::chrono::milliseconds(0);
        if (on_stop_) {
            on_stop_();
        }
    }

    void reset() {
        stop();
        std::lock_guard<std::mutex> lock(mutex_);
        remaining_time_ = total_exposure_time_;
    }

    auto isRunning() const -> bool {
        std::lock_guard<std::mutex> lock(mutex_);
        return is_running_;
    }

    auto remainingTime() const -> std::chrono::milliseconds {
        std::lock_guard<std::mutex> lock(mutex_);
        return remaining_time_;
    }

    auto totalTime() const -> std::chrono::milliseconds {
        std::lock_guard<std::mutex> lock(mutex_);
        return total_exposure_time_;
    }

    void adjustTime(std::chrono::milliseconds adjustment) {
        std::lock_guard<std::mutex> lock(mutex_);
        remaining_time_ += adjustment;
        if (remaining_time_.count() < 0) {
            remaining_time_ = std::chrono::milliseconds(0);
        }
    }

    void setOnPause(std::function<void()> on_pause) {
        std::lock_guard<std::mutex> lock(mutex_);
        on_pause_ = std::move(on_pause);
    }

    void setOnStop(std::function<void()> on_stop) {
        std::lock_guard<std::mutex> lock(mutex_);
        on_stop_ = std::move(on_stop);
    }

    void setOnResume(std::function<void()> on_resume) {
        std::lock_guard<std::mutex> lock(mutex_);
        on_resume_ = std::move(on_resume);
    }

    auto progress() const -> float {
        std::lock_guard<std::mutex> lock(mutex_);
        if (total_exposure_time_.count() == 0) {
            return 0.0F;
        }
        return 100.0F * (1.0F - (static_cast<float>(remaining_time_.count()) /
                                 total_exposure_time_.count()));
    }

private:
    void startDelay() {
        timer_.expires_after(delay_time_);
        timer_.async_wait([this](const asio::error_code& error) {
            if (!error) {
                runTimer();
            }
        });
    }

    void runTimer() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (remaining_time_ <= std::chrono::milliseconds(0)) {
            is_running_ = false;
            if (on_complete_) {
                on_complete_();
            }
            return;
        }

        timer_.expires_after(std::chrono::milliseconds(100));
        timer_.async_wait([this](const asio::error_code& error) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!error) {
                auto now = std::chrono::high_resolution_clock::now();
                auto elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - last_tick_time_);
                remaining_time_ -= elapsed;
                last_tick_time_ = now;

                if (on_tick_) {
                    on_tick_();
                }

                if (remaining_time_ <= std::chrono::milliseconds(0)) {
                    remaining_time_ = std::chrono::milliseconds(0);
                    is_running_ = false;
                    if (on_complete_) {
                        on_complete_();
                    }
                } else {
                    runTimer();
                }
            }
        });
    }

    mutable std::mutex mutex_;
    asio::steady_timer timer_;
    std::chrono::milliseconds total_exposure_time_;
    std::chrono::milliseconds remaining_time_;
    std::chrono::milliseconds delay_time_;
    bool is_running_;
    std::chrono::high_resolution_clock::time_point last_tick_time_;

    std::function<void()> on_complete_;
    std::function<void()> on_tick_;
    std::function<void()> on_stop_;
    std::function<void()> on_resume_;
    std::function<void()> on_start_;
    std::function<void()> on_pause_;
};

ExposureTimer::ExposureTimer(asio::io_context& io_context)
    : impl_(std::make_unique<Impl>(io_context)) {}

ExposureTimer::~ExposureTimer() = default;

void ExposureTimer::start(std::chrono::milliseconds exposure_time,
                          std::function<void()> on_complete,
                          std::function<void()> on_tick,
                          std::chrono::milliseconds delay,
                          std::function<void()> on_start) {
    impl_->start(exposure_time, std::move(on_complete), std::move(on_tick),
                 delay, std::move(on_start));
}

void ExposureTimer::pause() { impl_->pause(); }

void ExposureTimer::resume() { impl_->resume(); }

void ExposureTimer::stop() { impl_->stop(); }

void ExposureTimer::reset() { impl_->reset(); }

bool ExposureTimer::is_running() const { return impl_->isRunning(); }

std::chrono::milliseconds ExposureTimer::remaining_time() const {
    return impl_->remainingTime();
}

std::chrono::milliseconds ExposureTimer::total_time() const {
    return impl_->totalTime();
}

void ExposureTimer::adjust_time(std::chrono::milliseconds adjustment) {
    impl_->adjustTime(adjustment);
}

void ExposureTimer::set_on_pause(std::function<void()> on_pause) {
    impl_->setOnPause(std::move(on_pause));
}

void ExposureTimer::set_on_stop(std::function<void()> on_stop) {
    impl_->setOnStop(std::move(on_stop));
}

void ExposureTimer::set_on_resume(std::function<void()> on_resume) {
    impl_->setOnResume(std::move(on_resume));
}

float ExposureTimer::progress() const { return impl_->progress(); }
