/*
 * daemon.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-11-11

Description: Daemon process implementation

**************************************************/

#pragma once

#include <functional>
#include <ctime>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#endif

namespace Atom::Async
{
    // Class for managing process information
    class DaemonGuard
    {
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
         * @param mainCb The main callback function to be executed in the child process.
         * @return The return value of the main callback function.
         */
        int RealStart(int argc, char **argv,
                      std::function<int(int argc, char **argv)> mainCb);

        /**
         * @brief Starts the process. If a daemon process needs to be created, it will create the daemon process first.
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
        pid_t m_parentId = 0;         /**< The parent process ID. */
        pid_t m_mainId = 0;           /**< The child process ID. */
        time_t m_parentStartTime = 0; /**< The start time of the parent process. */
        time_t m_mainStartTime = 0;   /**< The start time of the child process. */
        int m_restartCount = 0;       /**< The number of restarts. */
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

}
