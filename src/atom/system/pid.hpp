/*
 * pid.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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