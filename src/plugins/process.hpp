/*
 * process.hpp
 * 
 * Copyright (C) 2023 Max Qian <lightapt.com>
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
 
Copyright: 2023 Max Qian. All rights reserved
 
Author: Max Qian

E-mail: astro_air@126.com
 
Date: 2023-3-31
 
Description: Process Manager
 
**************************************************/

#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <map>
#include <functional>

namespace OpenAPT {

    /**
     * @brief A class for managing child and independent processes
     */
    class ProcessManager {
        public:
            /**
             * @brief Constructor
             * @param maxProcesses Maximum number of concurrent processes allowed
             */
            ProcessManager(int maxProcesses);

            /**
             * @brief Destructor, stops all processes and shuts down logger
             */
            ~ProcessManager();

            /**
             * @brief Starts a child process and runs the specified function
             * @param name Name of the child process
             * @param func Function to run in the child process
             */
            void startChildProcess(const std::string& name, std::function<void()> func);

            /**
             * @brief Starts an independent process and runs the specified command with arguments
             * @param name Name of the process
             * @param command Command to run
             * @param args Arguments for the command
             */
            void startIndependentProcess(const std::string& name, const std::string& command, const std::vector<std::string>& args);

            /**
             * @brief Kills the specified child process
             * @param name Name of the child process to kill
             */
            void killChildProcess(const std::string& name);

            /**
             * @brief Kills the specified independent process
             * @param name Name of the process to kill
             */
            void killIndependentProcess(const std::string& name);

            /**
             * @brief Stops all processes
             */
            void stopAllProcesses();

            /**
             * @brief Lists all currently running process names
             * @return A vector of all currently running process names
             */
            std::vector<std::string> listProcesses();

            /**
             * @brief Checks if the specified process is currently running
             * @param name Name of the process to check
             * @return True if the process is running, false otherwise
             */
            bool isProcessRunning(const std::string& name);

        private:
            const int maxProcesses; // Maximum number of concurrent processes allowed
            std::map<std::string, std::unique_ptr<std::thread>> m_processes; // Map of process names to thread pointers for child processes
            std::map<std::string, bool> m_processStatus; // Map of process names to status for independent processes
            bool m_stop = false; // Flag to stop all processes
            std::mutex m_mutex; // Mutex for thread-safe access to process maps
    };
}

