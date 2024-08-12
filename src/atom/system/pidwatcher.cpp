/*
 * pidwatcher.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-17

Description: PID Watcher

**************************************************/

#include "pidwatcher.hpp"

#include <filesystem>
#include <fstream>
#include <istream>

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on
#else
#include <dirent.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

#include "atom/log/loguru.hpp"

namespace atom::system {
PidWatcher::PidWatcher() : running_(false), monitoring_(false) {}

PidWatcher::~PidWatcher() { stop(); }

void PidWatcher::setExitCallback(Callback callback) {
    std::lock_guard lock(mutex_);
    exit_callback_ = std::move(callback);
}

void PidWatcher::setMonitorFunction(Callback callback,
                                    std::chrono::milliseconds interval) {
    std::lock_guard lock(mutex_);
    monitor_callback_ = std::move(callback);
    monitor_interval_ = interval;
}

auto PidWatcher::getPidByName(const std::string &name) const -> pid_t {
#ifdef _WIN32
    DWORD pidList[1024];
    DWORD cbNeeded;
    if (EnumProcesses(pidList, sizeof(pidList), &cbNeeded)) {
        for (unsigned int i = 0; i < cbNeeded / sizeof(DWORD); i++) {
            HANDLE processHandle = OpenProcess(
                PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pidList[i]);
            if (processHandle != nullptr) {
                char filename[MAX_PATH];
                if (GetModuleFileNameEx(processHandle, nullptr, filename,
                                        MAX_PATH)) {
                    std::string processName = strrchr(filename, '\\') + 1;
                    if (processName == name) {
                        CloseHandle(processHandle);
                        return pidList[i];
                    }
                }
                CloseHandle(processHandle);
            }
        }
    }
#else
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) {
            continue;
        }
        std::string proc_dir_path = std::string("/proc/") + entry->d_name;
        std::ifstream cmdline_file(proc_dir_path + "/cmdline");
        std::string cmdline;
        getline(cmdline_file, cmdline);
        if (cmdline == name) {
            closedir(dir);
            return atoi(entry->d_name);
        }
    }
    closedir(dir);
#endif
    return 0;
}

// 开始监视指定进程
auto PidWatcher::start(const std::string &name) -> bool {
    std::lock_guard lock(mutex_);

    if (running_) {
        LOG_F(ERROR, "Already running.");
        return false;
    }

    pid_ = getPidByName(name);
    if (pid_ == 0) {
        LOG_F(ERROR, "Failed to get PID.");
        return false;
    }

    running_ = true;
    monitoring_ = true;

#if __cplusplus >= 202002L
    monitor_thread_ = std::jthread(&PidWatcher::monitorThread, this);
    exit_thread_ = std::jthread(&PidWatcher::exitThread, this);
#else
    monitor_thread_ = std::thread(&PidWatcher::MonitorThread, this);
    exit_thread_ = std::thread(&PidWatcher::ExitThread, this);
#endif

    return true;
}

// 停止监视进程
void PidWatcher::stop() {
    std::lock_guard lock(mutex_);

    if (!running_) {
        return;
    }

    running_ = false;
    monitoring_ = false;

    exit_cv_.notify_one();
    monitor_cv_.notify_one();

    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
    if (exit_thread_.joinable()) {
        exit_thread_.join();
    }
}

// 切换目标进程
bool PidWatcher::Switch(const std::string &name) {
    std::lock_guard lock(mutex_);

    if (!running_) {
        LOG_F(ERROR, "Not running.");
        return false;
    }

    pid_ = getPidByName(name);
    if (pid_ == 0) {
        LOG_F(ERROR, "Failed to get PID.");
        return false;
    }

    monitor_cv_.notify_one();

    return true;
}

void PidWatcher::monitorThread() {
    while (true) {
        std::unique_lock lock(mutex_);

        while (!monitoring_ && running_) {
            monitor_cv_.wait(lock);
        }

        if (!running_) {
            break;
        }

        if (monitor_callback_) {
            monitor_callback_();
        }

#ifdef _WIN32
        HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, pid_);
        if (processHandle == nullptr) {
            LOG_F(ERROR, "Failed to open process.");
            break;
        }
        DWORD waitResult = WaitForSingleObject(processHandle, INFINITE);
        if (waitResult == WAIT_FAILED) {
            LOG_F(ERROR, "Failed to wait for process.");
            break;
        }
        CloseHandle(processHandle);
#else
        int status;
        pid_t waitResult = waitpid(pid_, &status, 0);
        if (waitResult == -1) {
            LOG_F(ERROR, "Failed to wait for process.");
            break;
        }
#endif

        if (!running_) {
            break;
        }

        if (exit_callback_) {
            exit_callback_();
        }

        lock.unlock();

        if (monitor_interval_.count() <= 0) {
            break;
        }

        std::this_thread::sleep_for(monitor_interval_);
    }
}

void PidWatcher::exitThread() {
    while (true) {
        std::unique_lock lock(mutex_);

        if (!running_) {
            break;
        }

        exit_cv_.wait(lock);

        if (!running_) {
            break;
        }

#ifdef _WIN32
        HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, pid_);
        if (processHandle == nullptr) {
            LOG_F(ERROR, "Failed to open process.");
            break;
        }
        DWORD waitResult = WaitForSingleObject(processHandle, 0);
        CloseHandle(processHandle);
#else
        int status;
        pid_t waitResult = waitpid(pid_, &status, WNOHANG);
#endif

        if (waitResult != 0) {
            if (exit_callback_) {
                exit_callback_();
            }
        }
    }
}

}  // namespace atom::system
