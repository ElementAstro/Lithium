#ifndef EXPOSURE_TIMER_H
#define EXPOSURE_TIMER_H

#include <asio.hpp>
#include <chrono>
#include <functional>

class ExposureTimer {
public:
    ExposureTimer(asio::io_context& io_context);

    void start(std::chrono::milliseconds exposure_time,
               std::function<void()> on_complete,
               std::function<void()> on_tick = nullptr,
               std::chrono::milliseconds delay = std::chrono::milliseconds(0),
               std::function<void()> on_start = nullptr);

    void pause();
    void resume();
    void stop();
    void reset();

    bool is_running() const;
    std::chrono::milliseconds remaining_time() const;
    std::chrono::milliseconds total_time() const;
    void adjust_time(std::chrono::milliseconds adjustment);

    void set_on_pause(std::function<void()> on_pause);
    void set_on_stop(std::function<void()> on_stop);
    void set_on_resume(std::function<void()> on_resume);

    float progress() const;

private:
    void start_delay();
    void run_timer();

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

#endif  // EXPOSURE_TIMER_H
