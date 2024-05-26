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
#include <windows.h>
#include <psapi.h>
#else
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace fs = std::filesystem;

#include "atom/log/loguru.hpp"

namespace atom::system {
PidWatcher::PidWatcher() : running_(false), monitoring_(false) {}

PidWatcher::~PidWatcher() { Stop(); }

void PidWatcher::SetExitCallback(Callback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    exit_callback_ = std::move(callback);
}

void PidWatcher::SetMonitorFunction(Callback callback,
                                    std::chrono::milliseconds interval) {
    std::lock_guard<std::mutex> lock(mutex_);
    monitor_callback_ = std::move(callback);
    monitor_interval_ = interval;
}

pid_t PidWatcher::GetPidByName(const std::string &name) const {
#ifdef _WIN32
    DWORD pid_list[1024], cb_needed;
    if (EnumProcesses(pid_list, sizeof(pid_list), &cb_needed)) {
        for (unsigned int i = 0; i < cb_needed / sizeof(DWORD); i++) {
            HANDLE process_handle =
                OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE,
                            pid_list[i]);
            if (process_handle != NULL) {
                char filename[MAX_PATH];
                if (GetModuleFileNameEx(process_handle, NULL, filename,
                                        MAX_PATH)) {
                    std::string process_name = strrchr(filename, '\\') + 1;
                    if (process_name == name) {
                        CloseHandle(process_handle);
                        return pid_list[i];
                    }
                }
                CloseHandle(process_handle);
            }
        }
    }
#else
    DIR *dir = opendir("/proc");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
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
bool PidWatcher::Start(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (running_) {
        LOG_F(ERROR, "Already running.");
        return false;
    }

    pid_ = GetPidByName(name);
    if (pid_ == 0) {
        LOG_F(ERROR, "Failed to get PID.");
        return false;
    }

    running_ = true;
    monitoring_ = true;

#if __cplusplus >= 202002L
    monitor_thread_ = std::jthread(&PidWatcher::MonitorThread, this);
    exit_thread_ = std::jthread(&PidWatcher::ExitThread, this);
#else
    monitor_thread_ = std::thread(&PidWatcher::MonitorThread, this);
    exit_thread_ = std::thread(&PidWatcher::ExitThread, this);
#endif

    return true;
}

// 停止监视进程
void PidWatcher::Stop() {
    std::lock_guard<std::mutex> lock(mutex_);

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
    std::lock_guard<std::mutex> lock(mutex_);

    if (!running_) {
        LOG_F(ERROR, "Not running.");
        return false;
    }

    pid_ = GetPidByName(name);
    if (pid_ == 0) {
        LOG_F(ERROR, "Failed to get PID.");
        return false;
    }

    monitor_cv_.notify_one();

    return true;
}

void PidWatcher::MonitorThread() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);

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
        HANDLE process_handle = OpenProcess(SYNCHRONIZE, FALSE, pid_);
        if (process_handle == NULL) {
            LOG_F(ERROR, "Failed to open process.");
            break;
        }
        DWORD wait_result = WaitForSingleObject(process_handle, INFINITE);
        CloseHandle(process_handle);
#else
        int status;
        pid_t wait_result = waitpid(pid_, &status, 0);
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

void PidWatcher::ExitThread() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);

        if (!running_) {
            break;
        }

        exit_cv_.wait(lock);

        if (!running_) {
            break;
        }

#ifdef _WIN32
        HANDLE process_handle = OpenProcess(SYNCHRONIZE, FALSE, pid_);
        if (process_handle == NULL) {
            LOG_F(ERROR, "Failed to open process.");
            break;
        }
        DWORD wait_result = WaitForSingleObject(process_handle, 0);
        CloseHandle(process_handle);
#else
        int status;
        pid_t wait_result = waitpid(pid_, &status, WNOHANG);
#endif

        if (wait_result != 0) {
            if (exit_callback_) {
                exit_callback_();
            }
        }
    }
}

}  // namespace atom::system
