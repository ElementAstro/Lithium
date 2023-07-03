/*
 * threadpool.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-2

Description: Simple Thread Pool

**************************************************/

#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <vector>

namespace OpenAPT::Thread
{
    /**
     * @class ThreadPool
     * @brief 线程池类，用于管理多线程执行任务
     */
    class ThreadPool
    {
    public:
        /**
         * @brief 构造函数，使用指定的线程数量初始化线程池
         * @param num_threads 线程数量
         */
        explicit ThreadPool(size_t num_threads);

        /**
         * @brief 析构函数，销毁线程池
         */
        ~ThreadPool();

        /**
         * @brief 添加任务到线程池
         * @tparam F 任务函数类型
         * @tparam Args 任务函数参数类型
         * @param f 任务函数
         * @param args 任务函数参数
         */
        template <typename F, typename... Args>
        void enqueue(F &&f, Args &&...args)
        {
            auto task = std::make_shared<std::packaged_task<void()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            {
                std::unique_lock<std::mutex> lock(mutex_);
                tasks_.emplace([task]()
                               { (*task)(); });
            }
            condition_.notify_one();
        }

        /**
         * @brief 等待所有任务完成
         */
        void wait();

    private:
        std::vector<std::thread> workers_;        ///< 存储线程的容器
        std::queue<std::function<void()>> tasks_; ///< 存储任务的队列
        std::mutex mutex_;                        ///< 互斥锁，用于线程同步
        std::condition_variable condition_;       ///< 条件变量，用于线程同步
        bool stop_;                               ///< 停止标志，用于控制线程的停止
    };
}
