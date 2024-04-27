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
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Atom::Async {
/**
 * @brief 线程池
 */
class ThreadPool {
public:
    /**
     * @brief 构造函数，初始化线程池大小和停止标志位。
     *
     * 构造函数会创建 n_threads 个线程，并等待来自任务队列的任务分配。
     *
     * @param n_threads 线程池大小
     */
    ThreadPool(std::size_t n_threads) : stop(false) {
        for (std::size_t i = 0; i < n_threads; ++i) {
            threads.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(
                            lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
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
        for (std::thread &thread : threads) {
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
    auto enqueue(F &&f, Args &&...args)
        -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            tasks.emplace([task] { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

private:
    std::vector<std::thread> threads;         ///< 线程池中的线程列表
    std::queue<std::function<void()>> tasks;  ///< 任务队列

    std::mutex queue_mutex;             ///< 任务队列的互斥锁
    std::condition_variable condition;  ///< 任务队列的条件变量
    bool stop;                          ///< 停止标志位
};
}  // namespace Atom::Async

#endif