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

#include <algorithm>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <sstream>

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <tlhelp32.h>
#include <iprtrmib.h>
#include <iphlpapi.h>
// clang-format on
#elif defined(__linux__) || defined(__ANDROID__)
#include <dirent.h>
#include <pwd.h>
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

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/utils/convert.hpp"
#include "atom/utils/string.hpp"

namespace atom::system {
class ProcessManager::ProcessManagerImpl {
public:
    int m_maxProcesses;
    std::condition_variable cv;
    std::vector<Process> processes;
    mutable std::shared_timed_mutex mtx;

    ProcessManagerImpl(int maxProcess) : m_maxProcesses(maxProcess) {}

    ~ProcessManagerImpl() {
        // Ensure all processes are cleaned up
        waitForCompletion();
    }

    auto createProcess(const std::string &command,
                       const std::string &identifier) -> bool {
        pid_t pid;

#ifdef _WIN32
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Convert command to wide string
        std::wstring wcommand(command.begin(), command.end());

        // Start the child process.
        // TODO: Use CreateProcessW instead of CreateProcessA, but some programs
        // occured
        /*
        if (CreateProcessW(wcommand.c_str(),  // Command line
                NULL,          // 命令行参数，可以传 NULL
                NULL,          // 进程安全属性
                NULL,          // 线程安全属性
                FALSE,         // 不继承句柄
                0,             // 创建标志
                NULL,          // 使用父进程的环境
                NULL,          // 使用父进程的当前目录
                &si,  // 启动信息
                &si   // 进程信息
            == 0)) {
            return false;
        }
        */

        pid = pi.dwProcessId;
#else
        pid = fork();
        if (pid == 0) {
            // Child process code
            execlp(command.c_str(), command.c_str(), nullptr);
            exit(0);
        } else if (pid < 0) {
            return false;
        }
#endif
        std::unique_lock lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
#ifdef _WIN32
        process.handle = pi.hProcess;
#endif
        processes.push_back(process);
        return true;
    }

    auto terminateProcess(int pid, int signal) -> bool {
        std::unique_lock lock(mtx);
        auto it =
            std::find_if(processes.begin(), processes.end(),
                         [pid](const Process &p) { return p.pid == pid; });

        if (it != processes.end()) {
#ifdef _WIN32
            // Windows-specific logic to terminate the process
            if (!TerminateProcess(it->handle, signal)) {
                return false;
            }
            CloseHandle(it->handle);
#else
            kill(pid, signal);
#endif
            processes.erase(it);
            return true;
        }
        return false;
    }

    void waitForCompletion() {
        for (const auto &process : processes) {
#ifdef _WIN32
            // Windows-specific process waiting logic
            WaitForSingleObject(process.handle, INFINITE);
            CloseHandle(process.handle);
#else
            waitpid(process.pid, nullptr, 0);
#endif
        }
        processes.clear();
    }
};

ProcessManager::ProcessManager(int maxProcess)
    : impl(std::make_unique<ProcessManagerImpl>(maxProcess)) {}

ProcessManager::~ProcessManager() = default;

auto ProcessManager::createShared(int maxProcess)
    -> std::shared_ptr<ProcessManager> {
    return std::make_shared<ProcessManager>(maxProcess);
}

auto ProcessManager::createProcess(const std::string &command,
                                   const std::string &identifier) -> bool {
    return impl->createProcess(command, identifier);
}

auto ProcessManager::terminateProcess(int pid, int signal) -> bool {
    return impl->terminateProcess(pid, signal);
}

auto ProcessManager::hasProcess(const std::string &identifier) -> bool {
    std::shared_lock lock(impl->mtx);
    for (const auto &process : impl->processes) {
        if (process.name == identifier) {
            return true;
        }
    }
    return false;
}

void ProcessManager::waitForCompletion() { impl->waitForCompletion(); }

auto ProcessManager::getRunningProcesses() const -> std::vector<Process> {
    std::shared_lock lock(impl->mtx);
    return impl->processes;
}

auto ProcessManager::getProcessOutput(const std::string &identifier)
    -> std::vector<std::string> {
    auto it = std::find_if(
        impl->processes.begin(), impl->processes.end(),
        [&identifier](const Process &p) { return p.name == identifier; });

    if (it != impl->processes.end()) {
        std::vector<std::string> outputLines;
        std::stringstream sss(it->output);
        std::string line;

        while (getline(sss, line)) {
            outputLines.push_back(line);
        }

        return outputLines;
    }
    return {};
}

#ifdef _WIN32

auto getAllProcesses() -> std::vector<std::pair<int, std::string>> {
    std::vector<std::pair<int, std::string>> processes;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to create process snapshot");
        return processes;
    }

    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(processEntry);

    if (Process32First(snapshot, &processEntry) != 0) {
        do {
            int pid = processEntry.th32ProcessID;
            processes.emplace_back(pid, std::string(processEntry.szExeFile));
        } while (Process32Next(snapshot, &processEntry) != 0);
    }

    CloseHandle(snapshot);
    return processes;
}

#elif defined(__linux__)
auto GetProcessName(int pid) -> std::optional<std::string> {
    std::string path = "/proc/" + std::to_string(pid) + "/comm";
    std::ifstream commFile(path);
    if (commFile) {
        std::string name;
        std::getline(commFile, name);
        return name;
    }
    return std::nullopt;
}

