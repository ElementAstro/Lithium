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

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <tlhelp32.h>
// clang-format on
#elif defined(__linux__) || defined(__ANDROID__)
#include <dirent.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#elif defined(__APPLE__)
#include <libproc.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#else
#error "Unknown platform"
#endif

#include <algorithm>
#include <chrono>
#include <fstream>
#include <mutex>
#include <optional>
#include <sstream>
#include <thread>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/utils/convert.hpp"
#include "atom/utils/string.hpp"

namespace atom::system {
ProcessManager::ProcessManager(int maxProcess) : m_maxProcesses(maxProcess) {}

std::shared_ptr<ProcessManager> ProcessManager::createShared(int maxProcess) {
    return std::make_shared<ProcessManager>(maxProcess);
}

bool ProcessManager::createProcess(const std::string &command,
                                   const std::string &identifier) {
    pid_t pid;

#ifdef _WIN32
    STARTUPINFOW si{};
    PROCESS_INFORMATION pi{};
    std::wstring wideCommand = L"powershell.exe -Command \"" +
                               std::wstring(command.begin(), command.end()) +
                               L"\"";
    si.cb = sizeof(si);
    if (!CreateProcessW(NULL, &wideCommand[0], NULL, NULL, FALSE, 0, NULL, NULL,
                        &si, &pi)) {
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

    std::unique_lock lock(mtx);
    Process process;
    process.pid = pid;
    process.name = identifier;
    processes.push_back(process);
    DLOG_F(INFO, "Process created: {} (PID: {})", identifier, pid);
    return true;
}

bool ProcessManager::hasProcess(const std::string &identifier) {
    std::shared_lock lock(mtx);
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

    try {
#ifdef _WIN32
        std::string cmd = "powershell.exe -Command \"" + script + "\"";
        auto [output, status] = executeCommandWithStatus(cmd);
        if (status != 0) {
            LOG_F(ERROR, "Failed to create process");
            return false;
        }
        // On Windows, we don't get the PID of the process directly
        // Assuming the command was successful, we set pid as 0 for now
        pid = 0;
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

        std::unique_lock lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
        processes.push_back(process);
        DLOG_F(INFO, "Process created: {} (PID: {})", identifier, pid);
        return true;
    } catch (const std::runtime_error &e) {
        LOG_F(ERROR, "Exception occurred: {}", e.what());
        return false;
    }
}

bool ProcessManager::terminateProcess(int pid, int signal) {
    std::unique_lock lock(mtx);
    pid = static_cast<pid_t>(pid);
    auto it = std::find_if(processes.begin(), processes.end(),
                           [pid](const Process &p) { return p.pid == pid; });

    if (it != processes.end()) {
        try {
            killProcessByPID(pid, signal);
        } catch (const atom::error::SystemCollapse &e) {
            LOG_F(ERROR, "System collapse occurred: {}", e.what());
            return false;
        }
        DLOG_F(INFO, "Process terminated: {} (PID: {})", it->name, pid);
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

std::vector<Process> ProcessManager::getRunningProcesses() const {
    std::shared_lock lock(mtx);
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

#ifdef _WIN32

std::vector<std::pair<int, std::string>> getAllProcesses() {
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
            processes.emplace_back(
                pid, atom::utils::WCharArrayToString(processEntry.szExeFile));
        } while (Process32Next(snapshot, &processEntry));
    }

    CloseHandle(snapshot);
    return processes;
}

#elif defined(__linux__)
std::optional<std::string> GetProcessName(int pid) {
    std::string path = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream commFile(path);
    if (commFile) {
        std::string name;
        std::getline(commFile, name);
        return name;
    }
    return std::nullopt;
}

std::vector<std::pair<int, std::string>> getAllProcesses() {
    std::vector<std::pair<int, std::string>> processes;

    DIR *procDir = opendir("/proc");
    if (!procDir) {
        LOG_F(ERROR, "Failed to open /proc directory");
        return processes;
    }

    struct dirent *entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            char *end;
            long pid = strtol(entry->d_name, &end, 10);
            if (*end == '\0') {
                if (auto name = GetProcessName(pid)) {
                    processes.emplace_back(pid, *name);
                }
            }
        }
    }

    closedir(procDir);
    return processes;
}

#elif defined(__APPLE__)
std::optional<std::string> GetProcessName(int pid) {
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) > 0) {
        std::string path(pathbuf);
        size_t slashPos = path.rfind('/');
        if (slashPos != std::string::npos) {
            return path.substr(slashPos + 1);
        }
        return path;
    }
    return std::nullopt;
}

