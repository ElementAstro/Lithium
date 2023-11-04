/*
 * system.cpp
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

Date: 2023-6-17

Description: System

**************************************************/

#include "system.hpp"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <Windows.h>
#include <tlhelp32.h>
#include <pdh.h>
#include <Psapi.h>
#include <iphlpapi.h>
#elif __linux__
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/statfs.h>
#include <iterator>
#include <csignal>
#include <signal.h>
#elif __APPLE__
#include <mach/mach_init.h>
#include <mach/task_info.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#endif

namespace Lithium::System
{

    bool CheckSoftwareInstalled(const std::string &software_name)
    {
        bool is_installed = false;
#if defined(_WIN32)
        HKEY hKey;
        std::string regPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            char subKeyName[255];
            DWORD subKeyNameSize = 255;
            for (DWORD i = 0; RegEnumKeyEx(hKey, i, subKeyName, &subKeyNameSize, NULL, NULL, NULL, NULL) != ERROR_NO_MORE_ITEMS; i++)
            {
                HKEY hSubKey;
                if (RegOpenKeyEx(hKey, subKeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS)
                {
                    char displayName[255];
                    DWORD displayNameSize = 255;
                    if (RegQueryValueEx(hSubKey, "DisplayName", NULL, NULL, reinterpret_cast<LPBYTE>(displayName), &displayNameSize) == ERROR_SUCCESS)
                    {
                        if (softwareName == displayName)
                        {
                            RegCloseKey(hSubKey);
                            RegCloseKey(hKey);
                            is_installed = true;
                        }
                    }
                    RegCloseKey(hSubKey);
                }
                subKeyNameSize = 255;
            }
            RegCloseKey(hKey);
        }
#elif defined(__APPLE__)
        std::string command = "mdfind \"kMDItemKind == 'Application' && kMDItemFSName == '*" + software_name + "*.app'\"";
        FILE *pipe = popen(command.c_str(), "r");
        if (pipe)
        {
            char buffer[128];
            std::string result = "";
            while (!feof(pipe))
            {
                if (fgets(buffer, 128, pipe) != nullptr)
                {
                    result += buffer;
                }
            }

            pclose(pipe);

            is_installed = !result.empty();
        }
#elif defined(__linux__)
        std::string command = "which " + software_name + " > /dev/null 2>&1";
        int result = std::system(command.c_str());

        is_installed = (result == 0);
#endif

        return is_installed;
    }

    float GetCpuUsage()
    {
        float cpu_usage = 0.0;

#ifdef _WIN32
        PDH_HQUERY query;
        PdhOpenQuery(nullptr, 0, &query);

        PDH_HCOUNTER counter;
        PdhAddCounter(query, "\\Processor(_Total)\\% Processor Time", 0, &counter);
        PdhCollectQueryData(query);

        PDH_FMT_COUNTERVALUE counter_value;
        PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &counter_value);

        cpu_usage = static_cast<float>(counter_value.doubleValue);

        PdhCloseQuery(query);
#elif __linux__
        std::ifstream file("/proc/stat");
        std::string line;
        std::getline(file, line); // 读取第一行

        std::istringstream iss(line);
        std::vector<std::string> tokens(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

        unsigned long total_time = 0;
        for (size_t i = 1; i < tokens.size(); i++)
        {
            total_time += std::stoul(tokens[i]);
        }

        unsigned long idle_time = std::stoul(tokens[4]);

        float usage = static_cast<float>(total_time - idle_time) / total_time;
        cpu_usage = usage * 100.0;
#elif __APPLE__
        task_info_data_t tinfo;
        mach_msg_type_number_t task_info_count = TASK_INFO_MAX;
        if (task_info(mach_task_self(), TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&tinfo), &task_info_count) == KERN_SUCCESS)
        {
            cpu_usage = static_cast<float>(tinfo->cpu_ticks[CPU_STATE_USER] + tinfo->cpu_ticks[CPU_STATE_SYSTEM]) / tinfo->cpu_ticks[CPU_STATE_IDLE];
            cpu_usage *= 100.0;
        }
#endif

