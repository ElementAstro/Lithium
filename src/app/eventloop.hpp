#ifndef EVENT_LOOP_HPP
#define EVENT_LOOP_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    void run();
    void stop();

    template <typename F, typename... Args>
    auto post(int priority, F&& f,
              Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    template <typename F, typename... Args>
    auto post(F&& f,
              Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    template <typename F, typename... Args>
    auto postDelayed(std::chrono::milliseconds delay, int priority, F&& f,
                     Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    template <typename F, typename... Args>
    auto postDelayed(std::chrono::milliseconds delay, F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    void schedule_periodic(std::chrono::milliseconds interval, int priority,
                           std::function<void()> func);

private:
    struct Task {
        std::function<void()> func;
        int priority;
        std::chrono::steady_clock::time_point exec_time;

        bool operator<(const Task& other) const;
    };

    std::priority_queue<Task> tasks;
    std::mutex queue_mutex;
    std::atomic<bool> stop_flag;
    std::thread loop_thread;
};

#endif  // EVENT_LOOP_HPP