std::vector<std::pair<int, std::string>> getAllProcesses() {
    std::vector<std::pair<int, std::string>> processes;

    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t length = 0;

    if (sysctl(mib, 4, nullptr, &length, nullptr, 0) == -1) {
        LOG_F(ERROR, "Failed to get process info length");
        return processes;
    }

    auto procBuf = std::make_unique<kinfo_proc[]>(length / sizeof(kinfo_proc));
    if (sysctl(mib, 4, procBuf.get(), &length, nullptr, 0) == -1) {
        LOG_F(ERROR, "Failed to get process info");
        return processes;
    }

    int procCount = length / sizeof(kinfo_proc);
    for (int i = 0; i < procCount; ++i) {
        int pid = procBuf[i].kp_proc.p_pid;
        if (auto name = GetProcessName(pid)) {
            processes.emplace_back(pid, *name);
        }
    }

    return processes;
}
#else
#error "Unsupported operating system"
#endif

std::string getLatestLogFile(const std::string &folderPath) {
    std::vector<fs::path> logFiles;

    for (const auto &entry : fs::directory_iterator(folderPath)) {
        const auto &path = entry.path();
        if (path.extension() == ".log") {
            logFiles.push_back(path);
        }
    }

    if (logFiles.empty()) {
        return "";
    }

    auto latestFile = std::max_element(
        logFiles.begin(), logFiles.end(),
        [](const fs::path &a, const fs::path &b) {
            return fs::last_write_time(a) < fs::last_write_time(b);
        });

    return latestFile->string();
}

Process getSelfProcessInfo() {
    Process info;

    // 获取进程ID
#ifdef _WIN32
    info.pid = GetCurrentProcessId();
#else
    info.pid = getpid();
#endif

    // 获取进程位置
#ifdef _WIN32
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
#else
    char path[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
    }
#endif
    info.path = path;

    // 获取进程名称
    info.name = info.path.filename().string();

    // 获取进程状态
    info.status = "Unknown";
#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, info.pid);
    if (hProcess) {
        DWORD exitCode;
        if (GetExitCodeProcess(hProcess, &exitCode) &&
            exitCode == STILL_ACTIVE) {
            info.status = "Running";
        }
        CloseHandle(hProcess);
    }
#else
    if (fs::exists(info.path)) {
        info.status = "Running";
    }
#endif

    auto outputPath = getLatestLogFile("./log");
    if (!outputPath.empty()) {
        std::ifstream outputFile(outputPath);
        info.output = std::string(std::istreambuf_iterator<char>(outputFile),
                                  std::istreambuf_iterator<char>());
    }

    return info;
}

std::string ctermid() {
#ifdef _WIN32
    // Windows平台
    const int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    DWORD length = GetConsoleTitleA(buffer, BUFFER_SIZE);
    if (length > 0) {
        return std::string(buffer, length);
    }
#else
    // 类Unix系统
    char buffer[L_ctermid];
    if (::ctermid(buffer) != nullptr) {
        return buffer;
    }
#endif
    return "";
}

#ifdef _WIN32

std::optional<int> getProcessPriorityByPid(int pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) {
        return std::nullopt;
    }
    int priority = GetPriorityClass(hProcess);
    CloseHandle(hProcess);
    return priority;
}

std::optional<int> getProcessPriorityByName(const std::string &name) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return std::nullopt;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry)) {
        do {
            if (name == atom::utils::WCharArrayToString(entry.szExeFile)) {
                CloseHandle(snapshot);
                return getProcessPriorityByPid(entry.th32ProcessID);
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    return std::nullopt;
}

#elif defined(__linux__) || defined(__ANDROID__)

std::optional<int> getProcessPriorityByPid(int pid) {
    int priority = getpriority(PRIO_PROCESS, pid);
    if (priority == -1 && errno != 0) {
        return std::nullopt;
    }
    return priority;
}

std::optional<int> getProcessPriorityByName(const std::string &name) {
    DIR *procDir = opendir("/proc");
    if (!procDir) {
        return std::nullopt;
    }

    struct dirent *entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            int pid = atoi(entry->d_name);
            if (pid > 0) {
                std::ifstream cmdline("/proc/" + std::to_string(pid) +
                                      "/cmdline");
                std::string cmd;
                std::getline(cmdline, cmd, '\0');
                if (cmd.find(name) != std::string::npos) {
                    closedir(procDir);
                    return getProcessPriorityByPid(pid);
                }
            }
        }
    }

    closedir(procDir);
    return std::nullopt;
}

#elif defined(__APPLE__)

std::optional<int> getProcessPriorityByPid(int pid) {
    int priority = getpriority(PRIO_PROCESS, pid);
    if (priority == -1 && errno != 0) {
        return std::nullopt;
    }
    return priority;
}

