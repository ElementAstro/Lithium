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

namespace Lithium::Thread
{

    class ThreadManager
    {
    public:
        explicit ThreadManager(int maxThreads);
        ~ThreadManager();

        void addThread(std::function<void()> func, const std::string &name);
        void joinAllThreads();
        void joinThreadByName(const std::string &name);
        bool sleepThreadByName(const std::string &name, int seconds);
        bool isThreadRunning(const std::string &name);

    private:
#if __cplusplus >= 202002L
        void joinThread(std::unique_lock<std::mutex> &lock, std::tuple<std::unique_ptr<std::jthread>, std::string, bool> &t);
#else
        void joinThread(std::unique_lock<std::mutex> &lock, std::tuple<std::unique_ptr<std::thread>, std::string, bool> &t);
#endif
        int m_maxThreads;
#if __cplusplus >= 202002L
        std::vector<std::tuple<std::unique_ptr<std::jthread>, std::string, bool>> m_threads;
#else
        std::vector<std::tuple<std::unique_ptr<std::thread>, std::string, bool>> m_threads;
#endif
        std::mutex m_mtx;
        std::condition_variable m_cv;
        std::atomic<bool> m_stopFlag;
    };
}