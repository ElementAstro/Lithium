#include "exposure_timer.hpp"

#include <iostream>


ExposureTimer::ExposureTimer(asio::io_context& io_context)
    : timer_(io_context),
      total_exposure_time_(std::chrono::milliseconds(0)),
      remaining_time_(std::chrono::milliseconds(0)),
      delay_time_(std::chrono::milliseconds(0)),
      is_running_(false),
      last_tick_time_(std::chrono::high_resolution_clock::now()) {}

void ExposureTimer::start(std::chrono::milliseconds exposure_time,
                          std::function<void()> on_complete,
                          std::function<void()> on_tick,
                          std::chrono::milliseconds delay,
                          std::function<void()> on_start) {
    total_exposure_time_ = exposure_time;
    remaining_time_ = exposure_time;
    delay_time_ = delay;
    on_complete_ = on_complete;
    on_tick_ = on_tick;
    on_start_ = on_start;
    is_running_ = true;
    last_tick_time_ = std::chrono::high_resolution_clock::now();
    if (on_start_) {
        on_start_();
    }
    if (delay_time_ > std::chrono::milliseconds(0)) {
        start_delay();
    } else {
        run_timer();
    }
}

void ExposureTimer::pause() {
    if (is_running_) {
        timer_.cancel();
        is_running_ = false;
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_tick_time_);
        remaining_time_ -= elapsed;
        if (on_pause_) {
            on_pause_();
        }
    }
}

void ExposureTimer::resume() {
    if (!is_running_ && remaining_time_ > std::chrono::milliseconds(0)) {
        is_running_ = true;
        last_tick_time_ = std::chrono::high_resolution_clock::now();
        if (on_resume_) {
            on_resume_();
        }
        run_timer();
    }
}

void ExposureTimer::stop() {
    timer_.cancel();
    is_running_ = false;
    remaining_time_ = std::chrono::milliseconds(0);
    if (on_stop_) {
        on_stop_();
    }
}

void ExposureTimer::reset() {
    stop();
    remaining_time_ = total_exposure_time_;
}

bool ExposureTimer::is_running() const { return is_running_; }

std::chrono::milliseconds ExposureTimer::remaining_time() const {
    return remaining_time_;
}

std::chrono::milliseconds ExposureTimer::total_time() const {
    return total_exposure_time_;
}

void ExposureTimer::adjust_time(std::chrono::milliseconds adjustment) {
    remaining_time_ += adjustment;
    if (remaining_time_ < std::chrono::milliseconds(0)) {
        remaining_time_ = std::chrono::milliseconds(0);
    }
}

void ExposureTimer::set_on_pause(std::function<void()> on_pause) {
    on_pause_ = on_pause;
}

void ExposureTimer::set_on_stop(std::function<void()> on_stop) {
    on_stop_ = on_stop;
}

void ExposureTimer::set_on_resume(std::function<void()> on_resume) {
    on_resume_ = on_resume;
}

float ExposureTimer::progress() const {
    if (total_exposure_time_.count() == 0)
        return 0.0f;
    return 100.0f * (1.0f - (static_cast<float>(remaining_time_.count()) /
                             total_exposure_time_.count()));
}

void ExposureTimer::start_delay() {
    timer_.expires_after(delay_time_);
    timer_.async_wait([this](const asio::error_code& error) {
        if (!error) {
            run_timer();
        }
    });
}

void ExposureTimer::run_timer() {
    if (remaining_time_ <= std::chrono::milliseconds(0)) {
        is_running_ = false;
        if (on_complete_) {
            on_complete_();
        }
        return;
    }

    timer_.expires_after(std::chrono::milliseconds(100));
    timer_.async_wait([this](const asio::error_code& error) {
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
                run_timer();
            }
        }
    });
}
