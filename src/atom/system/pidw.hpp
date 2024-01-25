/*
 * pidw.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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
