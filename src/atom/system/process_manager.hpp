/*
 * process.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-24

Description: Enhanced Process Manager with additional functionalities.

**************************************************/

#ifndef ATOM_SYSTEM_PROCESS_MANAGER_HPP
#define ATOM_SYSTEM_PROCESS_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>

#include "process_info.hpp"

#include "atom/error/exception.hpp"

#include "atom/macro.hpp"

namespace atom::system {

/**
 * @class ProcessException
 * @brief Base exception class for process-related errors.
 */
class ProcessException : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_PROCESS_ERROR(...)                                         \
    throw atom::system::ProcessException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                         ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_PROCESS_ERROR(...)            \
    atom::system::ProcessException::rethrowNested( \
        ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, __VA_ARGS__)

/**
 * @class ProcessManager
 * @brief Manages system processes with enhanced functionalities.
 */
class ProcessManager {
public:
    /**
     * @brief Constructs a ProcessManager with a maximum number of processes.
     * @param maxProcess The maximum number of processes to manage.
     */
    explicit ProcessManager(int maxProcess = 20);

    /**
     * @brief Destroys the ProcessManager.
     */
    ~ProcessManager();

    /**
     * @brief Creates a shared pointer to a ProcessManager.
     * @param maxProcess The maximum number of processes to manage.
     * @return A shared pointer to a ProcessManager.
     */
    static auto createShared(int maxProcess = 20)
        -> std::shared_ptr<ProcessManager>;

    /**
     * @brief Creates a new process.
     * @param command The command to execute.
     * @param identifier An identifier for the process.
     * @param isBackground Whether to run the process in the background.
     * @return True if the process was created successfully, otherwise false.
     * @throws ProcessException if process creation fails.
     */
    auto createProcess(const std::string &command,
                       const std::string &identifier,
                       bool isBackground = false) -> bool;

    /**
     * @brief Terminates a process by its PID.
     * @param pid The process ID.
     * @param signal The signal to send to the process (default is SIGTERM).
     * @return True if the process was terminated successfully, otherwise false.
     * @throws ProcessException if termination fails.
     */
    auto terminateProcess(int pid, int signal = 15 /*SIGTERM*/) -> bool;

    /**
     * @brief Terminates a process by its name.
     * @param name The process name.
     * @param signal The signal to send to the process (default is SIGTERM).
     * @return True if the process was terminated successfully, otherwise false.
     * @throws ProcessException if termination fails.
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
     * @param isBackground Whether to run the script in the background.
     * @return True if the script was run successfully, otherwise false.
     * @throws ProcessException if script execution fails.
     */
    auto runScript(const std::string &script, const std::string &identifier,
                   bool isBackground = false) -> bool;

    /**
     * @brief Monitors the managed processes and updates their statuses.
     * @return True if monitoring was successful, otherwise false.
     */
    auto monitorProcesses() -> bool;

    /**
     * @brief Retrieves detailed information about a specific process.
     * @param pid The process ID.
     * @return A Process structure with detailed information.
     * @throws ProcessException if retrieval fails.
     */
    auto getProcessInfo(int pid) -> Process;

#ifdef _WIN32
    /**
     * @brief Gets the handle of a process by its PID (Windows only).
     * @param pid The process ID.
     * @return The handle of the process.
     * @throws ProcessException if retrieval fails.
     */
    auto getProcessHandle(int pid) const -> void *;
#else
    /**
     * @brief Gets the file path of a process by its PID (non-Windows).
     * @param pid The process ID.
     * @param file The file name.
     * @return The file path of the process.
     * @throws ProcessException if retrieval fails.
     */
    static auto getProcFilePath(int pid,
                                const std::string &file) -> std::string;
#endif

private:
    class ProcessManagerImpl;  ///< Forward declaration of implementation class
    std::unique_ptr<ProcessManagerImpl> impl;  ///< Pointer to implementation
};

}  // namespace atom::system

#endif
