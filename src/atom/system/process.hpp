/*
 * process.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Process Manager

**************************************************/

#ifndef ATOM_SYSTEM_PROCESS_HPP
#define ATOM_SYSTEM_PROCESS_HPP

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace atom::system {
struct Process {
    pid_t pid;
    std::string name;
    std::string output;
    std::string path;
    std::string status;
};

class ProcessManager {
public:
    /**
     * 创建一个进程管理器。
     */
    ProcessManager();

    /**
     * 创建一个进程管理器。
     * @param maxProcess 最大进程数。
     */
    ProcessManager(int maxProcess);

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    /**
     * 创建一个进程管理器。
     */
    static std::shared_ptr<ProcessManager> createShared();

    /**
     * 创建一个进程管理器。
     * @param maxProcess 最大进程数。
     */
    static std::shared_ptr<ProcessManager> createShared(int maxProcess);

    // -------------------------------------------------------------------
    // Process methods
    // -------------------------------------------------------------------

    /**
     * 创建一个新的进程。
     * @param command 要执行的命令。
     * @param identifier 进程的标识符。
     */
    bool createProcess(const std::string &command,
                       const std::string &identifier);

    /**
     * 终止一个进程。
     * @param pid 要终止的进程的PID。
     * @param signal 终止信号，默认为SIGTERM。
     */
    bool terminateProcess(pid_t pid, int signal = SIGTERM);

    /**
     * 终止一个进程。
     * @param name 要终止的进程的名称。
     * @param signal 终止信号，默认为SIGTERM。
     */
    bool terminateProcessByName(const std::string &name, int signal = SIGTERM);

    /**
     * 检查是否存在指定进程。
     * @param identifier 进程的标识符。
     * @return 是否存在指定进程。
     */
    bool hasProcess(const std::string &identifier);

    [[nodiscard]] std::vector<Process> getRunningProcesses();

    /**
     * 获取指定进程的输出信息。
     * @param identifier 进程的标识符。
     * @return 进程的输出信息。
     */
    [[nodiscard]] std::vector<std::string> getProcessOutput(
        const std::string &identifier);

    /**
     * 等待所有进程完成并清除进程列表。
     */
    void waitForCompletion();

    /**
     * 运行一个脚本。
     * @param script 要运行的脚本。
     * @param identifier 进程的标识符。
     */
    bool runScript(const std::string &script, const std::string &identifier);

    // -------------------------------------------------------------------
    // Script methods
    // -------------------------------------------------------------------

private:
    int m_maxProcesses;  ///< 最大进程数。 // Maximum number of processes.
    std::condition_variable
        cv;  ///< 条件变量，用于等待进程完成。 // Condition variable used to
             ///< wait for process completion.
    std::vector<Process> processes;  ///< 存储当前运行的进程列表。 // Stores the
                                     ///< list of currently running processes.
    std::mutex mtx;  ///< 互斥锁，用于操作进程列表。 // Mutex used for
                     ///< manipulating the process list.
};

/**
 * 获取所有进程信息。
 * @return 所有进程信息。
 */
std::vector<std::pair<int, std::string>> GetAllProcesses();

/*
 * 获取当前进程信息。
 */
Process GetSelfProcessInfo();

}  // namespace atom::system

#endif
