/*
 * system.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: System

**************************************************/

#include "system.hpp"

#include <cstdlib>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
namespace fs = std::filesystem;

#ifdef _WIN32
#include <Windows.h>
#include <tlhelp32.h>
#include <pdh.h>
#include <Psapi.h>
#include <iphlpapi.h>
#include <intrin.h>
#include <wincon.h>
#pragma comment(lib, "user32.lib")
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

#include "atom/log/loguru.hpp"
#include "atom/error/exception.hpp"

namespace Atom::System
{
    bool CheckSoftwareInstalled(const std::string &software_name)
    {
        bool is_installed = false;
#ifdef _WIN32
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
                        if (software_name == displayName)
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

    bool checkExecutableFile(const std::string &fileName, const std::string &fileExt)
    {
#ifdef _WIN32
        fs::path filePath = fileName + fileExt;
#else
        fs::path filePath = fileName;
#endif

        DLOG_F(INFO, "Checking file '%s'.", filePath.string().c_str());

        if (!fs::exists(filePath))
        {
            DLOG_F(WARNING, "The file '%s' does not exist.", filePath.string().c_str());
            return false;
        }

#ifdef _WIN32
        if (!fs::is_regular_file(filePath) || !(GetFileAttributesA(filePath.generic_string().c_str()) & FILE_ATTRIBUTE_DIRECTORY))
        {
            DLOG_F(WARNING, "The file '%s' is not a regular file or is not executable.", filePath.string().c_str());
            return false;
        }
#else
        if (!fs::is_regular_file(filePath) || access(filePath.c_str(), X_OK) != 0)
        {
            DLOG_F(WARNING, "The file '%s' is not a regular file or is not executable.", filePath.string().c_str());
            return false;
        }
#endif

        DLOG_F(INFO, "The file '%s' exists and is executable.", filePath.string().c_str());
        return true;
    }    

    bool Shutdown()
    {
#ifdef _WIN32
        ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
#else
        int ret = system("shutdown -h now");
        if  (ret == 0 || ret == 1 /* success */)
        {
            return true;
        }
#endif
        return true;
    }

    // 重启函数
    bool Reboot()
    {
#ifdef _WIN32
        ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
#else
        int ret = system("reboot");
        if  (ret == 0 || ret == 1 /* success */)
        {
            return true;
        }
#endif
        return true;
    }

    bool IsRoot()
    {
#ifdef _WIN32
        HANDLE hToken;
        TOKEN_ELEVATION elevation;
        DWORD dwSize;

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        {
            LOG_F(ERROR, "IsRoot error: OpenProcessToken error");
            return false;
        }

        if (!GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize))
        {
            LOG_F(ERROR, "IsRoot error: GetTokenInformation error");
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

    std::string GetCurrentUsername()
    {
#ifdef _WIN32
        char username[UNLEN + 1];
        DWORD usernameLen = UNLEN + 1;
        if (GetUserNameA(username, &usernameLen))
        {
            return std::string(username);
        }
#else
        char username[256];
        if (getlogin_r(username, sizeof(username)) == 0)
        {
            return std::string(username);
        }
#endif
        return "";
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

    bool CheckDuplicateProcess(const std::string &program_name)
    {
#ifdef _WIN32
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
        {
            LOG_F(ERROR, "CreateToolhelp32Snapshot failed: {}", GetLastError());
            return false;
        }

        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        BOOL bRet = Process32First(hSnapshot, &pe);
        while (bRet)
        {
            std::string name = pe.szExeFile;
            if (name == program_name)
            {
                LOG_F(WARNING, "Found duplicate {} process with PID {}", program_name, pe.th32ProcessID);
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                if (hProcess == NULL)
                {
                    LOG_F(ERROR, "OpenProcess failed: {}", GetLastError());
                    return false;
                }
                if (!TerminateProcess(hProcess, 0))
                {
                    LOG_F(ERROR, "TerminateProcess failed: {}", GetLastError());
                    return false;
                }
                CloseHandle(hProcess);
                break;
            }
            bRet = Process32Next(hSnapshot, &pe);
        }
        CloseHandle(hSnapshot);
#else
        DIR *dirp = opendir("/proc");
        if (dirp == NULL)
        {
            LOG_F(ERROR, "Cannot open /proc directory");
            return false;
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
                    LOG_F(ERROR, "Failed to get pids");
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
            DLOG_F(INFO, "No duplicate {} process found", program_name);
            return true;
        }

        for (auto pid : pids)
        {
            LOG_F(WARNING, "Found duplicate {} process with PID {}", program_name, pid);
            if (kill(pid, SIGTERM) != 0)
            {
                LOG_F(ERROR, "kill failed: {}", strerror(errno));
                return false;
            }
        }
#endif
        return true;
    }

    bool isProcessRunning(const std::string &processName)
    {
#ifdef _WIN32
        // Enumerate all processes in the system and find the specified process
        // 枚举系统中所有进程，查找指定名称的进程
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE)
        {
            return false;
        }
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(pe32);
        // Get the first process
        // 获取第一个进程
        if (!Process32First(hSnapshot, &pe32))
        {
            CloseHandle(hSnapshot);
            return false;
        }
        bool isRunning = false;
        do
        {
            if (processName.compare(pe32.szExeFile) == 0)
            {
                isRunning = true;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));

        CloseHandle(hSnapshot);
        return isRunning;
#else
        // Check /proc directory for the existence of the process directory
        // 检查 /proc 目录下是否存在指定名称的进程目录
        DIR *dir;
        struct dirent *ent;
        char processDirName[256];
        sprintf(processDirName, "/proc/%s", processName.c_str());
        if ((dir = opendir(processDirName)) == NULL)
        {
            return false;
        }

        closedir(dir);
        return true;
        // An alternative way to check
        /*
        std::string command = "pgrep -c " + processName;
        std::string output;
        std::ifstream pipe(command.c_str());
        if (pipe)
        {
            if (getline(pipe, output))
            {
                int count = std::stoi(output);
                return (count > 0);
            }
        }

        return false;
        */
#endif
    }

    std::vector<ProcessInfo> GetProcessDetails()
    {
        std::vector<ProcessInfo> processList;
#ifdef _WIN32
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);

            if (Process32First(hSnapshot, &pe32))
            {
                do
                {
                    ProcessInfo processInfo;
                    processInfo.processID = pe32.th32ProcessID;
                    processInfo.parentProcessID = pe32.th32ParentProcessID;
                    processInfo.basePriority = pe32.pcPriClassBase;
                    processInfo.executableFile = pe32.szExeFile;

                    processList.push_back(processInfo);
                } while (Process32Next(hSnapshot, &pe32));
            }

            CloseHandle(hSnapshot);
        }
#else
        DIR *dir = opendir("/proc");
        if (dir != NULL)
        {
            struct dirent *ent;
            while ((ent = readdir(dir)) != NULL)
            {
                std::string processDir = ent->d_name;
                if (processDir != "." && processDir != ".." && std::isdigit(processDir[0]))
                {
                    std::string exePath = "/proc/" + processDir + "/exe";
                    std::string cmdlinePath = "/proc/" + processDir + "/cmdline";
                    std::string statPath = "/proc/" + processDir + "/stat";

                    std::ifstream exeFile(exePath);
                    std::ifstream cmdlineFile(cmdlinePath);
                    std::ifstream statFile(statPath);

                    std::string exeName, cmdline, stat;

                    if (exeFile.is_open())
                    {
                        std::getline(exeFile, exeName);
                        exeFile.close();
                    }

                    if (cmdlineFile.is_open())
                    {
                        std::getline(cmdlineFile, cmdline);
                        cmdlineFile.close();
                    }

                    if (statFile.is_open())
                    {
                        std::getline(statFile, stat);
                        statFile.close();
                    }

                    std::istringstream iss(stat);
                    int pid, ppid, priority;

                    std::string temp;
                    for (int i = 0; i < 3; ++i)
                        iss >> temp;

                    iss >> pid >> temp >> ppid >> priority;

                    ProcessInfo processInfo;
                    processInfo.processID = pid;
                    processInfo.parentProcessID = ppid;
                    processInfo.basePriority = priority;
                    processInfo.executableFile = exeName;

                    processList.push_back(processInfo);
                }
            }

            closedir(dir);
        }
#endif
        return processList;
    }

#ifdef _WIN32
    DWORD GetParentProcessId(DWORD processId)
    {
        DWORD parentProcessId = 0;
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32 processEntry;
            processEntry.dwSize = sizeof(PROCESSENTRY32);

            if (Process32First(hSnapshot, &processEntry))
            {
                do
                {
                    if (processEntry.th32ProcessID == processId)
                    {
                        parentProcessId = processEntry.th32ParentProcessID;
                        break;
                    }
                } while (Process32Next(hSnapshot, &processEntry));
            }

            CloseHandle(hSnapshot);
        }
        return parentProcessId;
    }

    bool GetProcessInfoByID(DWORD processID, ProcessInfo &processInfo)
    {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
        if (hProcess != NULL)
        {
            char exeName[MAX_PATH];
            DWORD exeNameLength = MAX_PATH;

            if (QueryFullProcessImageNameA(hProcess, 0, exeName, &exeNameLength))
            {
                processInfo.processID = processID;
                processInfo.executableFile = std::string(exeName);
                // if (!GetProcessId(reinterpret_cast<HANDLE>(GetParentProcessId(reinterpret_cast<DWORD>(hProcess))), &processInfo.parentProcessID))
                processInfo.parentProcessID = 0;
                processInfo.basePriority = GetPriorityClass(hProcess);

                CloseHandle(hProcess);
                return true;
            }

            CloseHandle(hProcess);
        }

        return false;
    }

    bool GetProcessInfoByName(const std::string &processName, ProcessInfo &processInfo)
    {
        std::vector<ProcessInfo> processes = GetProcessDetails();
        for (const auto &process : processes)
        {
            if (process.executableFile == processName)
            {
                if (GetProcessInfoByID(process.processID, processInfo))
                    return true;
            }
        }

        return false;
    }
