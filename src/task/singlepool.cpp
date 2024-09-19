/**
 * @file singlepool.cpp
 * @brief Single-threaded pool for executing temporary tasks asynchronously.
 *
 * This file defines a single-threaded pool designed to manage and execute
 * temporary tasks asynchronously. The pool allows tasks to be enqueued and
 * processed by a single worker thread, facilitating asynchronous execution
 * without the overhead of managing multiple threads.
 *
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#include "singlepool.hpp"

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace lithium {
class SingleThreadPoolPrivate {
public:
    SingleThreadPoolPrivate();
    ~SingleThreadPoolPrivate();

    // Delete copy constructor and copy assignment operator
    SingleThreadPoolPrivate(const SingleThreadPoolPrivate&) = delete;
    SingleThreadPoolPrivate& operator=(const SingleThreadPoolPrivate&) = delete;

    // Delete move constructor and move assignment operator
    SingleThreadPoolPrivate(SingleThreadPoolPrivate&&) = delete;
    SingleThreadPoolPrivate& operator=(SingleThreadPoolPrivate&&) = delete;

private:
    std::atomic<bool> isThreadAboutToQuit_{false};
    std::atomic<bool> isFunctionAboutToQuit_{true};
    std::function<void(const std::atomic_bool&)> pendingFunction_;
    std::function<void(const std::atomic_bool&)> runningFunction_;
    std::mutex runLock_;
    std::condition_variable acquireCondition_;
    std::condition_variable releasedCondition_;
    std::thread workerThread_;

    void workerFunction();

    friend class SingleThreadPool;
};

SingleThreadPoolPrivate::SingleThreadPoolPrivate() {
    // Launch a worker thread to manage tasks
    workerThread_ = std::thread(&SingleThreadPoolPrivate::workerFunction, this);
}

SingleThreadPoolPrivate::~SingleThreadPoolPrivate() {
    isThreadAboutToQuit_ = true;
    acquireCondition_.notify_one();
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void SingleThreadPoolPrivate::workerFunction() {
    std::unique_lock lock(runLock_);
    while (!isThreadAboutToQuit_) {
        acquireCondition_.wait(lock, [this] {
            return pendingFunction_ != nullptr || isThreadAboutToQuit_;
        });

        if (isThreadAboutToQuit_) {
            break;
        }

        isFunctionAboutToQuit_ = false;
        runningFunction_ = std::move(pendingFunction_);
        releasedCondition_.notify_all();

        lock.unlock();
        if (runningFunction_) {
            runningFunction_(isFunctionAboutToQuit_);
        }
        lock.lock();

        runningFunction_ = nullptr;
    }
}

SingleThreadPool::SingleThreadPool()
    : d_ptr_(std::make_shared<SingleThreadPoolPrivate>()) {}

SingleThreadPool::~SingleThreadPool() = default;

auto SingleThreadPool::start(
    const std::function<void(const std::atomic_bool&)>& functionToRun) -> bool {
    if (!functionToRun) {
        return false;
    }

    auto dPtr = d_ptr_;
    std::unique_lock lock(dPtr->runLock_);
    if (dPtr->runningFunction_ != nullptr) {
        return false;
    }

    dPtr->pendingFunction_ = functionToRun;
    dPtr->isFunctionAboutToQuit_ = true;
    dPtr->acquireCondition_.notify_one();

    // Wait until the function starts running (when not on the worker thread)
    if (std::this_thread::get_id() != dPtr->workerThread_.get_id()) {
        dPtr->releasedCondition_.wait(
            lock, [dPtr] { return dPtr->pendingFunction_ == nullptr; });
    }

    return true;
}

void SingleThreadPool::startDetach(
    const std::function<void(const std::atomic_bool&)>& functionToRun) {
    if (!functionToRun) {
        return;
    }

    auto dPtr = d_ptr_;
    std::unique_lock lock(dPtr->runLock_);
    if (dPtr->runningFunction_ != nullptr) {
        return;
    }

    dPtr->pendingFunction_ = functionToRun;
    dPtr->isFunctionAboutToQuit_ = true;
    dPtr->acquireCondition_.notify_one();
}

auto SingleThreadPool::tryStart(
    const std::function<void(const std::atomic_bool&)>& functionToRun) -> bool {
    if (!functionToRun) {
        return false;
    }

    auto dPtr = d_ptr_;
    std::unique_lock lock(dPtr->runLock_, std::try_to_lock);
    if (!lock.owns_lock() || dPtr->runningFunction_ != nullptr) {
        return false;
    }

    dPtr->pendingFunction_ = functionToRun;
    dPtr->isFunctionAboutToQuit_ = true;
    dPtr->acquireCondition_.notify_one();

    // Wait until the function starts running (when not on the worker thread)
    if (std::this_thread::get_id() != dPtr->workerThread_.get_id()) {
        dPtr->releasedCondition_.wait(
            lock, [dPtr] { return dPtr->pendingFunction_ == nullptr; });
    }

    return true;
}

void SingleThreadPool::tryStartDetach(
    const std::function<void(const std::atomic_bool&)>& functionToRun) {
    if (!functionToRun) {
        return;
    }

    auto dPtr = d_ptr_;
    std::unique_lock lock(dPtr->runLock_, std::try_to_lock);
    if (!lock.owns_lock() || dPtr->runningFunction_ != nullptr) {
        return;
    }

    dPtr->pendingFunction_ = functionToRun;
    dPtr->isFunctionAboutToQuit_ = true;
    dPtr->acquireCondition_.notify_one();
}

void SingleThreadPool::quit() { start(nullptr); }
}  // namespace lithium