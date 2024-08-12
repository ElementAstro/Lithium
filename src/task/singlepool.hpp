/*
 * singlepool.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Single thread pool for executing temporary tasks asynchronously.

**************************************************/

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

    auto start(
        const std::function<void(const std::atomic_bool&)>& functionToRun) -> bool;
    void startDetach(
        const std::function<void(const std::atomic_bool&)>& functionToRun);
    auto tryStart(
        const std::function<void(const std::atomic_bool&)>& functionToRun) -> bool;
    void tryStartDetach(
        const std::function<void(const std::atomic_bool&)>& functionToRun);
    void quit();

private:
    std::shared_ptr<SingleThreadPoolPrivate> d_ptr_;
};
}  // namespace lithium

#endif
