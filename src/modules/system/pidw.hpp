/*
 * pidw.hpp
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

Description: PID Watcher with Network

**************************************************/

#pragma once

#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

class PidWWatcher
{
public:
    PidWWatcher(const std::string &processName);

    void start();
    void stop();

private:
    std::string processName_;
#ifdef _WIN32
    DWORD pid_ = 0;
#else
    int pid_ = 0;
#endif
    std::thread thread_;
    bool isMonitoring_;

    double GetNetworkUsage();
    double GetMemoryUsage();

    void MonitorThread();
};
