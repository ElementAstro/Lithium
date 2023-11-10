/*
 * pid.hpp
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

Date: 2023-4-4

Description: PID Watcher

**************************************************/

#pragma once

#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <memory>
#include <vector>

class PIDWatcher
{
public:
    PIDWatcher(const std::string &processName);
    ~PIDWatcher();

    void start();
    void stop();
    void watch();

    void setCallback(const std::function<void(int, int)> &callback);

private:
    std::wstring getWideString(const std::string &str);

private:
    std::string processName_;
    std::jthread thread_;
    std::mutex callbackMutex_;
    std::function<void(int, int)> callback_;
    std::atomic<bool> isRunning_;
    std::atomic<bool> shouldStop_;
};

class PIDWatcherManager
{
public:
    PIDWatcherManager() = default;
    ~PIDWatcherManager();

    void addWatcher(const std::string &processName);
    void startAll();
    void stopAll();
    void setCallbackForAll(const std::function<void(int, int)> &callback);

private:
    std::vector<std::shared_ptr<PIDWatcher>> watchers_;
};