std::optional<int> getProcessPriorityByName(const std::string &name) {
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    size_t length;

    if (sysctl(mib, 4, nullptr, &length, nullptr, 0) == -1) {
        return std::nullopt;
    }

    std::vector<kinfo_proc> procs(length / sizeof(kinfo_proc));
    if (sysctl(mib, 4, procs.data(), &length, nullptr, 0) == -1) {
        return std::nullopt;
    }

    for (const auto &proc : procs) {
        if (name == proc.kp_proc.p_comm) {
            return getProcessPriorityByPid(proc.kp_proc.p_pid);
        }
    }

    return std::nullopt;
}

#endif

bool isProcessRunning(const std::string &processName) {
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        return false;
    }

    bool isRunning = false;
    do {
        if (processName == atom::utils::WCharArrayToString(pe32.szExeFile)) {
            isRunning = true;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return isRunning;
#else
    std::filesystem::path procDir("/proc");
    if (!std::filesystem::exists(procDir) ||
        !std::filesystem::is_directory(procDir))
        return false;

    for (auto &p : std::filesystem::directory_iterator(procDir)) {
        if (p.is_directory()) {
            std::string pid = p.path().filename().string();
            if (std::all_of(pid.begin(), pid.end(), isdigit)) {
                std::ifstream cmdline(p.path() / "cmdline");
                std::string cmd;
                std::getline(cmdline, cmd);
                if (cmd.find(processName) != std::string::npos) {
                    return true;
                }
            }
        }
    }
    return false;
#endif
}

int getParentProcessId(int processId) {
#ifdef _WIN32
    DWORD parentProcessId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &processEntry)) {
            do {
                if (static_cast<int>(processEntry.th32ProcessID) == processId) {
                    parentProcessId = processEntry.th32ParentProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &processEntry));
        }

        CloseHandle(hSnapshot);
    }
    return static_cast<int>(parentProcessId);
#else
    std::string path = "/proc/" + std::to_string(processId) + "/stat";
    std::ifstream file(path);
    std::string line;
    if (std::getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) {
            tokens.push_back(token);
        }
        if (tokens.size() > 3) {
            return std::stoul(tokens[3]);  // PPID is at the 4th position
        }
    }
    return 0;
#endif
}

bool _CreateProcessAsUser(const std::string &command,
                          const std::string &username,
                          const std::string &domain,
                          const std::string &password) {
#ifdef _WIN32
    HANDLE hToken = NULL;
    HANDLE hNewToken = NULL;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    bool result = false;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    if (!LogonUserA(atom::utils::StringToLPSTR(username),
                    atom::utils::StringToLPSTR(domain),
                    atom::utils::StringToLPSTR(password),
                    LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
                    &hToken)) {
        LOG_F(ERROR, "LogonUser failed with error: {}", GetLastError());
        goto Cleanup;
    }

    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation,
                          TokenPrimary, &hNewToken)) {
        LOG_F(ERROR, "DuplicateTokenEx failed with error: {}", GetLastError());
        goto Cleanup;
    }

    if (!CreateProcessAsUserW(hNewToken, NULL,
                              atom::utils::StringToLPWSTR(command), NULL, NULL,
                              FALSE, 0, NULL, NULL, &si, &pi)) {
        LOG_F(ERROR, "CreateProcessAsUser failed with error: {}",
              GetLastError());
        goto Cleanup;
    }
    LOG_F(INFO, "Process created successfully!");
    result = true;
    WaitForSingleObject(pi.hProcess, INFINITE);

Cleanup:
    if (hToken) {
        CloseHandle(hToken);
    }
    if (hNewToken) {
        CloseHandle(hNewToken);
    }
    if (pi.hProcess) {
        CloseHandle(pi.hProcess);
    }
    if (pi.hThread) {
        CloseHandle(pi.hThread);
    }

    return result;
#else
    pid_t pid = fork();
    if (pid < 0) {
        LOG_F(ERROR, "Fork failed");
        return false;
    } else if (pid == 0) {
        struct passwd *pw = getpwnam(username.c_str());
        if (pw == NULL) {
            LOG_F(ERROR, "Failed to get user information for {}", username);
            exit(EXIT_FAILURE);
        }

        if (setgid(pw->pw_gid) != 0) {
            LOG_F(ERROR, "Failed to set group ID");
            exit(EXIT_FAILURE);
        }
        if (setuid(pw->pw_uid) != 0) {
            LOG_F(ERROR, "Failed to set user ID");
            exit(EXIT_FAILURE);
        }

        execl("/bin/sh", "sh", "-c", command.c_str(), (char *)0);
        LOG_F(ERROR, "Failed to execute command");
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            LOG_F("INFO", "Process exited with status {}",
                  std::to_string(WEXITSTATUS(status)));
            return WEXITSTATUS(status) == 0;
        } else {
            LOG_F(ERROR, "Process did not exit normally");
            return false;
        }
    }
#endif
}

}  // namespace atom::system
