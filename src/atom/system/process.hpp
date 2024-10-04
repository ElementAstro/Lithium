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

/**
 * @struct Process
 * @brief Represents a system process.
 */
struct Process {
    int pid;             ///< Process ID.
    std::string name;    ///< Process name.
    std::string output;  ///< Process output.
    fs::path path;       ///< Path to the process executable.
    std::string status;  ///< Process status.
#if _WIN32
    void *handle;  ///< Handle to the process (Windows only).
#endif
};

/**
 * @struct NetworkConnection
 * @brief Represents a network connection.
 */
struct NetworkConnection {
    std::string protocol;  ///< Protocol used by the connection.
    std::string line;      ///< Connection details.
} ATOM_ALIGNAS(64);

/**
 * @class ProcessManager
 * @brief Manages system processes.
 */
class ProcessManager {
public:
    /**
     * @brief Constructs a ProcessManager with a maximum number of processes.
     * @param maxProcess The maximum number of processes to manage.
     */
    explicit ProcessManager(int maxProcess = 10);

    /**
     * @brief Destroys the ProcessManager.
     */
    ~ProcessManager();

    /**
     * @brief Creates a shared pointer to a ProcessManager.
     * @param maxProcess The maximum number of processes to manage.
     * @return A shared pointer to a ProcessManager.
     */
    static auto createShared(int maxProcess = 10)
        -> std::shared_ptr<ProcessManager>;

    /**
     * @brief Creates a new process.
     * @param command The command to execute.
     * @param identifier An identifier for the process.
     * @return True if the process was created successfully, otherwise false.
     */
    auto createProcess(const std::string &command,
                       const std::string &identifier) -> bool;

    /**
     * @brief Terminates a process by its PID.
     * @param pid The process ID.
     * @param signal The signal to send to the process (default is SIGTERM).
     * @return True if the process was terminated successfully, otherwise false.
     */
    auto terminateProcess(int pid, int signal = 15 /*SIGTERM*/) -> bool;

    /**
     * @brief Terminates a process by its name.
     * @param name The process name.
     * @param signal The signal to send to the process (default is SIGTERM).
     * @return True if the process was terminated successfully, otherwise false.
     */
    auto terminateProcessByName(const std::string &name,
                                int signal = 15 /*SIGTERM*/) -> bool;

    /**
     * @brief Checks if a process with the given identifier exists.
     * @param identifier The process identifier.
     * @return True if the process exists, otherwise false.
     */
    auto hasProcess(const std::string &identifier) -> bool;

    /**
     * @brief Gets a list of running processes.
     * @return A vector of running processes.
     */
    [[nodiscard]] auto getRunningProcesses() const -> std::vector<Process>;

    /**
     * @brief Gets the output of a process by its identifier.
     * @param identifier The process identifier.
     * @return A vector of strings containing the process output.
     */
    [[nodiscard]] auto getProcessOutput(const std::string &identifier)
        -> std::vector<std::string>;

    /**
     * @brief Waits for all managed processes to complete.
     */
    void waitForCompletion();

    /**
     * @brief Runs a script as a new process.
     * @param script The script to run.
     * @param identifier An identifier for the process.
     * @return True if the script was run successfully, otherwise false.
     */
    auto runScript(const std::string &script,
                   const std::string &identifier) -> bool;

    /**
     * @brief Monitors the managed processes.
     * @return True if monitoring was successful, otherwise false.
     */
    auto monitorProcesses() -> bool;

#ifdef _WIN32
    /**
     * @brief Gets the handle of a process by its PID (Windows only).
     * @param pid The process ID.
     * @return The handle of the process.
     */
    auto getProcessHandle(int pid) const -> void *;
#else
    /**
     * @brief Gets the file path of a process by its PID (non-Windows).
     * @param pid The process ID.
     * @param file The file name.
     * @return The file path of the process.
     */
    static auto getProcFilePath(int pid,
                                const std::string &file) -> std::string;
#endif

private:
    class ProcessManagerImpl;  ///< Forward declaration of implementation class
    std::unique_ptr<ProcessManagerImpl> impl;  ///< Pointer to implementation
};

/**
 * @brief Gets information about all processes.
 * @return A vector of pairs containing process IDs and names.
 */
auto getAllProcesses() -> std::vector<std::pair<int, std::string>>;

/**
 * @brief Gets information about the current process.
 * @return A Process struct containing information about the current process.
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

/**
 * @brief Gets the network connections of a process by its PID.
 * @param pid The process ID.
 * @return A vector of NetworkConnection structs representing the network
 * connections.
 */
auto getNetworkConnections(int pid) -> std::vector<NetworkConnection>;

/**
 * @brief Gets the process IDs of processes with the specified name.
 * @param processName The name of the process.
 * @return A vector of process IDs.
 */
auto getProcessIdByName(const std::string &processName) -> std::vector<int>;

}  // namespace atom::system

#endif
