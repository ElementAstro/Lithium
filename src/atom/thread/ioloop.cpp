#include <iostream>
#include <functional>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/epoll.h>
#include <unistd.h>
#endif

class IOLoop
{
public:
    IOLoop() : running(false)
    {
#ifdef _WIN32
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            throw std::runtime_error("Failed to initialize Winsock");
        }
#endif
    }

    ~IOLoop()
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void addHandler(int fd, std::function<void()> callback, bool writeEvent = false)
    {
#ifdef _WIN32
        handlers.push_back({fd, callback, writeEvent});
#else
        epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN;

        if (writeEvent)
        {
            event.events |= EPOLLOUT;
        }

        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
        {
            throw std::runtime_error("Failed to add handler to epoll");
        }

        handlers[fd] = {callback, writeEvent};
#endif
    }

    void removeHandler(int fd)
    {
#ifdef _WIN32
        auto it = std::find_if(handlers.begin(), handlers.end(), [fd](const auto &handler)
                               { return handler.fd == fd; });
        if (it != handlers.end())
        {
            handlers.erase(it);
        }
#else
        if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1)
        {
            throw std::runtime_error("Failed to remove handler from epoll");
        }

        handlers.erase(fd);
#endif
    }

    void modifyHandler(int fd, std::function<void()> callback, bool writeEvent = false)
    {
#ifdef _WIN32
        auto it = std::find_if(handlers.begin(), handlers.end(), [fd](const auto &handler)
                               { return handler.fd == fd; });
        if (it != handlers.end())
        {
            it->callback = callback;
            it->writeEvent = writeEvent;
        }
#else
        epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN;

        if (writeEvent)
        {
            event.events |= EPOLLOUT;
        }

        if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event) == -1)
        {
            throw std::runtime_error("Failed to modify handler in epoll");
        }

        handlers[fd] = {callback, writeEvent};
#endif
    }

    void start()
    {
        running = true;

#ifdef _WIN32
        while (running)
        {
            fd_set readfds, writefds;
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            int maxFd = 0;

            for (const auto &handler : handlers)
            {
                if (handler.writeEvent)
                {
                    FD_SET(handler.fd, &writefds);
                }
                else
                {
                    FD_SET(handler.fd, &readfds);
                }
                maxFd = std::max(maxFd, handler.fd);
            }

            timeval timeout = {0, 0};
            int numReady = select(maxFd + 1, &readfds, &writefds, nullptr, &timeout);
            if (numReady < 0)
            {
                std::cerr << "Error in select" << std::endl;
                break;
            }

            for (const auto &handler : handlers)
            {
                if ((handler.writeEvent && FD_ISSET(handler.fd, &writefds)) ||
                    (!handler.writeEvent && FD_ISSET(handler.fd, &readfds)))
                {
                    handler.callback();
                }
            }
        }
#else
        epollFd = epoll_create1(0);
        if (epollFd == -1)
        {
            throw std::runtime_error("Failed to create epoll");
        }

        epoll_event events[MAX_EVENTS];

        while (running)
        {
            int numReady = epoll_wait(epollFd, events, MAX_EVENTS, -1);
            if (numReady == -1)
            {
                std::cerr << "Error in epoll_wait" << std::endl;
                break;
            }

            for (int i = 0; i < numReady; ++i)
            {
                int fd = events[i].data.fd;

                if (handlers[fd].writeEvent && (events[i].events & EPOLLOUT))
                {
                    handlers[fd].callback();
                }
                else if (!handlers[fd].writeEvent && (events[i].events & EPOLLIN))
                {
                    handlers[fd].callback();
                }
            }
        }

        close(epollFd);
#endif
    }

    void stop()
    {
        running = false;
    }

private:
    struct EventHandler
    {
#ifdef _WIN32
        int fd;
#endif
        std::function<void()> callback;
        bool writeEvent;
    };

    std::vector<EventHandler> handlers;

#ifndef _WIN32
    static const int MAX_EVENTS = 10;
    int epollFd;
    std::unordered_map<int, EventHandler> handlers;
#endif

    bool running;
};

void startWorkerThreads(int numThreads, std::function<void()> workerFunc)
{
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(workerFunc);
    }

    for (auto &thread : threads)
    {
        thread.join();
    }
}

class ThreadPool
{
public:
    ThreadPool(int numThreads) : stopFlag(false)
    {
        for (int i = 0; i < numThreads; ++i)
        {
            workers.emplace_back([this]()
                                 { this->workerThread(); });
        }
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stopFlag = true;
        }
        condition.notify_all();

        for (auto &worker : workers)
        {
            worker.join();
        }
    }

    void addTask(std::function<void()> task)
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        taskQueue.push(task);
        condition.notify_one();
    }

private:
    void workerThread()
    {
        while (true)
        {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(queueMutex);

                condition.wait(lock, [this]()
                               { return !taskQueue.empty() || stopFlag; });

                if (stopFlag && taskQueue.empty())
                {
                    break;
                }

                task = taskQueue.front();
                taskQueue.pop();
            }

            task();
        }
    }

    std::vector<std::thread> workers;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stopFlag;
};

int main()
{
    IOLoop ioLoop;

    // 添加文件描述符和对应的处理函数
#ifdef _WIN32
    HANDLE pipeRead, pipeWrite;
    if (!CreatePipe(&pipeRead, &pipeWrite, NULL, 0))
    {
        std::cerr << "Failed to create pipe" << std::endl;
        return 1;
    }

    ioLoop.addHandler(reinterpret_cast<intptr_t>(pipeRead), []()
                      { std::cout << "Received data from pipe" << std::endl; });
    ioLoop.addHandler(
        reinterpret_cast<intptr_t>(pipeWrite), []()
        { std::cout << "Received output event" << std::endl; },
        true);

        ioLoop.start();

    // 模拟发送数据到管道
    std::string message = "Hello, world!";
    DWORD bytesWritten;
    if (!WriteFile(pipeWrite, message.c_str(), static_cast<DWORD>(message.length()), &bytesWritten, NULL))
    {
        std::cerr << "Failed to write to pipe" << std::endl;
        return 1;
    }
#else
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        std::cerr << "Failed to create pipe" << std::endl;
        return 1;
    }

    ioLoop.addHandler(pipefd[0], []()
                      { std::cout << "Received data from pipe" << std::endl; });
    ioLoop.addHandler(
        1, []()
        { std::cout << "Received output event" << std::endl; },
        true);

    // 模拟发送数据到管道
    std::string message = "Hello, world!";
    write(pipefd[1], message.c_str(), message.length());
#endif

    // 启动IOLoop
    

    // 创建线程池，用于处理任务
    ThreadPool threadPool(4);

    // 添加任务到线程池
    for (int i = 0; i < 10; ++i)
    {
        threadPool.addTask([i]()
                           { std::cout << "Task " << i << " executed" << std::endl; });
    }

    // 等待一段时间
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 停止IOLoop和线程池
    ioLoop.stop();

    return 0;
}
