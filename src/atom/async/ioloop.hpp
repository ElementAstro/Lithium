/*
 * ioloop.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Just a try and learn something, not for production use.(Windows not supported)

**************************************************/

#pragma once

#include <functional>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <queue>

namespace Atom::Async
{

    /**
     * @brief The IOLoop class provides an event-driven I/O loop for handling I/O events on file descriptors.
     */
    class IOLoop
    {
    public:
        /**
         * @brief Constructs an IOLoop object.
         */
        IOLoop();

        /**
         * @brief Destroys the IOLoop object.
         */
        ~IOLoop();

        /**
         * @brief Adds a handler for the specified file descriptor.
         *
         * @param fd The file descriptor.
         * @param callback The callback function to be called when an event occurs on the file descriptor.
         * @param writeEvent Set to true if the handler should handle write events (default: false).
         */
        void addHandler(int fd, std::function<void()> callback, bool writeEvent = false);

        /**
         * @brief Removes the handler for the specified file descriptor.
         *
         * @param fd The file descriptor.
         */
        void removeHandler(int fd);

        /**
         * @brief Modifies the handler for the specified file descriptor.
         *
         * @param fd The file descriptor.
         * @param callback The new callback function to be called when an event occurs on the file descriptor.
         * @param writeEvent Set to true if the handler should handle write events (default: false).
         */
        void modifyHandler(int fd, std::function<void()> callback, bool writeEvent = false);

        /**
         * @brief Starts the IOLoop, entering the event loop.
         */
        void start();

        /**
         * @brief Stops the IOLoop, exiting the event loop.
         */
        void stop();

    private:
        struct EventHandler
        {
#ifdef _WIN32
            int fd;
#endif
            std::function<void()> callback;
            bool writeEvent;
        };

        std::vector<EventHandler> handlers; /**< Vector to store the registered event handlers. */

#ifndef _WIN32
        static const int MAX_EVENTS = 10;
        int epollFd;
        std::unordered_map<int, EventHandler> handlers; /**< Hash map to store the registered event handlers. */
#endif

        bool running; /**< Flag indicating whether the IOLoop is running. */
    };

    /**
     * @brief The ThreadPool class provides a pool of worker threads for executing tasks asynchronously.
     */
    class ThreadPool
    {
    public:
        /**
         * @brief Constructs a ThreadPool object with the specified number of worker threads.
         *
         * @param numThreads The number of worker threads to create.
         */
        ThreadPool(int numThreads);

        /**
         * @brief Destroys the ThreadPool object.
         */
        ~ThreadPool();

        /**
         * @brief Adds a task to the thread pool.
         *
         * @param task The task to be executed by a worker thread.
         */
        void addTask(std::function<void()> task);

    private:
        /**
         * @brief Worker thread function that continuously waits for and executes tasks.
         */
        void workerThread();

        std::vector<std::thread> workers;            /**< Vector to store the worker threads. */
        std::queue<std::function<void()>> taskQueue; /**< Queue to store the tasks. */
        std::mutex queueMutex;                       /**< Mutex used to synchronize access to the task queue. */
        std::condition_variable condition;           /**< Condition variable used to notify worker threads of new tasks. */
        bool stopFlag;                               /**< Flag indicating whether the thread pool should stop processing tasks. */
    };

} // namespace Atom::Async
