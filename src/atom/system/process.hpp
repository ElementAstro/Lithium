/*
 * process.cpp
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

Date: 2023-7-19

Description: Process Manager

**************************************************/

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <mutex>
#include <fstream>
#include <sstream>
#include <condition_variable>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

namespace Atom::System
{
    struct Process
    {
        pid_t pid;
        std::string name;
        std::string output;
        std::string path;
        std::string status;
    };

    class ProcessManager
    {
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

        /**
         * 创建一个进程管理器。
         */
        static std::unique_ptr<ProcessManager> createUnique();

        /**
         * 创建一个进程管理器。
         * @param maxProcess 最大进程数。
         */
        static std::unique_ptr<ProcessManager> createUnique(int maxProcess);

        // -------------------------------------------------------------------
        // Process methods
        // -------------------------------------------------------------------

        /**
         * 创建一个新的进程。
         * @param command 要执行的命令。
         * @param identifier 进程的标识符。
         */
        bool createProcess(const std::string &command, const std::string &identifier);

        /**
         * 终止一个进程。
         * @param pid 要终止的进程的PID。
         * @param signal 终止信号，默认为SIGTERM。
         */
        bool terminateProcess(pid_t pid, int signal = SIGTERM);

        bool terminateProcessByName(const std::string &name, int signal = SIGTERM);

        [[nodiscard]] std::vector<Process> getRunningProcesses();

        /**
         * 获取指定进程的输出信息。
         * @param identifier 进程的标识符。
         * @return 进程的输出信息。
         */
        [[nodiscard]] std::vector<std::string> getProcessOutput(const std::string &identifier);

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
        int m_maxProcesses;             ///< 最大进程数。 // Maximum number of processes.
        std::condition_variable cv;     ///< 条件变量，用于等待进程完成。 // Condition variable used to wait for process completion.
        std::vector<Process> processes; ///< 存储当前运行的进程列表。 // Stores the list of currently running processes.
        std::mutex mtx;                 ///< 互斥锁，用于操作进程列表。 // Mutex used for manipulating the process list.
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

}
