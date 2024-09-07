#include "eventloop.hpp"

EventLoop::EventLoop() : stop_flag(false) {
    loop_thread = std::thread(&EventLoop::run, this);
}

EventLoop::~EventLoop() {
    stop();
    if (loop_thread.joinable()) {
        loop_thread.join();
    }
}

void EventLoop::run() {
    while (!stop_flag) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (!tasks.empty()) {
                auto current_time = std::chrono::steady_clock::now();
                if (tasks.top().exec_time <= current_time) {
                    task = std::move(tasks.top().func);
                    tasks.pop();
                }
            }
        }
        if (task) {
            task();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Idle time
        }
    }
}

void EventLoop::stop() {
    stop_flag = true;
}

void EventLoop::schedule_periodic(std::chrono::milliseconds interval, int priority, std::function<void()> func) {
    std::thread([this, interval, priority, func] {
        while (!stop_flag) {
            post(priority, func);
            std::this_thread::sleep_for(interval);
        }
    }).detach();
}

bool EventLoop::Task::operator<(const Task& other) const {
    return exec_time > other.exec_time ||
           (exec_time == other.exec_time && priority < other.priority);
}

// Template method implementations
template <typename F, typename... Args>
auto EventLoop::post(int priority, F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> result = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(Task{[task]() { (*task)(); }, priority, std::chrono::steady_clock::now()});
    }
    return result;
}

template <typename F, typename... Args>
auto EventLoop::post(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    return post(0, std::forward<F>(f), std::forward<Args>(args)...);
}

template <typename F, typename... Args>
auto EventLoop::postDelayed(std::chrono::milliseconds delay, int priority, F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> result = task->get_future();
    auto exec_time = std::chrono::steady_clock::now() + delay;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(Task{[task]() { (*task)(); }, priority, exec_time});
    }
    return result;
}

template <typename F, typename... Args>
auto EventLoop::postDelayed(std::chrono::milliseconds delay, F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    return postDelayed(delay, 0, std::forward<F>(f), std::forward<Args>(args)...);
}
