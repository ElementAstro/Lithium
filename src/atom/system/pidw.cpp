/*
 * pidw.cpp
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

Date: 2023-4-4

Description: PID Watcher with Network

**************************************************/

#include "pidw.hpp"

#include <sstream>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#else
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#endif

#include "atom/log/loguru.hpp"

PidWWatcher::PidWWatcher(const std::string &processName) : processName_(processName), isMonitoring_(false) {}

void PidWWatcher::start()
{
    isMonitoring_ = true;
    thread_ = std::thread(&PidWWatcher::MonitorThread, this);
}

void PidWWatcher::stop()
{
    isMonitoring_ = false;
    if (thread_.joinable())
    {
        thread_.join();
    }
}

double PidWWatcher::GetNetworkUsage()
{
#ifdef _WIN32
    // TODO: 在Windows平台上获取网络访问量
    return 0;
#else
    std::ifstream file("/proc/net/dev");
    if (!file)
    {
        throw std::runtime_error("Failed to open /proc/net/dev");
    }

    std::string line;
    double rxBytes = 0, txBytes = 0;
    while (std::getline(file, line))
    {
        if (line.find("eth0") != std::string::npos || line.find("wlan0") != std::string::npos)
        {
            std::istringstream ss(line);
            std::string interface;
            ss >> interface >> rxBytes >> txBytes;
            break;
        }
    }

    return rxBytes + txBytes;
#endif
}

double PidWWatcher::GetMemoryUsage()
{
#ifdef _WIN32
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid_);
    if (processHandle == nullptr)
    {
        throw std::runtime_error("Failed to open process!");
    }

    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (!GetProcessMemoryInfo(processHandle, reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&pmc), sizeof(pmc)))
    {
        CloseHandle(processHandle);
        throw std::runtime_error("Failed to get process memory info!");
    }

    CloseHandle(processHandle);

    return pmc.PrivateUsage / 1024.0; // 转换为KB
#else
    std::ifstream file("/proc/" + std::to_string(pid_) + "/status");
    if (!file)
    {
        throw std::runtime_error("Failed to open /proc/" + std::to_string(pid_) + "/status");
    }

    std::string line;
    double memoryUsage = 0;
    while (std::getline(file, line))
    {
        if (line.find("VmRSS:") != std::string::npos)
        {
            std::istringstream ss(line);
            std::string label;
            double value;
            ss >> label >> value;
            memoryUsage = value;
            break;
        }
    }

    return memoryUsage;
#endif
}

void PidWWatcher::MonitorThread()
{
#ifdef _WIN32
    // 获取进程ID
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
    {
        LOG_F(ERROR, "Failed to enumerate processes!");
        return;
    }

    cProcesses = cbNeeded / sizeof(DWORD);
    for (DWORD i = 0; i < cProcesses; i++)
    {
        if (aProcesses[i] != 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);
            if (hProcess != nullptr)
            {
                HMODULE hMod;
                DWORD cbNeeded;
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
                {
                    TCHAR szModName[MAX_PATH];
                    if (GetModuleBaseName(hProcess, hMod, szModName, sizeof(szModName) / sizeof(TCHAR)))
                    {
                        std::string moduleName(szModName);
                        if (moduleName == processName_)
                        {
                            pid_ = aProcesses[i];
                            break;
                        }
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }

    if (pid_ == 0)
    {
        LOG_F(ERROR, "Failed to find process!");
        return;
    }
#else
    // 获取进程ID
    std::ifstream file("/proc/self/stat");
    if (!file)
    {
        LOG_F(ERROR, "Failed to read /proc/self/stat");
        return;
    }

    std::string stat;
    std::getline(file, stat);

    std::istringstream ss(stat);
    int pid;
    ss >> pid;

    DIR *procDir = opendir("/proc");
    if (procDir == nullptr)
    {
        LOG_F(ERROR, "Failed to open /proc");
        return;
    }

    dirent *dirEntry;
    while ((dirEntry = readdir(procDir)) != nullptr)
    {
        if (dirEntry->d_type == DT_DIR)
        {
            std::string fileName = dirEntry->d_name;

            try
            {
                int processId = std::stoi(fileName);
                if (processId == pid)
                {
                    continue;
                }

                std::ifstream cmdlineFile("/proc/" + fileName + "/cmdline");
                if (!cmdlineFile)
                {
                    continue;
                }

                std::string cmdline;
                std::getline(cmdlineFile, cmdline);

                if (cmdline.find(processName_) != std::string::npos)
                {
                    pid_ = processId;
                    break;
                }
            }
            catch (const std::invalid_argument &ex)
            {
                // 跳过无效的目录名
                continue;
            }
        }
    }

    closedir(procDir);

    if (pid_ == 0)
    {
        LOG_F(ERROR, "Failed to find process!");
        return;
    }
#endif

    double prevNetworkUsage = 0;
    double prevMemoryUsage = 0;

    while (isMonitoring_)
    {
        double networkUsage = GetNetworkUsage();
        double memoryUsage = GetMemoryUsage();

        DLOG_F(INFO, "Network Usage: {:.2f} bytes", networkUsage - prevNetworkUsage);
        DLOG_F(INFO, "Memory Usage: {:.2f} KB", memoryUsage - prevMemoryUsage);

        prevNetworkUsage = networkUsage;
        prevMemoryUsage = memoryUsage;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
