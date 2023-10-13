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
#include "config.h"

#if defined(_WIN32)
#include <windows.h>
#include <tlhelp32.h>
#elif defined(__linux__)
#include <dirent.h>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <libproc.h>
#else
#error "不支持的操作系统"
#endif

#include "loguru/loguru.hpp"

namespace Lithium::Process
{
    std::shared_ptr<ProcessManager> ProcessManager::createShared()
    {
        return std::make_shared<ProcessManager>();
    }

    std::shared_ptr<ProcessManager> ProcessManager::createShared(int maxProcess)
    {
        return std::make_shared<ProcessManager>(maxProcess);
    }

    bool ProcessManager::createProcess(const std::string &command, const std::string &identifier)
    {
        pid_t pid;

#ifdef _WIN32
        STARTUPINFO si{};
        PROCESS_INFORMATION pi{};
        std::string cmd = "powershell.exe -Command \"" + command + "\"";
        if (!CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            DLOG_F(ERROR, _("Failed to create PowerShell process"));
            return false;
        }
        pid = pi.dwProcessId;
#else
        pid = fork();

        if (pid == 0)
        {
            // Child process code
            DLOG_F(INFO, _("Running command: {}"), command);
            int pipefd[2];
            int result = pipe(pipefd);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[0]);
            close(pipefd[1]);

            execlp(command.c_str(), command.c_str(), NULL);
            exit(0);
        }
        else if (pid < 0)
        {
            // Error handling
            DLOG_F(ERROR, _("Failed to create process"));
            return false;
        }
#endif

        std::lock_guard<std::mutex> lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
        processes.push_back(process);
        DLOG_F(INFO, _("Process created: {} (PID: {})"), identifier, pid);
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
            DLOG_F(ERROR, _("Failed to create process"));
            return false;
        }
        pid = pi.dwProcessId;
#else
        pid = fork();

        if (pid == 0)
        {
            // Child process code
            DLOG_F(INFO, _("Running script: {}"), script);

#ifdef __APPLE__
            execl("/bin/sh", "sh", "-c", script.c_str(), NULL);
#else
            execl("/bin/bash", "bash", "-c", script.c_str(), NULL);
#endif
        }
        else if (pid < 0)
        {
            // Error handling
            DLOG_F(ERROR, _("Failed to create process"));
            return false;
        }
#endif

        std::lock_guard<std::mutex> lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
        processes.push_back(process);
        DLOG_F(INFO, _("Process created: {} (PID: {})"), identifier, pid);
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
                DLOG_F(INFO, _("Process terminated: {} (PID: {})"), it->name, pid);
            }
            else
            {
                DLOG_F(ERROR, _("Failed to terminate process"));
                return false;
            }
#else
            int status;
            kill(pid, signal);
            waitpid(pid, &status, 0);

            DLOG_F(INFO, _("Process terminated: {} (PID: {})"), it->name, pid);
#endif

            processes.erase(it);
            cv.notify_one();
        }
        else
        {
            DLOG_F(ERROR, _("Process not found"));
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
        DLOG_F(ERROR, _("Process not found by name: {}"), name);
        return false;
    }

    void ProcessManager::listProcesses()
    {
        std::lock_guard<std::mutex> lock(mtx);
        DLOG_F(INFO, _("Currently running processes:"));

        for (const auto &process : processes)
        {
            DLOG_F(INFO, _("{} (PID: {})"), process.name, process.pid);
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
            DLOG_F(ERROR, _("Process not found"));
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
                DLOG_F(INFO, _("Process completed: {} (PID: {})"), process.name, process.pid);
            }
            else
            {
                DLOG_F(ERROR, _("Failed to wait for process completion"));
            }
#else
            int status;
            waitpid(process.pid, &status, 0);

            DLOG_F(INFO, _("Process completed: %s (PID: %d)"), process.name.c_str(), process.pid);
#endif
        }

        processes.clear();
        DLOG_F(INFO, _("All processes completed."));
    }

#if defined(_WIN32)
    std::vector<std::pair<int, std::string>> GetAllProcesses()
    {
        std::vector<std::pair<int, std::string>> processes;

        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE)
        {
            DLOG_F(ERROR, _("Failed to create process snapshot"));
            return processes;
        }

        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(processEntry);

        if (Process32First(snapshot, &processEntry))
        {
            do
            {
                int pid = processEntry.th32ProcessID;
                std::string name = processEntry.szExeFile;
                processes.push_back(std::make_pair(pid, name));
            } while (Process32Next(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
        return processes;
    }
#elif defined(__linux__)
    std::string GetProcessName(int pid)
    {
        std::string name;
        std::string path = "/proc/" + std::to_string(pid) + "/comm";
        std::ifstream commFile(path);
        if (commFile)
        {
            std::getline(commFile, name);
        }
        commFile.close();
        return name;
    }

    std::vector<std::pair<int, std::string>> GetAllProcesses()
    {
        std::vector<std::pair<int, std::string>> processes;

        DIR *procDir = opendir("/proc");
        if (!procDir)
        {
            DLOG_F(ERROR, _("Failed to open /proc directory"));
            return processes;
        }

        dirent *entry;
        while ((entry = readdir(procDir)) != nullptr)
        {
            if (entry->d_type == DT_DIR)
            {
                char *end;
                long pid = strtol(entry->d_name, &end, 10);
                if (*end == '\0')
                {
                    std::string name = GetProcessName(pid);
                    processes.push_back(std::make_pair(pid, name));
                }
            }
        }

        closedir(procDir);
        return processes;
    }

#elif defined(__APPLE__)
    std::string GetProcessName(int pid)
    {
        char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
        if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) <= 0)
        {
            DLOG_F(ERROR, _("Failed to get process path"));
            return "";
        }
        std::string path(pathbuf);
        size_t slashPos = path.rfind('/');
        if (slashPos != std::string::npos)
        {
            return path.substr(slashPos + 1);
        }
        return path;
    }

    std::vector<std::pair<int, std::string>> GetAllProcesses()
    {
        std::vector<std::pair<int, std::string>> processes;

        int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
        size_t length = 0;

        if (sysctl(mib, 4, nullptr, &length, nullptr, 0) == -1)
        {
            DLOG_F(ERROR, _("Failed to get process info length"));
            return processes;
        }

        struct kinfo_proc *procBuf = (struct kinfo_proc *)malloc(length);
        if (!procBuf)
        {
            DLOG_F(ERROR, _("Failed to allocate memory"));
            return processes;
        }

        if (sysctl(mib, 4, procBuf, &length, nullptr, 0) == -1)
        {
            DLOG_F(ERROR, _("Failed to get process info"));
            free(procBuf);
            return processes;
        }

        int procCount = length / sizeof(struct kinfo_proc);
        for (int i = 0; i < procCount; ++i)
        {
            int pid = procBuf[i].kp_proc.p_pid;
            std::string name = GetProcessName(pid);
            processes.push_back(std::make_pair(pid, name));
        }

        free(procBuf);
        return processes;
    }
#else
#error "Unsupported operating system"
#endif

}
