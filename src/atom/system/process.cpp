/*
 * process.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-19

Description: Process Manager

**************************************************/

#include "process.hpp"
//#include "config.h"

#if defined(_WIN32)
#include <tlhelp32.h>
#include <windows.h>
#elif defined(__linux__)
#include <dirent.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#elif defined(__APPLE__)
#include <libproc.h>
#include <sys/sysctl.h>
#else
#error "Unknown platform"
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
ProcessManager::ProcessManager() { m_maxProcesses = 10; }

ProcessManager::ProcessManager(int maxProcess) : m_maxProcesses(maxProcess) {}

std::shared_ptr<ProcessManager> ProcessManager::createShared() {
    return std::make_shared<ProcessManager>();
}

std::shared_ptr<ProcessManager> ProcessManager::createShared(int maxProcess) {
    return std::make_shared<ProcessManager>(maxProcess);
}

bool ProcessManager::createProcess(const std::string &command,
                                   const std::string &identifier) {
    pid_t pid;

#ifdef _WIN32
    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};
    std::string cmd = "powershell.exe -Command \"" + command + "\"";
    if (!CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL,
                       NULL, &si, &pi)) {
        LOG_F(ERROR, "Failed to create PowerShell process");
        return false;
    }
    pid = pi.dwProcessId;
#else
    pid = fork();

    if (pid == 0) {
        // Child process code
        DLOG_F(INFO, _("Running command: {}"), command);
        int pipefd[2];
        int result = pipe(pipefd);
        if (result != 0) {
        }
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        execlp(command.c_str(), command.c_str(), NULL);
        exit(0);
    } else if (pid < 0) {
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
    DLOG_F(INFO, "Process created: {} (PID: {})", identifier, pid);
    return true;
}

bool ProcessManager::hasProcess(const std::string &identifier) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto &process : processes) {
        if (process.name == identifier) {
            return true;
        }
    }
    return false;
}

bool ProcessManager::runScript(const std::string &script,
                               const std::string &identifier) {
    pid_t pid;

#ifdef _WIN32
    std::string cmd = "powershell.exe -Command \"" + script + "\"";
    STARTUPINFO si{};
    PROCESS_INFORMATION pi{};
    if (!CreateProcess(NULL, (LPSTR)cmd.c_str(), NULL, NULL, FALSE, 0, NULL,
                       NULL, &si, &pi)) {
        LOG_F(ERROR, "Failed to create process");
        return false;
    }
    pid = pi.dwProcessId;
#else
    pid = fork();

    if (pid == 0) {
        // Child process code
        DLOG_F(INFO, _("Running script: {}"), script);

#ifdef __APPLE__
        execl("/bin/sh", "sh", "-c", script.c_str(), NULL);
#else
        execl("/bin/bash", "bash", "-c", script.c_str(), NULL);
#endif
    } else if (pid < 0) {
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
    DLOG_F(INFO, "Process created: {} (PID: {})", identifier, pid);
    return true;
}

bool ProcessManager::terminateProcess(pid_t pid, int signal) {
    auto it = std::find_if(processes.begin(), processes.end(),
                           [pid](const Process &p) { return p.pid == pid; });

    if (it != processes.end()) {
#ifdef _WIN32
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
        if (hProcess != NULL) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
            DLOG_F(INFO, "Process terminated: {} (PID: {})", it->name, pid);
        } else {
            LOG_F(ERROR, "Failed to terminate process");
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
    } else {
        LOG_F(ERROR, "Process not found");
        return false;
    }
    return true;
}

bool ProcessManager::terminateProcessByName(const std::string &name,
                                            int signal) {
    auto it =
        std::find_if(processes.begin(), processes.end(),
                     [&name](const Process &p) { return p.name == name; });

    if (it != processes.end()) {
        return terminateProcess(it->pid, signal);
    }
    LOG_F(ERROR, "Process not found by name: {}", name);
    return false;
}

std::vector<Process> ProcessManager::getRunningProcesses() {
    std::lock_guard<std::mutex> lock(mtx);
    return processes;
}

std::vector<std::string> ProcessManager::getProcessOutput(
    const std::string &identifier) {
    auto it = std::find_if(
        processes.begin(), processes.end(),
        [&identifier](const Process &p) { return p.name == identifier; });

    if (it != processes.end()) {
        std::vector<std::string> outputLines;
        std::stringstream ss(it->output);
        std::string line;

        while (getline(ss, line)) {
            outputLines.push_back(line);
        }

        return outputLines;
    } else {
        LOG_F(ERROR, "Process not found");
        return std::vector<std::string>();
    }
}

void ProcessManager::waitForCompletion() {
    for (const auto &process : processes) {
#ifdef _WIN32
        HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, process.pid);
        if (hProcess != NULL) {
            WaitForSingleObject(hProcess, INFINITE);
            CloseHandle(hProcess);
            DLOG_F(INFO, "Process completed: {} (PID: {})", process.name,
                   process.pid);
        } else {
            LOG_F(ERROR, "Failed to wait for process completion");
        }
#else
        int status;
        waitpid(process.pid, &status, 0);

        DLOG_F(INFO, _("Process completed: %s (PID: %d)"), process.name.c_str(),
               process.pid);
#endif
    }

    processes.clear();
    DLOG_F(INFO, "All processes completed.");
}

