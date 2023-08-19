/*
 * eventloop.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-29

Description: EventLoop

**************************************************/

#pragma once

#include <iostream>
#include <functional>
#include <unordered_map>
#include <chrono>
#include <queue>
#include <mutex>
#include <thread>
#include <future>

/**
 * @brief EventPriority enum
 *
 * This enum defines the priority levels for events.
 * The priority levels are used to determine the order in which events are processed by the EventLoop.
 *
 * @details
 * The priority levels are defined as follows:
 * - EventPriority::Low: Low priority events are processed first, and are suitable for tasks that do not require immediate attention.
 * - EventPriority::Medium: Medium priority events are processed next, and are suitable for tasks that require some attention, but can be delayed if necessary.
 * - EventPriority::High: High priority events are processed last, and are suitable for tasks that require immediate attention.
 *
 * @note
 * The priority levels are designed to be flexible, and can be customized as needed.
 */
enum class EventPriority
{
    Low,
    Medium,
    High
};

/**
 * @brief EventHandler typedef
 *
 * This typedef defines a function pointer to a void function that can be used as an event handler.
 * The event handler is called when an event is triggered.
 */
using EventHandler = std::function<void()>;

/**
 * @brief Event struct
 *
 * This struct defines a single event that can be triggered by the EventTrigger class.
 * It contains a reference to the event handler, and the priority of the event.
 * The priority is used to determine the order in which events are processed by the EventLoop.
 */
struct Event
{
    EventHandler handler;
    EventPriority priority;

    bool operator<(const Event &other) const
    {
        return priority > other.priority;
    }
};

/**
 * @brief EventTrigger class
 *
 * This class provides a way to trigger events in an EventLoop.
 * It uses a priority queue to manage the events, with low priority events being processed first.
 * Events can be added using the addEvent() method, and then triggered using the triggerEvents() method.
 *
 * @details
 * The EventTrigger class is designed to be easy to use and flexible.
 * It allows for registering events with different priorities, making it possible to handle events in the order they were registered.
 * The events are processed by the EventLoop in the order they were added, making it possible to handle events based on their priority.
 *
 * @note
 * This class is designed to be thread-safe.
 */
class EventTrigger
{
public:
    /**
     * @brief Add an event to the event trigger
     *
     * Add an event to the event trigger to be triggered.
     * The event will be triggered when the triggerEvents() method is called.
     *
     * @tparam F The type of the event handler
     * @param priority The priority of the event
     * @param f The event handler
     * @param args The arguments to pass to the event handler
     */
    template <typename F, typename... Args>
    void addEvent(EventPriority priority, F &&f, Args &&...args)
    {
        auto handler = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        Event event{handler, priority};
        std::lock_guard<std::mutex> lock(mutex);
        eventQueue.push(event);
    }

    /**
     * @brief Trigger all events
     *
     * Trigger all events in the event trigger.
     * This method should be called from the event loop thread.
     */
    void triggerEvents();

private:
    std::priority_queue<Event> eventQueue; ///< The queue of events
    std::mutex mutex;                      ///< The mutex for thread synchronization
};

/**
 * @brief EventLoop class
 *
 * This class provides an event loop for asynchronous tasks.
 * It allows for registering events and tasks, and then running the event loop to process them.
 *
 * @details
 * The EventLoop class is designed to be easy to use and flexible.
 * It uses a priority queue to manage events, with low priority events being processed first.
 * Tasks can be added to the event loop as either synchronous or asynchronous.
 * Synchronous tasks are executed immediately by the event loop, while asynchronous tasks are executed in a separate thread.
 *
 * @note
 * This class is designed to be thread-safe.
 */
class EventLoop
{
public:
    /**
     * @brief Start the event loop
     *
     * Start the event loop running in a separate thread.
     * This method should be called after all events and tasks have been registered.
     */
    void start();

    /**
     * @brief Stop the event loop
     *
     * Stop the event loop from running.
     * This method should be called when the event loop is no longer needed.
     */
    void stop();

    /**
     * @brief Add a task to the event loop
     *
     * Add a task to the event loop to be executed.
     * The task can be a synchronous function or a coroutine.
     *
     * @tparam F The type of the task to add
     * @param task The task to add
     */
    template <typename F>
    void addTask(F &&task)
    {
        std::lock_guard<std::mutex> lock(mutex);
        asyncTasks.push_back(std::async(std::launch::async, std::move(task)));
    }

