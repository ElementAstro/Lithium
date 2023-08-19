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

#include "single_thread.hpp"

namespace Lithium
{

    SingleThreadPool::SingleThreadPool()
        : isThreadAboutToQuit(false)
    {
    }

    SingleThreadPool::~SingleThreadPool()
    {
        quit();
    }

    void SingleThreadPool::start(const std::function<void(const std::atomic_bool &isAboutToClose)> &functionToRun)
    {
        std::unique_lock<std::mutex> lock(runLock);
        pendingFunction = functionToRun;
        isFunctionAboutToQuit = true;
        acquire.notify_one();

        // wait for run
        if (std::this_thread::get_id() != thread.get_id())
            released.wait(lock, [&]()
                          { return pendingFunction == nullptr; });
    }

    bool SingleThreadPool::tryStart(const std::function<void(const std::atomic_bool &)> &functionToRun)
    {
        std::unique_lock<std::mutex> lock(runLock);
        if (runningFunction != nullptr)
            return false;

        isFunctionAboutToQuit = true;
        pendingFunction = functionToRun;
        acquire.notify_one();

        // wait for run
        if (std::this_thread::get_id() != thread.get_id())
            released.wait(lock, [&]()
                          { return pendingFunction == nullptr; });

        return true;
    }

    void SingleThreadPool::quit()
    {
        start(nullptr);
    }

} // namespace Lithium
