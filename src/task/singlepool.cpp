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

#include <condition_variable>
#include <memory>

namespace Lithium {

class SingleThreadPoolPrivate {
public:
    SingleThreadPoolPrivate();
    virtual ~SingleThreadPoolPrivate();

    std::function<void(const std::atomic_bool &isAboutToClose)> pendingFunction;
    std::function<void(const std::atomic_bool &isAboutToClose)> runningFunction;
    std::atomic_bool isThreadAboutToQuit{false};
    std::atomic_bool isFunctionAboutToQuit{true};

    std::condition_variable_any acquire;
    std::condition_variable_any released;
    std::mutex runLock;
    std::vector<std::thread> threads;
};

SingleThreadPoolPrivate::SingleThreadPoolPrivate() {
    threads.emplace_back([this] {
        std::unique_lock<std::mutex> lock(runLock);
        for (;;) {
            acquire.wait(lock, [this] {
                return pendingFunction != nullptr || isThreadAboutToQuit;
            });

            if (isThreadAboutToQuit)
                break;

            isFunctionAboutToQuit = false;
            std::swap(runningFunction, pendingFunction);
            released.notify_all();

            lock.unlock();
            runningFunction(isFunctionAboutToQuit);
            lock.lock();

            runningFunction = nullptr;
        }
    });
}

SingleThreadPoolPrivate::~SingleThreadPoolPrivate() {
    {
        isFunctionAboutToQuit = true;
        isThreadAboutToQuit = true;
        std::unique_lock<std::mutex> lock(runLock);
        acquire.notify_all();
    }

    for (auto &thread : threads) {
        if (thread.joinable())
            thread.join();
    }
}

SingleThreadPool::SingleThreadPool()
    : d_ptr(std::make_shared<SingleThreadPoolPrivate>()) {}

SingleThreadPool::~SingleThreadPool() {}

bool SingleThreadPool::start(
    const std::function<void(const std::atomic_bool &isAboutToClose)>
        &functionToRun) {
    if (!functionToRun)
        return false;

    auto d = d_ptr;
    std::unique_lock<std::mutex> lock(d->runLock);
    if (d->runningFunction != nullptr)
        return false;

    d->pendingFunction = functionToRun;
    d->isFunctionAboutToQuit = true;
    d->acquire.notify_one();

    if (std::this_thread::get_id() != d->threads[0].get_id())
        d->released.wait(lock, [d] { return d->pendingFunction == nullptr; });

    return true;
}

void SingleThreadPool::startDetach(
    const std::function<void(const std::atomic_bool &isAboutToClose)>
        &functionToRun) {
    if (!functionToRun)
        return;

    auto d = d_ptr;
    std::unique_lock<std::mutex> lock(d->runLock);
    if (d->runningFunction != nullptr)
        return;

    d->pendingFunction = functionToRun;
    d->isFunctionAboutToQuit = true;
    d->acquire.notify_one();
}

bool SingleThreadPool::tryStart(
    const std::function<void(const std::atomic_bool &isAboutToClose)>
        &functionToRun) {
    if (!functionToRun)
        return false;

    auto d = d_ptr;
    std::unique_lock<std::mutex> lock(d->runLock);
    if (d->runningFunction != nullptr)
        return false;

    d->isFunctionAboutToQuit = true;
    d->pendingFunction = functionToRun;
    d->acquire.notify_one();

    if (std::this_thread::get_id() != d->threads[0].get_id())
        d->released.wait(lock, [d] { return d->pendingFunction == nullptr; });

    return true;
}

void SingleThreadPool::tryStartDetach(
    const std::function<void(const std::atomic_bool &isAboutToClose)>
        &functionToRun) {
    if (!functionToRun)
        return;

    auto d = d_ptr;
    std::unique_lock<std::mutex> lock(d->runLock);
    if (d->runningFunction != nullptr)
        return;

    d->isFunctionAboutToQuit = true;
    d->pendingFunction = functionToRun;
    d->acquire.notify_one();
}

void SingleThreadPool::quit() { start(nullptr); }
}  // namespace Lithium
