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
    /**
     * @brief Construct a new Thread Pool object
     *
     * @param n_threads The number of threads in the pool
     */
    explicit ThreadPool(std::size_t n_threads)
        : stop_(false), active_count_(0) {
        startThreads(n_threads);
    }

    /**
     * @brief Destroy the Thread Pool object
     */
    ~ThreadPool() { stopPool(); }

    /**
     * @brief Enqueue a task to the pool
     *
     * @tparam F The type of the function
     * @tparam Args The type of the arguments
     * @param f The function
     * @param args The arguments
     * @return std::future<std::invoke_result_t<F, Args...>> The future of the
     * task
     */
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

    /**
     * @brief Wait for all tasks to finish
     */
    void wait() {
        std::unique_lock lock(queue_mutex_);
        condition_.wait(
            lock, [this] { return tasks_.empty() && active_count_ == 0; });
    }

    /**
     * @brief Get the number of threads in the pool
     *
     * @return std::size_t The number of threads in the pool
     */
    auto size() const -> std::size_t { return threads_.size(); }

    /**
     * @brief Get the number of tasks in the pool
     *
     * @return std::size_t The number of tasks in the pool
     */
    auto taskCount() const -> std::size_t {
        std::scoped_lock lock(queue_mutex_);
        return tasks_.size();
    }

    /**
     * @brief Resize the number of threads in the pool
     *
     * @param n_threads The number of threads in the pool
     */
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

    /**
     * @brief Start the threads in the pool
     *
     * @param n_threads The number of threads in the pool
     */
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

                    task();
                    --active_count_;
                    condition_.notify_one();
                }
            });
        }
    }

    /**
     * @brief Stop the pool
     */
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
