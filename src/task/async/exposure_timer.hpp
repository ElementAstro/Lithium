#ifndef EXPOSURE_TIMER_H
#define EXPOSURE_TIMER_H

#include <chrono>
#include <functional>
#include <memory>

namespace asio {
class io_context;
}

class ExposureTimer {
public:
    ExposureTimer(asio::io_context& io_context);
    ~ExposureTimer();

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
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif  // EXPOSURE_TIMER_H