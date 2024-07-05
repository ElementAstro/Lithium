/*
 * pool.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-13

Description: A very simple thread pool for preload

**************************************************/

#ifndef ATOM_ASYNC_POOL_HPP
#define ATOM_ASYNC_POOL_HPP

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/type/noncopyable.hpp"

namespace atom::async {
class ThreadPool : public NonCopyable {
public:
    explicit ThreadPool(std::size_t n_threads)
        : stop_(false), active_count_(0) {
        startThreads(n_threads);
    }

    ~ThreadPool() { stopPool(); }

    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        auto res = task->get_future();
        {
            std::scoped_lock lock(queue_mutex_);
            if (stop_) {
                THROW_UNLAWFUL_OPERATION("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task = std::move(task)]() { (*task)(); });
        }
        condition_.notify_one();
        return res;
    }

    void wait() {
        std::unique_lock lock(queue_mutex_);
        condition_.wait(
            lock, [this] { return tasks_.empty() && active_count_ == 0; });
    }

    auto size() const -> std::size_t { return threads_.size(); }

    auto taskCount() const -> std::size_t {
        std::scoped_lock lock(queue_mutex_);
        return tasks_.size();
    }

    void resize(std::size_t n_threads) {
        {
            std::scoped_lock lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto& thread : threads_) {
            thread.join();
        }
        threads_.clear();
        {
            std::scoped_lock lock(queue_mutex_);
            stop_ = false;
        }
        startThreads(n_threads);
    }

private:
    std::vector<std::jthread> threads_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    std::atomic<int> active_count_;

    void startThreads(std::size_t n_threads) {
        threads_.reserve(n_threads);
        for (std::size_t i = 0; i < n_threads; ++i) {
            threads_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(queue_mutex_);
                        condition_.wait(
                            lock, [this] { return stop_ || !tasks_.empty(); });

                        if (stop_ && tasks_.empty()) {
                            return;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                        ++active_count_;
                    }

                    try {
                        task();
                    } catch (...) {
                        // Handle exceptions from tasks
                    }
                    --active_count_;
                    condition_.notify_one();
                }
            });
        }
    }

    void stopPool() {
        {
            std::scoped_lock lock(queue_mutex_);
            stop_ = true;
        }
        condition_.notify_all();
        for (auto& thread : threads_) {
            thread.join();
        }
    }
};

}  // namespace atom::async

#endif  // ATOM_ASYNC_POOL_HPP
