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
PidWatcher::PidWatcher() : running_(false), monitoring_(false) {
    LOG_F(INFO, "PidWatcher constructor called");
}

PidWatcher::~PidWatcher() {
    LOG_F(INFO, "PidWatcher destructor called");
    stop();
}

void PidWatcher::setExitCallback(Callback callback) {
    LOG_F(INFO, "Setting exit callback");
    std::lock_guard lock(mutex_);
    exit_callback_ = std::move(callback);
}

void PidWatcher::setMonitorFunction(Callback callback,
                                    std::chrono::milliseconds interval) {
    LOG_F(INFO, "Setting monitor function with interval: {} ms",
          interval.count());
    std::lock_guard lock(mutex_);
    monitor_callback_ = std::move(callback);
    monitor_interval_ = interval;
}

auto PidWatcher::getPidByName(const std::string &name) const -> pid_t {
    LOG_F(INFO, "Getting PID by name: {}", name);
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
                        LOG_F(INFO, "Found PID: {} for name: {}", pidList[i],
                              name);
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
            LOG_F(INFO, "Found PID: {} for name: {}", entry->d_name, name);
            return atoi(entry->d_name);
        }
    }
    closedir(dir);
#endif
    LOG_F(WARNING, "PID not found for name: {}", name);
    return 0;
}

// 开始监视指定进程
auto PidWatcher::start(const std::string &name) -> bool {
    LOG_F(INFO, "Starting PidWatcher for process name: {}", name);
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
    monitor_thread_ = std::thread(&PidWatcher::monitorThread, this);
    exit_thread_ = std::thread(&PidWatcher::exitThread, this);
#endif

    LOG_F(INFO, "PidWatcher started for process name: {}", name);
    return true;
}

// 停止监视进程
void PidWatcher::stop() {
    LOG_F(INFO, "Stopping PidWatcher");
    std::lock_guard lock(mutex_);

    if (!running_) {
        LOG_F(INFO, "PidWatcher is not running");
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

    LOG_F(INFO, "PidWatcher stopped");
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

    LOG_F(INFO, "PidWatcher switched to process name: {}", name);
    return true;
}

void PidWatcher::monitorThread() {
    LOG_F(INFO, "Monitor thread started");
    while (true) {
        std::unique_lock lock(mutex_);

        while (!monitoring_ && running_) {
            monitor_cv_.wait(lock);
        }

        if (!running_) {
            LOG_F(INFO, "Monitor thread exiting");
            break;
        }

        if (monitor_callback_) {
            LOG_F(INFO, "Executing monitor callback");
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
            LOG_F(INFO, "Monitor thread exiting");
            break;
        }

        if (exit_callback_) {
            LOG_F(INFO, "Executing exit callback");
            exit_callback_();
        }

        lock.unlock();

        if (monitor_interval_.count() <= 0) {
            break;
        }

        std::this_thread::sleep_for(monitor_interval_);
    }
    LOG_F(INFO, "Monitor thread exited");
}

void PidWatcher::exitThread() {
    LOG_F(INFO, "Exit thread started");
    while (true) {
        std::unique_lock lock(mutex_);

        if (!running_) {
            LOG_F(INFO, "Exit thread exiting");
            break;
        }

        exit_cv_.wait(lock);

        if (!running_) {
            LOG_F(INFO, "Exit thread exiting");
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
                LOG_F(INFO, "Executing exit callback");
                exit_callback_();
            }
        }
    }
    LOG_F(INFO, "Exit thread exited");
}

}  // namespace atom::system