        return cpu_usage;
    }

    float GetCpuTemperature()
    {
        float temperature = 0.0f;

#ifdef _WIN32
        HKEY hKey;
        DWORD temperatureValue = 0;
        DWORD size = sizeof(DWORD);

        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&temperatureValue, &size) == ERROR_SUCCESS)
            {
                temperature = static_cast<float>(temperatureValue) / 10.0f;
            }
            RegCloseKey(hKey);
        }
#elif defined(__APPLE__)
        FILE *pipe = popen("sysctl -a | grep machdep.xcpm.cpu_thermal_level", "r");
        if (pipe != nullptr)
        {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                std::string result(buffer);
                size_t pos1 = result.find(": ");
                size_t pos2 = result.find("\n");
                if (pos1 != std::string::npos && pos2 != std::string::npos)
                {
                    std::string tempStr = result.substr(pos1 + 2, pos2 - pos1 - 2);
                    try
                    {
                        temperature = std::stof(tempStr);
                    }
                    catch (const std::exception &e)
                    {
                    }
                }
            }
            pclose(pipe);
        }
#elif defined(__linux__)
        std::ifstream tempFile("/sys/class/thermal/thermal_zone0/temp");
        if (tempFile.is_open())
        {
            int temp = 0;
            tempFile >> temp;
            tempFile.close();
            temperature = static_cast<float>(temp) / 1000.0f; // 温度以摄氏度为单位
        }
#endif

        return temperature;
    }

    float GetMemoryUsage()
    {
        float memory_usage = 0.0;

#ifdef _WIN32
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);

        float total_memory = static_cast<float>(status.ullTotalPhys / 1024 / 1024);
        float available_memory = static_cast<float>(status.ullAvailPhys / 1024 / 1024);

        memory_usage = (total_memory - available_memory) / total_memory * 100.0;
#elif __linux__
        std::ifstream file("/proc/meminfo");
        std::string line;

        unsigned long total_memory = 0;
        unsigned long free_memory = 0;
        unsigned long buffer_memory = 0;
        unsigned long cache_memory = 0;

        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string name;
            unsigned long value;

            if (iss >> name >> value)
            {
                if (name == "MemTotal:")
                {
                    total_memory = value;
                }
                else if (name == "MemFree:")
                {
                    free_memory = value;
                }
                else if (name == "Buffers:")
                {
                    buffer_memory = value;
                }
                else if (name == "Cached:")
                {
                    cache_memory = value;
                }
            }
        }

        unsigned long used_memory = total_memory - free_memory - buffer_memory - cache_memory;
        memory_usage = static_cast<float>(used_memory) / total_memory * 100.0;
#elif __APPLE__
        struct statfs stats;
        statfs("/", &stats);

        unsigned long long total_space = stats.f_blocks * stats.f_bsize;
        unsigned long long free_space = stats.f_bfree * stats.f_bsize;

        unsigned long long used_space = total_space - free_space;
        memory_usage = static_cast<float>(used_space) / total_space * 100.0;
#endif

        return memory_usage;
    }

    std::vector<std::pair<std::string, float>> GetDiskUsage()
    {
        std::vector<std::pair<std::string, float>> disk_usage;

#ifdef _WIN32
        DWORD drives = GetLogicalDrives();
        char drive_letter = 'A';

        while (drives)
        {
            if (drives & 1)
            {
                std::string drive_path = std::string(1, drive_letter) + ":\\";
                ULARGE_INTEGER total_space, free_space;

                if (GetDiskFreeSpaceExA(drive_path.c_str(), nullptr, &total_space, &free_space))
                {
                    unsigned long long total = total_space.QuadPart / (1024 * 1024);
                    unsigned long long free = free_space.QuadPart / (1024 * 1024);

                    float usage = 100.0 * static_cast<float>(total - free) / total;
                    disk_usage.push_back(std::make_pair(drive_path, usage));
                }
            }

            drives >>= 1;
            drive_letter++;
        }
#elif __linux__ || __APPLE__
        std::ifstream file("/proc/mounts");
        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string device, path;
            iss >> device >> path;

            struct statfs stats;
            if (statfs(path.c_str(), &stats) == 0)
            {
                unsigned long long totalSpace = static_cast<unsigned long long>(stats.f_blocks) * stats.f_bsize;
                unsigned long long freeSpace = static_cast<unsigned long long>(stats.f_bfree) * stats.f_bsize;

                unsigned long long usedSpace = totalSpace - freeSpace;
                float usage = static_cast<float>(usedSpace) / totalSpace * 100.0;
                disk_usage.push_back({path, usage});
            }
        }