#else
    bool GetProcessInfoByID(int processID, ProcessInfo &processInfo)
    {

        std::string statPath = "/proc/" + std::to_string(processID) + "/stat";

        std::ifstream statFile(statPath);
        std::string stat;

        if (statFile.is_open())
        {
            std::getline(statFile, stat);
            statFile.close();

            std::istringstream iss(stat);
            int pid, ppid, priority;

            std::string temp;
            for (int i = 0; i < 3; ++i)
                iss >> temp;

            iss >> pid >> temp >> ppid >> priority;

            processInfo.processID = pid;
            processInfo.parentProcessID = ppid;
            processInfo.basePriority = priority;

            std::string exePath = "/proc/" + std::to_string(processID) + "/exe";
            char exeName[PATH_MAX];
            if (realpath(exePath.c_str(), exeName) != NULL)
            {
                processInfo.executableFile = std::string(exeName);
                return true;
            }
        }

        return false;
    }

    bool GetProcessInfoByName(const std::string &processName, ProcessInfo &processInfo)
    {
        std::vector<ProcessInfo> processes = GetProcessDetails();
        for (const auto &process : processes)
        {
            size_t pos = process.executableFile.rfind('/');
            if (pos != std::string::npos)
            {
                std::string fileName = process.executableFile.substr(pos + 1);
                if (fileName == processName)
                {
                    if (GetProcessInfoByID(process.processID, processInfo))
                        return true;
                }
            }
        }

        return false;
    }
#endif
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