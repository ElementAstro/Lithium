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

#include "process.hpp"

#include "loguru/loguru.hpp"

namespace Lithium::Process
{
    bool ProcessManager::createProcess(const std::string &command, const std::string &identifier)
    {
        pid_t pid;

#ifdef _WIN32
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};
        std::string cmd = "powershell.exe -Command \"" + command + "\"";
        if (!CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            LOG_F(ERROR, "Failed to create PowerShell process");
            return false;
        }
        pid = pi.dwProcessId;
#else
        pid = fork();

        if (pid == 0)
        {
            // Child process code
            LOG_F(INFO, "Running command: %s", command.c_str());
            int pipefd[2];
            pipe(pipefd);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);

            execlp(command.c_str(), command.c_str(), NULL);
            exit(0);
        }
        else if (pid < 0)
        {
            // Error handling
            LOG_F(ERROR, "Failed to create process");
            return false;
        }
#endif

        std::lock_guard<std::mutex> lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
        processes.push_back(process);
        LOG_F(INFO, "Process created: %s (PID: %d)", identifier.c_str(), pid);
        return true;
    }

    bool ProcessManager::runScript(const std::string &script, const std::string &identifier)
    {
        pid_t pid;

#ifdef _WIN32
        std::string cmd = "powershell.exe -Command \"" + script + "\"";
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};
        if (!CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            LOG_F(ERROR, "Failed to create process");
            return false;
        }
        pid = pi.dwProcessId;
#else
        pid = fork();

        if (pid == 0)
        {
            // Child process code
            LOG_F(INFO, "Running script: %s", script.c_str());

#ifdef __APPLE__
            execl("/bin/sh", "sh", "-c", script.c_str(), NULL);
#else
            execl("/bin/bash", "bash", "-c", script.c_str(), NULL);
#endif
        }
        else if (pid < 0)
        {
            // Error handling
            LOG_F(ERROR, "Failed to create process");
            return false;
        }
#endif

        std::lock_guard<std::mutex> lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
        processes.push_back(process);
        LOG_F(INFO, "Process created: %s (PID: %d)", identifier.c_str(), pid);
        return true;
    }

    bool ProcessManager::terminateProcess(pid_t pid, int signal)
    {
        auto it = std::find_if(processes.begin(), processes.end(), [pid](const Process &p)
                               { return p.pid == pid; });

        if (it != processes.end())
        {
#ifdef _WIN32
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
            if (hProcess != NULL)
            {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
                LOG_F(INFO, "Process terminated: %s (PID: %d)", it->name.c_str(), pid);
            }
            else
            {
                LOG_F(ERROR, "Failed to terminate process");
                return false;
            }
#else
            int status;
            kill(pid, signal);
            waitpid(pid, &status, 0);

            LOG_F(INFO, "Process terminated: %s (PID: %d)", it->name.c_str(), pid);
#endif

            processes.erase(it);
            cv.notify_one();
        }
        else
        {
            LOG_F(ERROR, "Process not found");
            return false;
        }
        return true;
    }

    bool ProcessManager::terminateProcessByName(const std::string &name, int signal)
    {
        auto it = std::find_if(processes.begin(), processes.end(), [&name](const Process &p)
                               { return p.name == name; });

        if (it != processes.end())
        {
            return terminateProcess(it->pid, signal);
        }
        LOG_F(ERROR, "Process not found by name: %s", name.c_str());
        return false;
    }

    void ProcessManager::listProcesses()
    {
        std::lock_guard<std::mutex> lock(mtx);
        LOG_F(INFO, "Currently running processes:");

        for (const auto &process : processes)
        {
            LOG_F(INFO, "%s (PID: %d)", process.name.c_str(), process.pid);
        }
    }

    std::vector<Process> ProcessManager::getRunningProcesses()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return processes;
    }

    std::vector<std::string> ProcessManager::getProcessOutput(const std::string &identifier)
    {
        auto it = std::find_if(processes.begin(), processes.end(), [&identifier](const Process &p)
                               { return p.name == identifier; });

        if (it != processes.end())
        {
            std::vector<std::string> outputLines;
            std::stringstream ss(it->output);
            std::string line;

            while (getline(ss, line))
            {
                outputLines.push_back(line);
            }

            return outputLines;
        }
        else
        {
            LOG_F(ERROR, "Process not found");
            return std::vector<std::string>();
        }
    }

    void ProcessManager::waitForCompletion()
    {
        for (const auto &process : processes)
        {
#ifdef _WIN32
            HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, process.pid);
            if (hProcess != NULL)
            {
                WaitForSingleObject(hProcess, INFINITE);
                CloseHandle(hProcess);
                LOG_F(INFO, "Process completed: %s (PID: %d)", process.name.c_str(), process.pid);
            }
            else
            {
                LOG_F(ERROR, "Failed to wait for process completion");
            }
#else
            int status;
            waitpid(process.pid, &status, 0);

            LOG_F(INFO, "Process completed: %s (PID: %d)", process.name.c_str(), process.pid);
#endif
        }

        processes.clear();
        LOG_F(INFO, "All processes completed.");
    }

}
