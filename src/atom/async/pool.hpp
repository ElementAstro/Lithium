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
#include <cstdint>
#include <functional>
#include <future>
#include <latch>
#include <mutex>
#include <queue>
#include <semaphore>
#include <stdexcept>
#include <thread>
#include <vector>

#include "atom/error/exception.hpp"

namespace atom::async {
class ThreadPool {
public:
    /**
     * @brief Construct a new Thread Pool object
     *
     * @param n_threads The number of threads in the pool
     */
    explicit ThreadPool(std::size_t n_threads)
        : threads(n_threads), stop(false), active_count(0) {
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
            std::unique_lock lock(queue_mutex);
            if (stop) {
                THROW_UNLAWFUL_OPERATION("enqueue on stopped ThreadPool");
            }
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    /**
     * @brief Wait for all tasks to finish
     */
    void wait() {
        std::unique_lock lock(queue_mutex);
        condition.wait(lock,
                       [this] { return tasks.empty() && active_count == 0; });
    }

    /**
     * @brief Get the number of threads in the pool
     *
     * @return std::size_t The number of threads in the pool
     */
    std::size_t size() const { return threads.size(); }

    /**
     * @brief Get the number of tasks in the pool
     *
     * @return std::size_t The number of tasks in the pool
     */
    std::size_t taskCount() const {
        std::unique_lock lock(queue_mutex);
        return tasks.size();
    }

    /**
     * @brief Resize the number of threads in the pool
     *
     * @param n_threads The number of threads in the pool
     */
    void resize(std::size_t n_threads) {
        std::unique_lock lock(queue_mutex);
        stop = true;
        condition.notify_all();
        lock.unlock();

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        threads.clear();
        stop = false;
        startThreads(n_threads);
    }

private:
    std::vector<std::jthread> threads;
    std::queue<std::function<void()>> tasks;

    mutable std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<int> active_count;

    /**
     * @brief Start the threads in the pool
     *
     * @param n_threads The number of threads in the pool
     */
    void startThreads(std::size_t n_threads) {
        for (std::size_t i = 0; i < n_threads; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock lock(queue_mutex);
                        condition.wait(
                            lock, [this] { return stop || !tasks.empty(); });

                        if (stop && tasks.empty()) {
                            return;
                        }

                        task = std::move(tasks.front());
                        tasks.pop();
                        ++active_count;
                    }

                    task();
                    --active_count;
                }
            });
        }
    }

    /**
     * @brief Stop the pool
     */
    void stopPool() {
        {
            std::unique_lock lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
};

}  // namespace atom::async

#endif  // ATOM_ASYNC_POOL_HPP
