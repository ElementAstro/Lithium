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

namespace OpenAPT {

    class ThreadManager {
        public:
            ThreadManager() : m_maxThreads(10), m_stopFlag(false) {}
            ThreadManager(size_t maxThreads) : m_maxThreads(maxThreads), m_stopFlag(false) {}

            ~ThreadManager();
            // 添加线程并启动
            void addThread(std::function<void()> func, const std::string& name);
            // 结束所有线程
            void joinAllThreads();
            // 结束指定名称的线程
            void joinThreadByName(const std::string& name);

            bool sleepThreadByName(const std::string& name, int seconds);

            bool isThreadRunning(const std::string& name);
            
        private:

            bool isThreadNameExist(const std::string& name) const {
                return std::any_of(m_threads.begin(), m_threads.end(),
                    [&](const auto& t) { return std::get<1>(t) == name; });
            }

            const size_t m_maxThreads;
            bool m_stopFlag;
            std::mutex m_mtx;
            std::condition_variable m_cv;
            std::vector<std::tuple<std::unique_ptr<std::thread>, std::string, bool>> m_threads;
    };

}