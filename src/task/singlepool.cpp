/*
 * singlepool.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Single thread pool for executing temporary tasks asynchronously.

**************************************************/

#include "singlepool.hpp"

#include <cassert>
#include <condition_variable>
#include <deque>
#include <future>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>

namespace lithium {
class SingleThreadPoolPrivate {
public:
    SingleThreadPoolPrivate();
    ~SingleThreadPoolPrivate();

    std::atomic<bool> isThreadAboutToQuit{false};
    std::atomic<bool> isFunctionAboutToQuit{true};
    std::function<void(const std::atomic_bool&)> pendingFunction;
    std::function<void(const std::atomic_bool&)> runningFunction;
    std::mutex runLock;
    std::condition_variable acquireCondition;
    std::condition_variable releasedCondition;
    std::thread workerThread;

    void workerFunction();
};

SingleThreadPoolPrivate::SingleThreadPoolPrivate() {
    // Launch a worker thread to manage tasks
    workerThread = std::thread(&SingleThreadPoolPrivate::workerFunction, this);
}

SingleThreadPoolPrivate::~SingleThreadPoolPrivate() {
    isThreadAboutToQuit = true;
    acquireCondition.notify_one();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void SingleThreadPoolPrivate::workerFunction() {
    std::unique_lock lock(runLock);
    while (!isThreadAboutToQuit) {
        acquireCondition.wait(lock, [this] {
            return pendingFunction != nullptr || isThreadAboutToQuit;
        });

        if (isThreadAboutToQuit)
            break;

        isFunctionAboutToQuit = false;
        runningFunction = std::move(pendingFunction);
        releasedCondition.notify_all();

        lock.unlock();
        if (runningFunction) {
            runningFunction(isFunctionAboutToQuit);
        }
        lock.lock();

        runningFunction = nullptr;
    }
}

SingleThreadPool::SingleThreadPool()
    : d_ptr(std::make_shared<SingleThreadPoolPrivate>()) {}

SingleThreadPool::~SingleThreadPool() = default;

bool SingleThreadPool::start(
    const std::function<void(const std::atomic_bool&)>& functionToRun) {
    if (!functionToRun)
        return false;

    auto d = d_ptr;
    std::unique_lock lock(d->runLock);
    if (d->runningFunction != nullptr)
        return false;

    d->pendingFunction = functionToRun;
    d->isFunctionAboutToQuit = true;
    d->acquireCondition.notify_one();

    // Wait until the function starts running (when not on the worker thread)
    if (std::this_thread::get_id() != d->workerThread.get_id()) {
        d->releasedCondition.wait(
            lock, [d] { return d->pendingFunction == nullptr; });
    }

    return true;
}

void SingleThreadPool::startDetach(
    const std::function<void(const std::atomic_bool&)>& functionToRun) {
    if (!functionToRun)
        return;

    auto d = d_ptr;
    std::unique_lock lock(d->runLock);
    if (d->runningFunction != nullptr)
        return;

    d->pendingFunction = functionToRun;
    d->isFunctionAboutToQuit = true;
    d->acquireCondition.notify_one();
}

bool SingleThreadPool::tryStart(
    const std::function<void(const std::atomic_bool&)>& functionToRun) {
    if (!functionToRun)
        return false;

    auto d = d_ptr;
    std::unique_lock lock(d->runLock, std::try_to_lock);
    if (!lock.owns_lock() || d->runningFunction != nullptr)
        return false;

    d->pendingFunction = functionToRun;
    d->isFunctionAboutToQuit = true;
    d->acquireCondition.notify_one();

    // Wait until the function starts running (when not on the worker thread)
    if (std::this_thread::get_id() != d->workerThread.get_id()) {
        d->releasedCondition.wait(
            lock, [d] { return d->pendingFunction == nullptr; });
    }

    return true;
}

void SingleThreadPool::tryStartDetach(
    const std::function<void(const std::atomic_bool&)>& functionToRun) {
    if (!functionToRun)
        return;

    auto d = d_ptr;
    std::unique_lock lock(d->runLock, std::try_to_lock);
    if (!lock.owns_lock() || d->runningFunction != nullptr)
        return;

    d->pendingFunction = functionToRun;
    d->isFunctionAboutToQuit = true;
    d->acquireCondition.notify_one();
}

void SingleThreadPool::quit() { start(nullptr); }
}  // namespace lithium
