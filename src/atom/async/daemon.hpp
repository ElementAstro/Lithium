/*
 * daemon.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-11

Description: Daemon process implementation

**************************************************/

#ifndef ATOM_SERVER_DAEMON_HPP
#define ATOM_SERVER_DAEMON_HPP

#include <atomic>
#include <cstring>
#include <ctime>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace atom::async {
// Class for managing process information
class DaemonGuard {
public:
    /**
     * @brief Default constructor.
     */
    DaemonGuard() {}

    /**
     * @brief Converts process information to a string.
     *
     * @return The process information as a string.
     */
    std::string ToString() const;

    /**
     * @brief Starts a child process to execute the actual task.
     *
     * @param argc The number of command line arguments.
     * @param argv An array of command line arguments.
     * @param mainCb The main callback function to be executed in the child
     * process.
     * @return The return value of the main callback function.
     */
    int RealStart(int argc, char **argv,
                  std::function<int(int argc, char **argv)> mainCb);

    /**
     * @brief Starts a child process to execute the actual task.
     *
     * @param argc The number of command line arguments.
     * @param argv An array of command line arguments.
     * @param mainCb The main callback function to be executed in the child
     * process.
     * @return The return value of the main callback function.
     */
    int RealDaemon(int argc, char **argv,
                   std::function<int(int argc, char **argv)> mainCb);

    /**
     * @brief Starts the process. If a daemon process needs to be created, it
     * will create the daemon process first.
     *
     * @param argc The number of command line arguments.
     * @param argv An array of command line arguments.
     * @param mainCb The main callback function to be executed.
     * @param isDaemon Determines if a daemon process should be created.
     * @return The return value of the main callback function.
     */
    int StartDaemon(int argc, char **argv,
                    std::function<int(int argc, char **argv)> mainCb,
                    bool isDaemon);

private:
#ifdef _WIN32
    HANDLE m_parentId = 0;
    HANDLE m_mainId = 0;
#else
    pid_t m_parentId = 0; /**< The parent process ID. */
    pid_t m_mainId = 0;   /**< The child process ID. */
#endif
    time_t m_parentStartTime = 0; /**< The start time of the parent process. */
    time_t m_mainStartTime = 0;   /**< The start time of the child process. */
    std::atomic<int> m_restartCount{0}; /**< The number of restarts. */
};

/**
 * @brief Signal handler function.
 *
 * @param signum The signal number.
 */
void SignalHandler(int signum);

/**
 * @brief Writes the process ID to a file.
 */
void WritePidFile();

/**
 * @brief Checks if the process ID file exists.
 *
 * @return True if the process ID file exists, false otherwise.
 */
bool CheckPidFile();

}  // namespace atom::async

#endif
