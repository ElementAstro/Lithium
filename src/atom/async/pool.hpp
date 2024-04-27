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
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace atom::async {
class ThreadPool {
public:
    /**
     * @brief 构造函数，初始化线程池大小和停止标志位。
     *
     * 构造函数会创建 n_threads 个线程，并等待来自任务队列的任务分配。
     *
     * @param n_threads 线程池大小
     */
    explicit ThreadPool(std::size_t n_threads) : stop(false) {
        for (std::size_t i = 0; i < n_threads; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock,
                                   [this] { return stop || !tasks.empty(); });
                    if (stop && tasks.empty()) {
                        return;
                    }
                    auto task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }

    /**
     * @brief 析构函数，销毁所有线程并退出。
     *
     * 析构函数会向任务队列中插入空任务，并等待所有线程完成该任务并退出。
     */
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto &thread : threads) {
            thread.join();
        }
    }

    /**
     * @brief 将指定任务添加到任务队列中，并返回该任务的 future 对象。
     *
     * 该函数用于将函数 f 和其参数 args 添加到任务队列中等待执行，并返回一
     * 个 std::future 对象，以便查询任务完成情况。当任务队列已满或线程池被
     * 停止时，将会抛出 std::runtime_error 异常。
     *
     * @tparam F 函数类型
     * @tparam Args 参数类型
     * @param f 要执行的函数对象
     * @param args 函数参数
     * @return 返回一个 std::future 对象，用于查询任务完成情况
     */
    template <typename F, typename... Args>
    auto enqueue(F &&f, Args &&...args) {
        using return_type = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        auto res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            tasks.emplace([task = std::move(task)]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    /**
     * @brief 等待所有任务完成。
     *
     * 该函数会等待任务队列中的所有任务完成，然后返回。
     */
    void wait() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        condition.wait(lock, [this] { return tasks.empty(); });
    }

    /**
     * @brief 返回线程池中的线程数量。
     *
     * @return 线程池中的线程数量
     */
    std::size_t size() const { return threads.size(); }

    /**
     * @brief 返回任务队列中待执行的任务数量。
     *
     * @return 任务队列中待执行的任务数量
     */
    std::size_t taskCount() const {
        std::unique_lock<std::mutex> lock(queue_mutex);
        return tasks.size();
    }

private:
    std::vector<std::thread> threads;         ///< 线程池中的线程列表
    std::queue<std::function<void()>> tasks;  ///< 任务队列

    mutable std::mutex queue_mutex;     ///< 任务队列的互斥锁
    std::condition_variable condition;  ///< 任务队列的条件变量
    bool stop;                          ///< 停止标志位
};
}  // namespace atom::async

#endif