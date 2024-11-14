// async_system_command.hpp
#pragma once

#include <atomic>
#include <mutex>
#include <string>

/**
 * @class AsyncSystemCommand
 * @brief A class to execute system commands asynchronously.
 *
 * This class provides functionality to run system commands asynchronously,
 * check their running status, and terminate them if needed.
 */
class AsyncSystemCommand {
public:
    /**
     * @brief Constructs an AsyncSystemCommand with the given command.
     * @param cmd The system command to be executed.
     */
    explicit AsyncSystemCommand(const std::string& cmd);

    /**
     * @brief Destructor for AsyncSystemCommand.
     *
     * Ensures that any running command is terminated before the object is
     * destroyed.
     */
    ~AsyncSystemCommand();

    /**
     * @brief Runs the system command asynchronously.
     *
     * If a command is already running, this function will log a warning and
     * return.
     */
    void run();

    /**
     * @brief Terminates the running system command.
     *
     * If no command is running, this function will log an informational message
     * and return.
     */
    void terminate();

    /**
     * @brief Checks if the system command is currently running.
     * @return True if the command is running, false otherwise.
     */
    bool isRunning() const;

private:
    std::string cmd_;         ///< The system command to be executed.
    std::atomic<pid_t> pid_;  ///< The process ID of the running command.
    std::atomic<bool>
        running_;  ///< Indicates whether the command is currently running.
    mutable std::mutex
        mutex_;  ///< Mutex to protect access to shared resources.
};