#if defined(_WIN32)
std::vector<std::pair<int, std::string>> GetAllProcesses() {
    std::vector<std::pair<int, std::string>> processes;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to create process snapshot");
        return processes;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(processEntry);

    if (Process32First(snapshot, &processEntry)) {
        do {
            int pid = processEntry.th32ProcessID;
            std::string name = processEntry.szExeFile;
            processes.push_back(std::make_pair(pid, name));
        } while (Process32Next(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return processes;
}
#elif defined(__linux__)
std::string GetProcessName(int pid) {
    std::string name;
    std::string path = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream commFile(path);
    if (commFile) {
        std::getline(commFile, name);
    }
    commFile.close();
    return name;
}

std::vector<std::pair<int, std::string>> GetAllProcesses() {
    std::vector<std::pair<int, std::string>> processes;

    DIR *procDir = opendir("/proc");
    if (!procDir) {
        LOG_F(ERROR, "Failed to open /proc directory");
        return processes;
    }

    dirent *entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            char *end;
            long pid = strtol(entry->d_name, &end, 10);
            if (*end == '\0') {
                std::string name = GetProcessName(pid);
                processes.push_back(std::make_pair(pid, name));
            }
        }
    }

    closedir(procDir);
    return processes;
}

#elif defined(__APPLE__)
std::string GetProcessName(int pid) {
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) <= 0) {
        LOG_F(ERROR, _("Failed to get process path"));
        return "";
    }
    std::string path(pathbuf);
    size_t slashPos = path.rfind('/');
    if (slashPos != std::string::npos) {
        return path.substr(slashPos + 1);
    }
    return path;
}

std::vector<std::pair<int, std::string>> GetAllProcesses() {
    std::vector<std::pair<int, std::string>> processes;

    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t length = 0;

    if (sysctl(mib, 4, nullptr, &length, nullptr, 0) == -1) {
        LOG_F(ERROR, _("Failed to get process info length"));
        return processes;
    }

    struct kinfo_proc *procBuf = (struct kinfo_proc *)malloc(length);
    if (!procBuf) {
        LOG_F(ERROR, _("Failed to allocate memory"));
        return processes;
    }

    if (sysctl(mib, 4, procBuf, &length, nullptr, 0) == -1) {
        LOG_F(ERROR, _("Failed to get process info"));
        free(procBuf);
        return processes;
    }

    int procCount = length / sizeof(struct kinfo_proc);
    for (int i = 0; i < procCount; ++i) {
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

Process GetSelfProcessInfo() {
    Process info;

    // 获取进程ID
#ifdef _WIN32
    DWORD pid = GetCurrentProcessId();
#else
    pid_t pid = getpid();
#endif
    info.pid = pid;

    // 获取进程位置
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileName(NULL, path, MAX_PATH);
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
    }
#endif
    info.path = path;

    // 获取进程状态
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess) {
        DWORD exitCode;
        if (GetExitCodeProcess(hProcess, &exitCode)) {
            info.status = "Running";
        } else {
            info.status = "Unknown";
        }
        CloseHandle(hProcess);
    } else {
        info.status = "Unknown";
    }
#else
    struct stat statBuf;
    if (stat(path, &statBuf) == 0) {
        info.status = "Running";
    } else {
        info.status = "Unknown";
    }
#endif

    return info;
}

}  // namespace atom::system