#endif

        return disk_usage;
    }

    bool IsRoot()
    {
#ifdef _WIN32
        HANDLE hToken;
        TOKEN_ELEVATION elevation;
        DWORD dwSize;

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            return false;
        }

        if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
        {
            CloseHandle(hToken);
            return false;
        }

        bool elevated = (elevation.TokenIsElevated != 0);
        CloseHandle(hToken);
        return elevated;
#else
        return (getuid() == 0);
#endif
    }

    std::vector<std::pair<std::string, std::string>> GetProcessInfo()
    {
        std::vector<std::pair<std::string, std::string>> processInfo;

#ifdef _WIN32
        // 使用 Windows API 获取进程信息和文件地址
        DWORD processes[1024];
        DWORD cbNeeded;
        if (EnumProcesses(processes, sizeof(processes), &cbNeeded))
        {
            DWORD numProcesses = cbNeeded / sizeof(DWORD);
            for (DWORD i = 0; i < numProcesses; i++)
            {
                DWORD processId = processes[i];
                if (processId != 0)
                {
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
                    if (hProcess != NULL)
                    {
                        char filename[MAX_PATH];
                        if (GetModuleFileNameExA(hProcess, NULL, filename, sizeof(filename)))
                        {
                            std::string processName = "";
                            size_t pos = std::string(filename).find_last_of("\\/");
                            if (pos != std::string::npos)
                            {
                                processName = std::string(filename).substr(pos + 1);
                            }

                            processInfo.push_back(std::make_pair(processName, filename));
                        }
                        CloseHandle(hProcess);
                    }
                }
            }
        }
#elif defined(__linux__)
        // 使用 Linux 文件系统获取进程信息和文件地址
        DIR *dir;
        struct dirent *dirEntry;
        char exePath[PATH_MAX];

        dir = opendir("/proc");
        if (dir != NULL)
        {
            while ((dirEntry = readdir(dir)) != NULL)
            {
                if (dirEntry->d_type == DT_DIR && std::isdigit(dirEntry->d_name[0]))
                {
                    std::string pid = dirEntry->d_name;
                    std::string statPath = "/proc/" + pid + "/stat";
                    std::string exeLink = "/proc/" + pid + "/exe";
                    ssize_t bytes = readlink(exeLink.c_str(), exePath, sizeof(exePath) - 1);
                    if (bytes != -1)
                    {
                        exePath[bytes] = '\0';
                        FILE *statusFile = fopen(statPath.c_str(), "r");
                        if (statusFile != NULL)
                        {
                            char name[1024];
                            int result = fscanf(statusFile, "%*d %s", name);
                            if (result != 1)
                            {
                            }
                            fclose(statusFile);

                            std::string processName(name);
                            std::string filePath(exePath);

                            processInfo.push_back(std::make_pair(processName, filePath));
                        }
                    }
                }
            }
            closedir(dir);
        }
#elif defined(__APPLE__)
        // 使用 MacOS 文件系统获取进程信息和文件地址
        DIR *dir;
        struct dirent *dirEntry;
        char pidPath[PATH_MAX];

        dir = opendir("/proc");
        if (dir != NULL)
        {
            while ((dirEntry = readdir(dir)) != NULL)
            {
                if (dirEntry->d_type == DT_DIR && std::isdigit(dirEntry->d_name[0]))
                {
                    std::string pid = dirEntry->d_name;
                    std::string execPath = "/proc/" + pid + "/path";
                    FILE *file = fopen(execPath.c_str(), "r");
                    if (file != NULL)
                    {
                        memset(pidPath, 0, sizeof(pidPath));
                        fgets(pidPath, sizeof(pidPath) - 1, file);
                        fclose(file);

                        std::string processName = "";
                        size_t pos = std::string(pidPath).find_last_of("/");
                        if (pos != std::string::npos)
                        {
                            processName = std::string(pidPath).substr(pos + 1);
                        }
                        std::string filePath(pidPath);

                        processInfo.push_back(std::make_pair(processName, filePath));
                    }
                }
            }
            closedir(dir);
        }
#endif

        return processInfo;
    }

    void CheckDuplicateProcess(const std::string &program_name)
    {
#ifdef _WIN32 // Windows平台
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
        {
            // // spdlog::error("CreateToolhelp32Snapshot failed: {}", GetLastError());
            exit(EXIT_FAILURE);
        }

        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        BOOL bRet = Process32First(hSnapshot, &pe);
        while (bRet)
        {
            std::string name = pe.szExeFile;
            if (name == program_name)
            {
                // spdlog::warn("Found duplicate {} process with PID {}", program_name, pe.th32ProcessID);
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess == NULL)
                {
                    // // spdlog::error("OpenProcess failed: {}", GetLastError());
                    exit(EXIT_FAILURE);
                }
                if (!TerminateProcess(hProcess, 0))
                {
                    // // spdlog::error("TerminateProcess failed: {}", GetLastError());
                    exit(EXIT_FAILURE);
                }
                CloseHandle(hProcess);
                break;
            }
            bRet = Process32Next(hSnapshot, &pe);
        }
        CloseHandle(hSnapshot);
