#ifndef EVENT_LOOP_HPP
#define EVENT_LOOP_HPP

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <vector>

#ifdef USE_ASIO
#include <boost/asio.hpp>
#endif

#ifdef _WIN32
#include <winsock2.h>
#elif __linux__
#include <sys/epoll.h>
#include <sys/signalfd.h>
#endif

namespace lithium::app {

class EventLoop {
public:
    explicit EventLoop(int num_threads = 1);
    ~EventLoop();

    void run();
    void stop();

    // 提交任务 (支持优先级)
    template <typename F, typename... Args>
    auto post(int priority, F&& f,
              Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    // 无优先级任务提交
    template <typename F, typename... Args>
    auto post(F&& f,
              Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    // 延迟任务提交
    template <typename F, typename... Args>
    auto postDelayed(std::chrono::milliseconds delay, int priority, F&& f,
                     Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    template <typename F, typename... Args>
    auto postDelayed(std::chrono::milliseconds delay, F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

    // 动态调整任务优先级
    auto adjustTaskPriority(int task_id, int new_priority) -> bool;

    // 任务依赖
    template <typename F, typename G>
    void postWithDependency(F&& f, G&& dependency_task);

    // 定时任务
    void schedulePeriodic(std::chrono::milliseconds interval, int priority,
                          std::function<void()> func);

    // 增加任务取消支持
    template <typename F, typename... Args>
    auto postCancelable(F&& f,
                        std::atomic<bool>& cancel_flag) -> std::future<void>;

    // 定时器接口
    void setTimeout(std::function<void()> func,
                    std::chrono::milliseconds delay);
    void setInterval(std::function<void()> func,
                     std::chrono::milliseconds interval);

    // 事件订阅/发布
    using EventCallback = std::function<void()>;
    void subscribeEvent(const std::string& event_name,
                        const EventCallback& callback);
    void emitEvent(const std::string& event_name);

    // 文件描述符的支持（epoll 或 select）
#ifdef __linux__
    void addEpollFd(int fd) const;
    void addSignalHandler(int signal,
                          std::function<void()> handler);  // 增加信号处理
#elif _WIN32
    void add_socket_fd(SOCKET fd);
#endif

private:
    struct Task {
        std::function<void()> func;
        int priority;
        std::chrono::steady_clock::time_point execTime;
        int taskId;

        auto operator<(const Task& other) const -> bool;
    };

    std::priority_queue<Task> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_flag_;
    std::vector<std::jthread> thread_pool_;  // 线程池支持

    std::unordered_map<std::string, std::vector<EventCallback>>
        event_subscribers_;  // 事件订阅者
    std::unordered_map<int, std::function<void()>>
        signal_handlers_;  // 信号处理
    int next_task_id_ = 0;

#ifdef USE_ASIO
    boost::asio::io_context io_context_;
    std::vector<std::unique_ptr<boost::asio::steady_timer>>
        timers_;  // 定时器列表
#endif

#ifdef __linux__
    int epoll_fd_;
    std::vector<struct epoll_event> epoll_events_;
    int signal_fd_;  // 用于监听系统信号
#elif _WIN32
    fd_set read_fds;
#endif

    void workerThread();  // 用于线程池中的工作线程
    void wakeup();        // 用于唤醒 `epoll` 或 `select` 处理
};

}  // namespace lithium::app

#endif  // EVENT_LOOP_HPP