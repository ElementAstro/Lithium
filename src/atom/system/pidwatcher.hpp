/*
 * pidwatcher.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-17

Description: PID Watcher

**************************************************/

#ifndef ATOM_SYSTEM_PIDWATCHER_HPP
#define ATOM_SYSTEM_PIDWATCHER_HPP

#include <chrono>
#include <condition_variable>
#include <functional>
#include <thread>

#include <sys/types.h>

namespace Atom::System {

class PidWatcher {
public:
    using Callback = std::function<void()>;

    PidWatcher();

    ~PidWatcher();

    // 设置进程退出后的回调函数
    void SetExitCallback(Callback callback);

    // 设置监视函数，每隔一段时间运行一次
    void SetMonitorFunction(Callback callback,
                            std::chrono::milliseconds interval);

    // 根据进程名称获取PID
    pid_t GetPidByName(const std::string &name) const;

    // 开始监视指定进程
    bool Start(const std::string &name);

    // 停止监视进程
    void Stop();

    // 切换目标进程
    bool Switch(const std::string &name);

private:
    void MonitorThread();

    void ExitThread();

private:
    pid_t pid_;
    bool running_;
    bool monitoring_;

    Callback exit_callback_;
    Callback monitor_callback_;
    std::chrono::milliseconds monitor_interval_;

#if __cplusplus >= 202002L
    std::jthread monitor_thread_;
    std::jthread exit_thread_;
#else
    std::thread monitor_thread_;
    std::thread exit_thread_;
#endif

    std::mutex mutex_;
    std::condition_variable monitor_cv_;
    std::condition_variable exit_cv_;
};
}  // namespace Atom::System

#endif