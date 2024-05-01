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

namespace atom::system {

/**
 * @brief A class for monitoring processes by their PID.
 *
 * This class allows monitoring of processes by their PID. It provides
 * functionality to set callbacks on process exit, set a monitor function to
 * run at intervals, get PID by process name, start monitoring a specific
 * process, stop monitoring, and switch the target process.
 */
class PidWatcher {
public:
    using Callback = std::function<void()>;

    /**
     * @brief Constructs a PidWatcher object.
     */
    PidWatcher();

    /**
     * @brief Destroys the PidWatcher object.
     */
    ~PidWatcher();

    /**
     * @brief Sets the callback function to be executed on process exit.
     *
     * @param callback The callback function to set.
     */
    void SetExitCallback(Callback callback);

    /**
     * @brief Sets the monitor function to be executed at specified intervals.
     *
     * @param callback The monitor function to set.
     * @param interval The interval at which the monitor function should run.
     */
    void SetMonitorFunction(Callback callback,
                            std::chrono::milliseconds interval);

    /**
     * @brief Retrieves the PID of a process by its name.
     *
     * @param name The name of the process.
     * @return The PID of the process.
     */
    pid_t GetPidByName(const std::string &name) const;

    /**
     * @brief Starts monitoring the specified process by name.
     *
     * @param name The name of the process to monitor.
     * @return True if monitoring started successfully, false otherwise.
     */
    bool Start(const std::string &name);

    /**
     * @brief Stops monitoring the currently monitored process.
     */
    void Stop();

    /**
     * @brief Switches the target process to monitor.
     *
     * @param name The name of the process to switch to.
     * @return True if the process was successfully switched, false otherwise.
     */
    bool Switch(const std::string &name);

private:
    /**
     * @brief The thread function for monitoring the process.
     */
    void MonitorThread();

    /**
     * @brief The thread function for handling process exit.
     */
    void ExitThread();

private:
    pid_t pid_;        ///< The PID of the currently monitored process.
    bool running_;     ///< Flag indicating if the monitoring is running.
    bool monitoring_;  ///< Flag indicating if a process is being monitored.

    Callback exit_callback_;  ///< Callback function to execute on process exit.
    Callback monitor_callback_;  ///< Monitor function to execute at intervals.
    std::chrono::milliseconds
        monitor_interval_;  ///< Interval for monitor function execution.

#if __cplusplus >= 202002L
    std::jthread monitor_thread_;  ///< Thread for monitoring the process.
    std::jthread exit_thread_;     ///< Thread for handling process exit.
#else
    std::thread monitor_thread_;  ///< Thread for monitoring the process.
    std::thread exit_thread_;     ///< Thread for handling process exit.
#endif

    std::mutex mutex_;  ///< Mutex for thread synchronization.
    std::condition_variable
        monitor_cv_;                   ///< Condition variable for monitoring.
    std::condition_variable exit_cv_;  ///< Condition variable for process exit.
};

}  // namespace atom::system

#endif