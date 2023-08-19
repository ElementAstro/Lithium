/*
 * single_thread.hpp
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

Date: 2023-4-9

Description: Single Thread Pool (indisinglethreadpool)

**************************************************/

#ifndef SINGLE_THREAD_POOL_H
#define SINGLE_THREAD_POOL_H

#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace Lithium
{

    class SingleThreadPool
    {
    public:
        SingleThreadPool();
        ~SingleThreadPool();

        void start(const std::function<void(const std::atomic_bool &isAboutToClose)> &functionToRun);
        bool tryStart(const std::function<void(const std::atomic_bool &)> &functionToRun);
        void quit();

    private:
        std::atomic_bool isThreadAboutToQuit;
        std::atomic_bool isFunctionAboutToQuit;
        std::function<void(const std::atomic_bool &isAboutToClose)> runningFunction;
        std::function<void(const std::atomic_bool &)> pendingFunction;
        std::mutex runLock;
        std::condition_variable acquire;
        std::condition_variable released;
        std::thread thread;
    };

} // namespace LIHTIUM

#endif // SINGLE_THREAD_POOL_H
