/**
 * @file singlepool.hpp
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

#ifndef LITHIUM_TASK_SINGLEPOOL_HPP
#define LITHIUM_TASK_SINGLEPOOL_HPP

#include <atomic>
#include <functional>
#include <memory>

namespace lithium {
class SingleThreadPoolPrivate;

class SingleThreadPool {
public:
    SingleThreadPool();
    ~SingleThreadPool();

    auto start(const std::function<void(const std::atomic_bool&)>&
                   functionToRun) -> bool;
    void startDetach(
        const std::function<void(const std::atomic_bool&)>& functionToRun);
    auto tryStart(const std::function<void(const std::atomic_bool&)>&
                      functionToRun) -> bool;
    void tryStartDetach(
        const std::function<void(const std::atomic_bool&)>& functionToRun);
    void quit();

private:
    std::shared_ptr<SingleThreadPoolPrivate> d_ptr_;
};
}  // namespace lithium

#endif
