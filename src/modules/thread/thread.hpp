/*
 * thread.hpp
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

Date: 2023-3-27

Description: Thread Manager

**************************************************/

#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#include <functional>
#include <condition_variable>

namespace OpenAPT::Thread
{

    /**
     * @class ThreadManager
     * @brief 线程管理器类，用于管理多线程执行任务
     */
    class ThreadManager
    {
    public:
        /**
         * @brief 默认构造函数，使用默认参数初始化线程管理器
         */
        ThreadManager() : m_maxThreads(10), m_stopFlag(false) {}

        /**
         * @brief 构造函数，使用指定的最大线程数初始化线程管理器
         * @param maxThreads 最大线程数
         */
        ThreadManager(int maxThreads);

        /**
         * @brief 析构函数，销毁线程管理器
         */
        ~ThreadManager();

        /**
         * @brief 添加线程
         * @param func 线程执行的函数对象
         * @param name 线程的名称
         */
        void addThread(std::function<void()> func, const std::string &name);

        /**
         * @brief 等待所有线程完成
         */
        void joinAllThreads();

        /**
         * @brief 根据线程名称等待指定线程完成
         * @param name 线程的名称
         */
        void joinThreadByName(const std::string &name);

        /**
         * @brief 根据线程名称让指定线程休眠一段时间
         * @param name 线程的名称
         * @param seconds 休眠的秒数
         * @return 如果找到指定名称的线程并成功让其休眠，则返回 true，否则返回 false
         */
        bool sleepThreadByName(const std::string &name, int seconds);

        /**
         * @brief 判断指定名称的线程是否正在运行
         * @param name 线程的名称
         * @return 如果指定名称的线程正在运行，则返回 true，否则返回 false
         */
        bool isThreadRunning(const std::string &name);

    private:
        /**
         * @brief 根据给定的锁和线程元组对象等待线程完成
         * @param lock 线程锁
         * @param t 线程元组对象
         */
        void joinThread(std::unique_lock<std::mutex> &lock, std::tuple<std::unique_ptr<std::thread>, std::string, bool> &t);

        std::mutex m_mtx;                                                                   ///< 互斥锁，用于线程同步
        std::condition_variable m_cv;                                                       ///< 条件变量，用于线程同步
        std::vector<std::tuple<std::unique_ptr<std::thread>, std::string, bool>> m_threads; ///< 存储线程的容器
        std::atomic_bool m_stopFlag{false};                                                 ///< 停止标志，用于控制线程的停止
        int m_maxThreads;                                                                   ///< 最大线程数
    };

}