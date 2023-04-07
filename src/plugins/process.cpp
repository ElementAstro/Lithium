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

Date: 2023-3-31

Description: Process Manager

**************************************************/

#include <spdlog/spdlog.h>

#include "process.hpp"

#include <chrono>
#include <stdexcept>
#include <cstdlib>
#include <spdlog/spdlog.h>

#ifdef _WIN32
#include <process.h> // For _spawnvp on Windows
#else
#include <unistd.h> // For fork, execvp, and kill on Unix-based systems
#include <sys/wait.h>
#endif

namespace OpenAPT
{

    ProcessManager::ProcessManager(int maxProcesses) : maxProcesses(maxProcesses) {}

    ProcessManager::~ProcessManager()
    {
        stopAllProcesses();
        spdlog::shutdown();
    }

    void ProcessManager::startChildProcess(const std::string &name, std::function<void()> func)
    {
        // If process limit is exceeded, wait and try again
        while (m_processes.size() >= static_cast<unsigned long>(maxProcesses))
        {
            spdlog::warn("Process count exceeds limit. Waiting for a process to finish...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        try
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            auto newThread = std::make_unique<std::thread>(func);
            m_processes[name] = std::move(newThread);
            m_processStatus[name] = true;
        }
        catch (const std::exception &ex)
        {
            spdlog::error("Exception caught when starting child process {}: {}", name, ex.what());
        }
    }

    void ProcessManager::startIndependentProcess(const std::string &name, const std::string &command, const std::vector<std::string> &args)
    {
        // If process limit is exceeded, wait and try again
        while (m_processes.size() >= static_cast<unsigned long>(maxProcesses))
        {
            spdlog::warn("Process count exceeds limit. Waiting for a process to finish...");
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        // Combine command and arguments into a single vector
        std::vector<const char *> cArgs(args.size() + 2);
        cArgs[0] = command.c_str();
        for (std::vector<std::string>::size_type i = 0; i < args.size(); ++i)
        {
            cArgs[i + 1] = args[i].c_str();
        }
        cArgs[cArgs.size() - 1] = nullptr;

#ifdef _WIN32
        // Create a new process with _spawnvp
        int ret = _spawnvp(_P_DETACH, command.c_str(), cArgs.data());
        if (ret == -1)
        {
            spdlog::error("Error when starting independent process {}: {}", name, strerror(errno));
        }
        else
        {
            m_processStatus[name] = true;
        }
#else
        // Create a fork and execute the command in the child process with execvp
        pid_t pid = fork();
        if (pid == -1)
        {
            spdlog::error("Error when forking to start independent process {}: {}", name, strerror(errno));
        }
        else if (pid == 0)
        {
            // Child process
            execvp(command.c_str(), const_cast<char *const *>(cArgs.data()));
            // If execvp returns, there was an error
            spdlog::error("Error when executing independent process {}: {}", name, strerror(errno));
            exit(1);
        }
        else
        {
            // Parent process
            m_processStatus[name] = true;
        }
#endif
        // Log success or failure
        if (m_processStatus[name])
        {
            spdlog::info("Started independent process {}: {} {}", name, command, args.empty() ? "" : args[0]);
        }
        else
        {
            spdlog::error("Failed to start independent process {}: {} {}", name, command, args.empty() ? "" : args[0]);
        }
    }

    void ProcessManager::killChildProcess(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_processes.find(name) == m_processes.end())
        {
            spdlog::warn("Cannot kill child process {}. Process not found.", name);
            return;
        }
        // Check if process is joinable, and terminate with join or detach accordingly
        auto &proc = m_processes[name];
        if (proc->joinable())
        {
            proc->join();
            m_processes.erase(name);
            m_processStatus.erase(name);
        }
        else
        {
            spdlog::warn("Child process {} not joinable. Detaching...", name);
            proc->detach();
            m_processes.erase(name);
            m_processStatus.erase(name);
        }
        spdlog::info("Killed child process {}", name);
    }

    void ProcessManager::killIndependentProcess(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_processStatus.find(name) == m_processStatus.end())
        {
            spdlog::warn("Cannot kill process {}. Process not found.", name);
            return;
        }
#ifdef _WIN
        // Kill the process by name on Windows using taskkill
        std::string command = "taskkill /F /IM " + name + ".exe > nul 2>&1";
        int ret = system(command.c_str());
#else
        // Kill the process by name on Unix-based systems using pkill
        std::string command = "pkill -f " + name;
        int ret = system(command.c_str());
#endif
        if (ret != 0)
        {
            spdlog::error("Error when killing independent process {}: {}", name, strerror(errno));
        }
        else
        {
            m_processStatus[name] = false;
            spdlog::info("Killed independent process {}", name);
        }
    }

    void ProcessManager::stopAllProcesses()
    {
        m_stop = true;
        // Stop all child processes
        for (auto &it : m_processes)
        {
            killChildProcess(it.first);
        }
        // Stop all independent processes
        for (auto &it : m_processStatus)
        {
            if (it.second)
            {
                killIndependentProcess(it.first);
            }
        }
        spdlog::info("Stopped all processes");
    }

    std::vector<std::string> ProcessManager::listProcesses()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<std::string> processList;
        // Add child process names
        for (auto &it : m_processes)
        {
            processList.push_back(it.first + " (child)");
        }
        // Add independent process names
        for (auto &it : m_processStatus)
        {
            if (it.second)
            {
                processList.push_back(it.first + " (independent)");
            }
        }
        return processList;
    }

    bool ProcessManager::isProcessRunning(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
#ifdef _WIN
        // Check if independent process is running on Windows using tasklist
        std::string command = "tasklist /FI \"IMAGENAME eq " + name + ".exe\" /NH > nul 2>&1";
        return system(command.c_str()) == 0;
#else
        // Check if independent process is running on Unix-based systems using pgrep
        std::string command = "pgrep -f " + name;
        return system(command.c_str()) == 0;
#endif
    }
}

/*
int main() {
    // 初始化 spdlog

    // 创建进程管理器
    ProcessManager pm;

    // 启动一个子进程
    pm.startChildProcess("child_process", []() {
        spdlog::info("Child process started.");
        int count = 0;
        while (count < 5) {
            spdlog::info("Child process running... count = {}.", count);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            count++;
        }
    });

    // 启动一个独立进程
    pm.startIndependentProcess("independent_process", "ls", {"-a"});

    // 列出当前正在运行的进程
    auto processList = pm.listProcesses();
    spdlog::debug("Current running processes:");
    for (const auto& process : processList) {
        spdlog::debug("- {}", process);
    }

    // 等待子进程执行完毕
    while (pm.isProcessRunning("child_process")) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    sleep(10);

    // 终止独立进程

    // 终止所有进程
    pm.stopAllProcesses();

    sleep(10);

    return 0;
}
*/