    /**
     * @brief Add an event to the event loop
     *
     * Add an event to the event loop to be processed.
     * The event will be executed when the event loop is running.
     *
     * @tparam F The type of the event handler
     * @param priority The priority of the event
     * @param f The event handler
     * @param args The arguments to pass to the event handler
     */
    template <typename F, typename... Args>
    void runAfter(int milliseconds, F &&f, Args &&...args)
    {
        auto task = [this, milliseconds, f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            std::apply(f, args);
        };

        addTask(std::move(task));
    }

    /**
     * @brief Register an event trigger
     *
     * Register an event trigger with the event loop.
     * The event trigger will be called when an event is triggered.
     *
     * @tparam F The type of the event trigger
     * @param f The event trigger
     * @param args The arguments to pass to the event trigger
     */
    template <typename F, typename... Args>
    void registerEventTrigger(F &&f, Args &&...args)
    {
        eventTrigger.addEvent(EventPriority::Medium, std::forward<F>(f), std::forward<Args>(args)...);
    }

private:
    /**
     * @brief Process events in the event queue
     *
     * Process all events in the event queue, sorted by priority.
     * Low priority events are processed first.
     */
    void processEvents();

    /**
     * @brief Process asynchronous tasks
     *
     * Process all asynchronous tasks, executing them one at a time.
     * This method is called from the event loop thread.
     */
    void processAsyncTasks();

    /**
     * @brief Thread function for the event loop
     *
     * This method is called from the event loop thread.
     * It runs the event loop, processing events and tasks as needed.
     */
    void eventLoopThreadFunc();

    bool running;                              ///< Flag indicating if the event loop is running
    std::mutex mutex;                          ///< Mutex for thread synchronization
    std::vector<std::future<void>> asyncTasks; ///< Queue of asynchronous tasks
    EventTrigger eventTrigger;                 ///< Event trigger for registered events
    std::jthread eventLoopThread;              ///< Thread for the event loop
};

// 定义协程类
/**
 * @brief Coroutine class
 *
 * This class provides a way to write asynchronous code using coroutines.
 * A coroutine is a lightweight thread that can be suspended and resumed at will.
 * It allows for writing asynchronous code that looks like synchronous code, making it easier to read and debug.
 *
 * @details
 * The Coroutine class is designed to be easy to use and flexible.
 * It allows for defining a coroutine as a regular function, and then starting it using the start() method.
 * The coroutine can be stopped using the stop() method, and it will automatically be stopped when the function returns.
 *
 * @note
 * This class is designed to be thread-safe.
 */
class Coroutine
{
public:
    /**
     * @brief Construct a new Coroutine object
     *
     * @param loop The event loop to use for scheduling the coroutine
     */
    Coroutine(EventLoop &loop) : eventLoop(loop), task(nullptr), stopFlag(false) {}

    /**
     * @brief Destroy the Coroutine object
     */
    ~Coroutine()
    {
        stop();
    }

    /**
     * @brief Start the coroutine
     *
     * Start the coroutine running.
     * This method should be called after the coroutine has been defined.
     *
     * @tparam F The type of the coroutine function
     * @param f The coroutine function
     */
    template <typename F>
    void start(F &&f)
    {
        task = std::make_shared<std::function<void()>>(std::forward<F>(f));
        run();
    }

    /**
     * @brief Stop the coroutine
     *
     * Stop the coroutine from running.
     * This method should be called when the coroutine is no longer needed.
     */
    void stop()
    {
        stopFlag = true;
    }

private:
    /**
     * @brief Run the coroutine
     *
     * This method is called from the event loop thread.
     * It runs the coroutine until it is stopped.
     */
    void run()
    {
        eventLoop.addTask([this]()
                          {
            if (task && !stopFlag) {
                (*task)();
                run();
            } });
    }

    EventLoop &eventLoop;                        ///< The event loop used for scheduling the coroutine
    std::shared_ptr<std::function<void()>> task; ///< The coroutine function
    bool stopFlag;                               ///< Flag indicating if the coroutine should be stopped
};