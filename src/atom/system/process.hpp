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

#include <condition_variable>
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace atom::system {
struct Process {
    int pid;
    std::string name;
    std::string output;
    fs::path path;
    std::string status;
};

class ProcessManager {
public:
    /**
     * 创建一个进程管理器。
     * @param maxProcess 最大进程数。
     */
    explicit ProcessManager(int maxProcess = 10);

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    /**
     * 创建一个进程管理器。
     * @param maxProcess 最大进程数。
     */
    static std::shared_ptr<ProcessManager> createShared(int maxProcess = 10);

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
    bool terminateProcess(int pid, int signal = 15 /*SIGTERM*/);

    /**
     * 终止一个进程。
     * @param name 要终止的进程的名称。
     * @param signal 终止信号，默认为SIGTERM。
     */
    bool terminateProcessByName(const std::string &name,
                                int signal = 15 /*SIGTERM*/);

    /**
     * 检查是否存在指定进程。
     * @param identifier 进程的标识符。
     * @return 是否存在指定进程。
     */
    bool hasProcess(const std::string &identifier);

    [[nodiscard]] std::vector<Process> getRunningProcesses() const;

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
    std::shared_mutex mtx;  ///< 互斥锁，用于操作进程列表。 // Mutex used for
                            ///< manipulating the process list.
};

/**
 * 获取所有进程信息。
 * @return 所有进程信息。
 */
std::vector<std::pair<int, std::string>> getAllProcesses();

/*
 * 获取当前进程信息。
 */
[[nodiscard("The process info is not used")]] Process getSelfProcessInfo();

/**
 * @brief Returns the name of the controlling terminal.
 *
 * This function returns the name of the controlling terminal associated with
 * the current process.
 *
 * @return The name of the controlling terminal.
 */
[[nodiscard]] std::string ctermid();

/**
 * @brief Returns the priority of the current process.
 *
 * This function returns the priority of the current process.
 *
 * @return The priority of the current process.
 */
std::optional<int> getProcessPriorityByPid(int pid);

/**
 * @brief Returns the priority of the current process.
 *
 * This function returns the priority of the current process.
 *
 * @return The priority of the current process.
 */
std::optional<int> getProcessPriorityByName(const std::string &name);

/**
 * @brief Returns the priority of the current process.
 *
 * This function returns the priority of the current process.
 *
 * @return The priority of the current process.
 */
bool isProcessRunning(const std::string &processName);

/**
 * @brief Returns the priority of the current process.
 *
 * This function returns the priority of the current process.
 *
 * @return The priority of the current process.
 */
int getParentProcessId(int processId);

}  // namespace atom::system

#endif