auto getAllProcesses() -> std::vector<std::pair<int, std::string>> {
    std::vector<std::pair<int, std::string>> processes;

    DIR *procDir = opendir("/proc");
    if (procDir == nullptr) {
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

auto getLatestLogFile(const std::string &folderPath) -> std::string {
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

auto getSelfProcessInfo() -> Process {
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
    GetModuleFileNameW(nullptr, path, MAX_PATH);
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
    if (hProcess != nullptr) {
        DWORD exitCode;
        if ((GetExitCodeProcess(hProcess, &exitCode) != 0) &&
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

auto ctermid() -> std::string {
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

auto getProcessPriorityByPid(int pid) -> std::optional<int> {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == nullptr) {
        return std::nullopt;
    }
    int priority = GetPriorityClass(hProcess);
    CloseHandle(hProcess);
    return priority;
}

auto getProcessPriorityByName(const std::string &name) -> std::optional<int> {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return std::nullopt;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry) != 0) {
        do {
            if (name == std::string(entry.szExeFile)) {
                CloseHandle(snapshot);
                return getProcessPriorityByPid(entry.th32ProcessID);
            }
        } while (Process32Next(snapshot, &entry) != 0);
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

auto isProcessRunning(const std::string &processName) -> bool {
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(hSnapshot, &pe32) == 0) {
        CloseHandle(hSnapshot);
        return false;
    }

    bool isRunning = false;
    do {
        if (processName == std::string(pe32.szExeFile)) {
            isRunning = true;
            break;
        }
    } while (Process32Next(hSnapshot, &pe32) != 0);

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

auto getParentProcessId(int processId) -> int {
#ifdef _WIN32
    DWORD parentProcessId = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &processEntry) != 0) {
            do {
                if (static_cast<int>(processEntry.th32ProcessID) == processId) {
                    parentProcessId = processEntry.th32ParentProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &processEntry) != 0);
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

auto _CreateProcessAsUser(
    const std::string &command, const std::string &username,
    [[maybe_unused]] const std::string &domain,
    [[maybe_unused]] const std::string &password) -> bool {
#ifdef _WIN32
    HANDLE hToken = nullptr;
    HANDLE hNewToken = nullptr;
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    bool result = false;
    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
    if (LogonUserA(atom::utils::StringToLPSTR(username),
                   atom::utils::StringToLPSTR(domain),
                   atom::utils::StringToLPSTR(password),
                   LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
                   &hToken) == 0) {
        LOG_F(ERROR, "LogonUser failed with error: {}", GetLastError());
        goto Cleanup;
    }

    if (DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, nullptr,
                         SecurityImpersonation, TokenPrimary,
                         &hNewToken) == 0) {
        LOG_F(ERROR, "DuplicateTokenEx failed with error: {}", GetLastError());
        goto Cleanup;
    }

    if (CreateProcessAsUserW(
            hNewToken, nullptr, atom::utils::StringToLPWSTR(command), nullptr,
            nullptr, FALSE, 0, nullptr, nullptr, &si, &pi) == 0) {
        LOG_F(ERROR, "CreateProcessAsUser failed with error: {}",
              GetLastError());
        goto Cleanup;
    }
    LOG_F(INFO, "Process created successfully!");
    result = true;
    WaitForSingleObject(pi.hProcess, INFINITE);

Cleanup:
    if (hToken != nullptr) {
        CloseHandle(hToken);
    }
    if (hNewToken != nullptr) {
        CloseHandle(hNewToken);
    }
    if (pi.hProcess != nullptr) {
        CloseHandle(pi.hProcess);
    }
    if (pi.hThread != nullptr) {
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
        if (pw == nullptr) {
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
            LOG_F(INFO, "Process exited with status {}",
                  std::to_string(WEXITSTATUS(status)));
            return WEXITSTATUS(status) == 0;
        }
        LOG_F(ERROR, "Process did not exit normally");
        return false;
    }
#endif
}

#ifdef _WIN32
auto ProcessManager::getProcessHandle(int pid) const -> HANDLE {
    return OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
}
#else
auto ProcessManager::getProcFilePath(int pid,
                                     const std::string &file) -> std::string {
    return "/proc/" + std::to_string(pid) + "/" + file;
}
#endif

auto getNetworkConnections([[maybe_unused]] int pid)
    -> std::vector<NetworkConnection> {
    std::vector<NetworkConnection> connections;
#ifdef _WIN32
    MIB_TCPTABLE_OWNER_PID *pTCPInfo;
    DWORD dwSize = 0;
    GetExtendedTcpTable(nullptr, &dwSize, false, AF_INET,
                        TCP_TABLE_OWNER_PID_ALL, 0);
    pTCPInfo = (MIB_TCPTABLE_OWNER_PID *)malloc(dwSize);
    if (GetExtendedTcpTable(pTCPInfo, &dwSize, false, AF_INET,
                            TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pTCPInfo->dwNumEntries; i++) {
            if (pTCPInfo->table[i].dwOwningPid == pid) {
                std::ostringstream oss;
                oss << "Local: "
                    << inet_ntoa(*(in_addr *)&pTCPInfo->table[i].dwLocalAddr)
                    << ":" << ntohs((u_short)pTCPInfo->table[i].dwLocalPort);
                oss << " Remote: "
                    << inet_ntoa(*(in_addr *)&pTCPInfo->table[i].dwRemoteAddr)
                    << ":" << ntohs((u_short)pTCPInfo->table[i].dwRemotePort);
                connections.push_back({"TCP", oss.str()});
            }
        }
    }
    free(pTCPInfo);

#else
    for (const auto &[protocol, path] :
         {std::pair{"TCP", "net/tcp"}, {"UDP", "net/udp"}}) {
        std::ifstream netFile(ProcessManager::getProcFilePath(getpid(), path));
        if (netFile.is_open()) {
            std::string line;
            while (std::getline(netFile, line)) {
                connections.push_back({protocol, line});
            }
        }
    }
#endif
    return connections;
}

}  // namespace atom::system