#else // Linux和macOS平台
        DIR *dirp = opendir("/proc");
        if (dirp == NULL)
        {
            // // spdlog::error("Cannot open /proc directory");
            exit(EXIT_FAILURE);
        }

        std::vector<pid_t> pids;
        struct dirent *dp;
        while ((dp = readdir(dirp)) != NULL)
        {
            if (!isdigit(dp->d_name[0]))
            {
                continue;
            }
            pid_t pid = atoi(dp->d_name);
            char cmdline_file[256];
            snprintf(cmdline_file, sizeof(cmdline_file), "/proc/%d/cmdline", pid);

            FILE *cmd_file = fopen(cmdline_file, "r");
            if (cmd_file)
            {
                char cmdline[1024];
                if (fgets(cmdline, sizeof(cmdline), cmd_file) == NULL)
                {
                    // // spdlog::error("Failed to get pids");
                }
                fclose(cmd_file);
                std::string name = cmdline;
                if (name == program_name)
                {
                    pids.push_back(pid);
                }
            }
        }
        closedir(dirp);

        if (pids.size() <= 1)
        {
            // DLOG_F(INFO,"No duplicate {} process found", program_name);
            return;
        }

        for (auto pid : pids)
        {
            // spdlog::warn("Found duplicate {} process with PID {}", program_name, pid);
            if (kill(pid, SIGTERM) != 0)
            {
                // // spdlog::error("kill failed: {}", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
#endif
    }

}

/*
int main()
{
    float cpu_usage = cpustat();
    float mem_usage = memstat();
    float gpu_usage = gpustat();

    std::cout << "CPU Usage: " << cpu_usage << "%" << std::endl;
    std::cout << "Memory Usage: " << mem_usage << "%" << std::endl;
    std::cout << "GPU Usage: " << gpu_usage << "%" << std::endl;

    std::vector<std::pair<std::string, float>> disk_usage = diskstat();
    for (const auto &disk : disk_usage)
    {
        std::cout << "Disk " << disk.first << " Usage: " << disk.second << "%" << std::endl;
    }

    std::vector<std::string> net_connections = netstat();
    for (const auto &conn : net_connections)
    {
        std::cout << "Network Connection: " << conn << std::endl;
    }

    float cpuTemperature = GetCpuTemperature();

    std::cout << "CPU Temperature: " << cpuTemperature << "°C" << std::endl;

    bool isConnected = IsConnectedToInternet();
    if (isConnected) {
        std::cout << "Connected to the Internet." << std::endl;
    } else {
        std::cout << "Not connected to the Internet." << std::endl;
    }

    bool elevated = IsRoot();
    if (elevated) {
        std::cout << "Current process has elevated privileges." << std::endl;
    } else {
        std::cout << "Current process does not have elevated privileges." << std::endl;
    }

    std::vector<std::pair<std::string, std::string>> processInfo = GetProcessInfo();

    for (const auto &info : processInfo)
    {
        std::cout << "Process Name: " << info.first << std::endl;
        std::cout << "File Address: " << info.second << std::endl;
        std::cout << std::endl;
    }

    return 0;
}
*/