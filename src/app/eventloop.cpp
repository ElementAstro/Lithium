#include "eventloop.hpp"

#ifdef __linux__
#include <sys/eventfd.h>
#include <unistd.h>
#elif _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#include "atom/log/loguru.hpp"

namespace lithium::app {
EventLoop::EventLoop(int num_threads) : stop_flag_(false) {
#ifdef __linux__
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1) {
        ABORT_F("Failed to create epoll file descriptor");
        exit(EXIT_FAILURE);
    }
    epoll_events_.resize(10);
#elif _WIN32
    FD_ZERO(&read_fds);
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

#ifdef USE_ASIO
    // Boost.Asio 初始化
    for (int i = 0; i < num_threads; ++i) {
        thread_pool_.emplace_back([this] { io_context_.run(); });
    }
#else
    // 初始化线程池
    for (int i = 0; i < num_threads; ++i) {
        thread_pool_.emplace_back(&EventLoop::workerThread, this);
    }
#endif
}

EventLoop::~EventLoop() {
    stop();
    for (auto& thread : thread_pool_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
#ifdef __linux__
    close(epoll_fd_);
    if (signal_fd_ != -1) {
        close(signal_fd_);
    }
#elif _WIN32
    WSACleanup();
#endif
}

void EventLoop::run() {
    stop_flag_.store(false);
#ifndef USE_ASIO
    workerThread();
#endif
}

void EventLoop::workerThread() {
    while (!stop_flag_.load()) {
        std::function<void()> task;
        {
            std::unique_lock lock(queue_mutex_);
            if (!tasks_.empty()) {
                auto currentTime = std::chrono::steady_clock::now();
                if (tasks_.top().execTime <= currentTime) {
                    task = tasks_.top().func;
                    tasks_.pop();
                }
            }
        }

        if (task) {
            task();
        } else {
#ifdef __linux__
            int nfds = epoll_wait(epoll_fd_, epoll_events_.data(),
                                  epoll_events_.size(), 10);
            if (nfds == -1) {
                ABORT_F("Epoll wait failed");
            } else if (nfds > 0) {
                for (int i = 0; i < nfds; ++i) {
                    int fd = epoll_events_[i].data.fd;
                    if (fd == signal_fd_) {
                        // 处理信号事件
                        uint64_t sigVal;
                        read(signal_fd_, &sigVal, sizeof(uint64_t));
                        auto it = signal_handlers_.find(sigVal);
                        if (it != signal_handlers_.end()) {
                            it->second();
                        }
                    } else {
                        // 处理文件描述符事件
                    }
                }
            }
#elif _WIN32
            timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 10000;  // 10ms
            fd_set tmp_fds = read_fds;
            int result = select(0, &tmp_fds, nullptr, nullptr, &timeout);
            if (result > 0) {
                for (u_int i = 0; i < tmp_fds.fd_count; ++i) {
                    SOCKET fd = tmp_fds.fd_array[i];
                    // 处理 socket 事件
                }
            }
#endif
            std::this_thread::sleep_for(
                std::chrono::milliseconds(10));  // Idle time
        }
    }
}

void EventLoop::stop() {
    stop_flag_.store(true);
    wakeup();
}

template <typename F, typename... Args>
auto EventLoop::post(int priority, F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> result = task->get_future();
    {
        std::unique_lock lock(queue_mutex_);
        tasks_.emplace(Task{(*task)(), priority,
                            std::chrono::steady_clock::now(), next_task_id_++});
    }
#ifdef USE_ASIO
    io_context_.post(task { (*task)(); });
#else
    condition_.notify_one();  // 通知等待中的线程
#endif
    return result;
}

template <typename F, typename... Args>
auto EventLoop::postDelayed(std::chrono::milliseconds delay, int priority,
                            F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> result = task->get_future();
    auto execTime = std::chrono::steady_clock::now() + delay;
#ifdef USE_ASIO
    auto timer =
        std::make_unique<boost::asio::steady_timer>(io_context_, delay);
    timers_.emplace_back(std::move(timer));
    timers_.back()->async_wait([task](const boost::system::error_code& ec) {
        if (!ec) {
            (*task)();
        }
    });
#else
    {
        std::unique_lock lock(queue_mutex_);
        tasks_.emplace(
            Task{[task]() { (*task)(); }, priority, execTime, next_task_id_++});
    }
    condition_.notify_one();
#endif
    return result;
}

template <typename F, typename... Args>
auto EventLoop::postDelayed(std::chrono::milliseconds delay, F&& f,
                            Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    return postDelayed(delay, 0, std::forward<F>(f),
                       std::forward<Args>(args)...);
}

auto EventLoop::adjustTaskPriority(int task_id, int new_priority) -> bool {
    std::unique_lock lock(queue_mutex_);
    std::priority_queue<Task> newQueue;

    bool found = false;
    while (!tasks_.empty()) {
        Task task = std::move(const_cast<Task&>(tasks_.top()));
        tasks_.pop();
        if (task.taskId == task_id) {
            task.priority = new_priority;
            found = true;
        }
        newQueue.push(std::move(task));
    }

    tasks_ = std::move(newQueue);
    return found;
}

template <typename F, typename G>
void EventLoop::postWithDependency(F&& f, G&& dependency_task) {
    std::future<void> dependency = dependency_task.get_future();
    std::thread([this, f = std::forward<F>(f),
                 dependency = std::move(dependency)]() mutable {
        dependency.wait();   // 等待依赖任务完成
        post(std::move(f));  // 执行依赖任务完成后的任务
    }).detach();
}

void EventLoop::subscribeEvent(const std::string& event_name,
                               const EventCallback& callback) {
    std::unique_lock lock(queue_mutex_);
    event_subscribers_[event_name].push_back(callback);
}

void EventLoop::emitEvent(const std::string& event_name) {
    std::unique_lock lock(queue_mutex_);
    if (event_subscribers_.count(event_name)) {
        for (const auto& callback : event_subscribers_[event_name]) {
            post(callback);
        }
    }
}

#ifdef __linux__
void EventLoop::addSignalHandler(int signal, std::function<void()> handler) {
    std::unique_lock lock(queue_mutex_);
    signal_handlers_[signal] = std::move(handler);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, signal);
    signal_fd_ = signalfd(-1, &mask, SFD_NONBLOCK);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = signal_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, signal_fd_, &ev);
}
#endif

void EventLoop::wakeup() {
#ifdef __linux__
    // Linux: 使用 eventfd 唤醒 epoll
    int eventFd = eventfd(0, EFD_NONBLOCK);
    epoll_event ev = {0};
    ev.events = EPOLLIN;
    ev.data.fd = eventFd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, eventFd, &ev);
    uint64_t one = 1;
    write(eventFd, &one, sizeof(uint64_t));  // 触发事件
    close(eventFd);
#elif _WIN32
    // Windows: 利用 select 中的 socket 模拟唤醒机制
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    FD_SET(sock, &read_fds);
#endif
}

#ifdef __linux__
void EventLoop::addEpollFd(int fd) const {
    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        ABORT_F("Failed to add fd to epoll");
    }
}
#elif _WIN32
void EventLoop::add_socket_fd(SOCKET fd) { FD_SET(fd, &read_fds); }
#endif

}  // namespace lithium::app
