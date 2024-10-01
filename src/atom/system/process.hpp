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

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include "macro.hpp"

namespace fs = std::filesystem;

namespace atom::system {
struct Process {
    int pid;
    std::string name;
    std::string output;
    fs::path path;
    std::string status;
#if _WIN32
    void *handle;
#endif
};

struct NetworkConnection {
    std::string protocol;
    std::string line;
} ATOM_ALIGNAS(64);

class ProcessManager {
public:
    explicit ProcessManager(int maxProcess = 10);
    ~ProcessManager();

    static auto createShared(int maxProcess = 10)
        -> std::shared_ptr<ProcessManager>;

    auto createProcess(const std::string &command,
                       const std::string &identifier) -> bool;

    auto terminateProcess(int pid, int signal = 15 /*SIGTERM*/) -> bool;
    auto terminateProcessByName(const std::string &name,
                                int signal = 15 /*SIGTERM*/) -> bool;

    auto hasProcess(const std::string &identifier) -> bool;
    [[nodiscard]] auto getRunningProcesses() const -> std::vector<Process>;
    [[nodiscard]] auto getProcessOutput(const std::string &identifier)
        -> std::vector<std::string>;

    void waitForCompletion();
    auto runScript(const std::string &script,
                   const std::string &identifier) -> bool;

    auto monitorProcesses() -> bool;

#ifdef _WIN32
    auto getProcessHandle(int pid) const -> void *;
#else
    static auto getProcFilePath(int pid,
                                const std::string &file) -> std::string;
#endif

private:
    class ProcessManagerImpl;  // Forward declaration of implementation class
    std::unique_ptr<ProcessManagerImpl> impl;  // Pointer to implementation
};

/**
 * 获取所有进程信息。
 * @return 所有进程信息。
 */
auto getAllProcesses() -> std::vector<std::pair<int, std::string>>;

/*
 * 获取当前进程信息。
 */
[[nodiscard("The process info is not used")]] auto getSelfProcessInfo()
    -> Process;

/**
 * @brief Returns the name of the controlling terminal.
 *
 * This function returns the name of the controlling terminal associated with
 * the current process.
 *
 * @return The name of the controlling terminal.
 */
[[nodiscard]] auto ctermid() -> std::string;

/**
 * @brief Returns the priority of a process by its PID.
 *
 * This function retrieves the priority of a process given its process ID (PID).
 * If the process is not found or an error occurs, an empty std::optional is
 * returned.
 *
 * @param pid The process ID of the target process.
 * @return std::optional<int> The priority of the process if found, otherwise an
 * empty std::optional.
 */
auto getProcessPriorityByPid(int pid) -> std::optional<int>;

/**
 * @brief Returns the priority of a process by its name.
 *
 * This function retrieves the priority of a process given its name.
 * If the process is not found or an error occurs, an empty std::optional is
 * returned.
 *
 * @param name The name of the target process.
 * @return std::optional<int> The priority of the process if found, otherwise an
 * empty std::optional.
 */
auto getProcessPriorityByName(const std::string &name) -> std::optional<int>;

/**
 * @brief Checks if a process is running by its name.
 *
 * This function checks if a process with the specified name is currently
 * running.
 *
 * @param processName The name of the process to check.
 * @return bool True if the process is running, otherwise false.
 */
auto isProcessRunning(const std::string &processName) -> bool;

/**
 * @brief Returns the parent process ID of a given process.
 *
 * This function retrieves the parent process ID (PPID) of a specified process.
 * If the process is not found or an error occurs, the function returns -1.
 *
 * @param processId The process ID of the target process.
 * @return int The parent process ID if found, otherwise -1.
 */
auto getParentProcessId(int processId) -> int;

/**
 * @brief Creates a process as a specified user.
 *
 * This function creates a new process using the specified user credentials.
 * It logs in the user, duplicates the user token, and creates a new process
 * with the specified command. This function is only available on Windows.
 *
 * @param command The command to be executed by the new process.
 * @param username The username of the user account.
 * @param domain The domain of the user account.
 * @param password The password of the user account.
 * @return bool True if the process is created successfully, otherwise false.
 */
auto _CreateProcessAsUser(const std::string &command,
                          const std::string &username,
                          const std::string &domain,
                          const std::string &password) -> bool;

auto getNetworkConnections(int pid) -> std::vector<NetworkConnection>;

auto getProcessIdByName(const std::string &processName) -> std::vector<int>;
}  // namespace atom::system

#endif
