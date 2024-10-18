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
#include <tchar.h>
#include <psapi.h>
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

constexpr size_t BUFFER_SIZE = 256;

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

auto getProcessInfoByPid(int pid) -> Process {
    Process info;
    info.pid = pid;

    // 获取进程位置
#ifdef _WIN32
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess != nullptr) {
        wchar_t path[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, nullptr, path, MAX_PATH) != 0) {
            info.path = path;
        }
        CloseHandle(hProcess);
    }
#else
    char path[PATH_MAX];
    std::string procPath = "/proc/" + std::to_string(pid) + "/exe";
    ssize_t count = readlink(procPath.c_str(), path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
        info.path = path;
    }
#endif

    // 获取进程名称
    info.name = fs::path(info.path).filename().string();

    // 获取进程状态
    info.status = "Unknown";
#ifdef _WIN32
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

auto isProcessRunning(const std::string &processName) -> bool {
#ifdef _WIN32
    // Windows-specific: Use Toolhelp32 API to iterate over running processes.
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

#elif __APPLE__
    // macOS-specific: Use `pgrep` to search for the process.
    std::string command = "pgrep -x " + processName + " > /dev/null 2>&1";
    return system(command.c_str()) == 0;

#elif __linux__ || __ANDROID__
    // Linux/Android: Iterate over `/proc` to find the process by name.
    std::filesystem::path procDir("/proc");
    if (!std::filesystem::exists(procDir) ||
        !std::filesystem::is_directory(procDir))
        return false;

    for (const auto &p : std::filesystem::directory_iterator(procDir)) {
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

auto CreateProcessAsUser(const std::string &command, const std::string &user,
                         [[maybe_unused]] const std::string &domain,
                         [[maybe_unused]] const std::string &password) -> bool {
#ifdef _WIN32
    HANDLE tokenHandle = nullptr;
    HANDLE newTokenHandle = nullptr;
    STARTUPINFOW startupInfo;
    PROCESS_INFORMATION processInfo;
    bool result = false;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFOW));
    startupInfo.cb = sizeof(STARTUPINFOW);
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    struct Cleanup {
        HANDLE &tokenHandle;
        HANDLE &newTokenHandle;
        PROCESS_INFORMATION &processInfo;

        ~Cleanup() {
            if (tokenHandle != nullptr) {
                CloseHandle(tokenHandle);
            }
            if (newTokenHandle != nullptr) {
                CloseHandle(newTokenHandle);
            }
            if (processInfo.hProcess != nullptr) {
                CloseHandle(processInfo.hProcess);
            }
            if (processInfo.hThread != nullptr) {
                CloseHandle(processInfo.hThread);
            }
        }
    } cleanup{tokenHandle, newTokenHandle, processInfo};

    if (LogonUserA(atom::utils::StringToLPSTR(user),
                   atom::utils::StringToLPSTR(domain),
                   atom::utils::StringToLPSTR(password),
                   LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT,
                   &tokenHandle) == 0) {
        LOG_F(ERROR, "LogonUser failed with error: {}", GetLastError());
        return false;
    }

    if (DuplicateTokenEx(tokenHandle, TOKEN_ALL_ACCESS, nullptr,
                         SecurityImpersonation, TokenPrimary,
                         &newTokenHandle) == 0) {
        LOG_F(ERROR, "DuplicateTokenEx failed with error: {}", GetLastError());
        return false;
    }

    if (CreateProcessAsUserW(newTokenHandle, nullptr,
                             atom::utils::StringToLPWSTR(command), nullptr,
                             nullptr, FALSE, 0, nullptr, nullptr, &startupInfo,
                             &processInfo) == 0) {
        LOG_F(ERROR, "CreateProcessAsUser failed with error: {}",
              GetLastError());
        return false;
    }
    LOG_F(INFO, "Process created successfully!");
    result = true;
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    return result;
#else
    pid_t pid = fork();
    if (pid < 0) {
        LOG_F(ERROR, "Fork failed");
        return false;
    } else if (pid == 0) {
        struct passwd *pw = getpwnam(user.c_str());
        if (pw == nullptr) {
            LOG_F(ERROR, "Failed to get user information for {}", user);
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
        execl("/bin/sh", "sh", "-c", command.c_str(), (char *)nullptr);
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

auto parseAddressPort(const std::string &addressPort)
    -> std::pair<std::string, int> {
    size_t colonPos = addressPort.find_last_of(':');
    if (colonPos != std::string::npos) {
        std::string address = addressPort.substr(0, colonPos);
        int port = std::stoi(addressPort.substr(colonPos + 1));
        return {address, port};
    }
    return {"", 0};
}

auto getNetworkConnections(int pid) -> std::vector<NetworkConnection> {
    std::vector<NetworkConnection> connections;

#ifdef _WIN32
    // Windows: Use GetExtendedTcpTable to get TCP connections.
    MIB_TCPTABLE_OWNER_PID *pTCPInfo = nullptr;
    DWORD dwSize = 0;
    GetExtendedTcpTable(nullptr, &dwSize, false, AF_INET,
                        TCP_TABLE_OWNER_PID_ALL, 0);
    pTCPInfo = (MIB_TCPTABLE_OWNER_PID *)malloc(dwSize);
    if (GetExtendedTcpTable(pTCPInfo, &dwSize, false, AF_INET,
                            TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pTCPInfo->dwNumEntries; ++i) {
            if (pTCPInfo->table[i].dwOwningPid == pid) {
                NetworkConnection conn;
                conn.protocol = "TCP";
                conn.localAddress =
                    inet_ntoa(*(in_addr *)&pTCPInfo->table[i].dwLocalAddr);
                conn.localPort = ntohs((u_short)pTCPInfo->table[i].dwLocalPort);
                conn.remoteAddress =
                    inet_ntoa(*(in_addr *)&pTCPInfo->table[i].dwRemoteAddr);
                conn.remotePort =
                    ntohs((u_short)pTCPInfo->table[i].dwRemotePort);
                connections.push_back(conn);
                LOG_F(INFO, "Found TCP connection: Local {}:{} -> Remote {}:{}",
                      conn.localAddress, conn.localPort, conn.remoteAddress,
                      conn.remotePort);
            }
        }
    } else {
        LOG_F(ERROR, "Failed to get TCP table. Error: {}", GetLastError());
    }
    free(pTCPInfo);

#elif __APPLE__
    // macOS: Use `lsof` to get network connections.
    std::array<char, 128> buffer;
    std::string command = "lsof -i -n -P | grep " + std::to_string(pid);
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        LOG_F(ERROR, "Failed to run lsof command.");
        return connections;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::istringstream iss(buffer.data());
        std::string proto, local, remote, ignore;
        iss >> ignore >> ignore >> ignore >> proto >> local >> remote;

        auto [localAddr, localPort] = parseAddressPort(local);
        auto [remoteAddr, remotePort] = parseAddressPort(remote);

        connections.push_back(
            {proto, localAddr, remoteAddr, localPort, remotePort});
        LOG_F(INFO, "Found {} connection: Local {}:{} -> Remote {}:{}", proto,
              localAddr, localPort, remoteAddr, remotePort);
    }
    pclose(pipe);

#elif __linux__ || __ANDROID__
    // Linux/Android: Parse /proc/<pid>/net/tcp and /proc/<pid>/net/udp.
    for (const auto &[protocol, path] :
         {std::pair{"TCP", "net/tcp"}, {"UDP", "net/udp"}}) {
        std::ifstream netFile("/proc/" + std::to_string(pid) + "/" + path);
        if (!netFile.is_open()) {
            LOG_F(ERROR, "Failed to open: /proc/{}/{}", pid, path);
            continue;
        }

        std::string line;
        std::getline(netFile, line);  // Skip header line.

        while (std::getline(netFile, line)) {
            std::istringstream iss(line);
            std::string localAddress, remoteAddress, ignore;
            int state, inode;

            // Parse the fields from the /proc file.
            iss >> ignore >> localAddress >> remoteAddress >> std::hex >>
                state >> ignore >> ignore >> ignore >> inode;

            auto [localAddr, localPort] = parseAddressPort(localAddress);
            auto [remoteAddr, remotePort] = parseAddressPort(remoteAddress);

            connections.push_back(
                {protocol, localAddr, remoteAddr, localPort, remotePort});
            LOG_F(INFO, "Found {} connection: Local {}:{} -> Remote {}:{}",
                  protocol, localAddr, localPort, remoteAddr, remotePort);
        }
    }
#endif

    return connections;
}

auto getProcessIdByName(const std::string &processName) -> std::vector<int> {
    std::vector<int> pids;
#ifdef _WIN32
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to create snapshot!");
        return pids;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe32) != 0) {
        do {
            if (_stricmp(pe32.szExeFile, processName.c_str()) == 0) {
                pids.push_back(static_cast<int>(pe32.th32ProcessID));
            }
        } while (Process32Next(hSnapshot, &pe32) != 0);
    }

    CloseHandle(hSnapshot);
#elif defined(__linux__)
    DIR *dir = opendir("/proc");
    if (!dir) {
        LOG_F(ERROR, "Failed to open /proc directory.");
        return pids;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (isdigit(entry->d_name[0])) {
            std::string pid_dir = std::string("/proc/") + entry->d_name;
            std::ifstream cmd_file(pid_dir + "/comm");
            if (cmd_file) {
                std::string cmd_name;
                std::getline(cmd_file, cmd_name);
                if (cmd_name == processName) {
                    pids.push_back(
                        static_cast<pid_t>(std::stoi(entry->d_name)));
                }
            }
        }
    }
    closedir(dir);
#elif defined(__APPLE__)
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    struct kinfo_proc *processList = nullptr;
    size_t size = 0;

    if (sysctl(mib, 4, nullptr, &size, nullptr, 0) == -1) {
        LOG_F(ERROR, "Failed to get process size.");
        return pids;
    }

    processList = new kinfo_proc[size / sizeof(struct kinfo_proc)];
    if (sysctl(mib, 4, processList, &size, nullptr, 0) == -1) {
        LOG_F(ERROR, "Failed to get process list.");
        delete[] processList;
        return pids;
    }

    for (size_t i = 0; i < size / sizeof(struct kinfo_proc); ++i) {
        char processPath[PROC_PIDPATHINFO_MAXSIZE];
        proc_pidpath(processList[i].kp_proc.p_pid, processPath,
                     sizeof(processPath));

        std::string proc_name = processPath;
        if (proc_name.find(processName) != std::string::npos) {
            pids.push_back(processList[i].kp_proc.p_pid);
        }
    }
    delete[] processList;
#else
#error "Unsupported operating system"
#endif
    return pids;
}

#ifdef _WIN32
// Get current user privileges on Windows systems
auto getWindowsPrivileges(int pid) -> PrivilegesInfo {
    PrivilegesInfo info;
    HANDLE tokenHandle = nullptr;
    DWORD tokenInfoLength = 0;
    PTOKEN_PRIVILEGES privileges = nullptr;
    std::array<char, BUFFER_SIZE> username;
    auto usernameLen = static_cast<DWORD>(username.size());

    // Get the username
    if (GetUserNameA(username.data(), &usernameLen) != 0) {
        info.username = username.data();
        LOG_F(INFO, "Current User: {}", info.username);
    } else {
        LOG_F(ERROR, "Failed to get username. Error: {}", GetLastError());
        
    }

    // Open the access token of the specified process
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (processHandle != nullptr &&
        OpenProcessToken(processHandle, TOKEN_QUERY, &tokenHandle) != 0) {
        CloseHandle(processHandle);
        // Get the length of the token information
        GetTokenInformation(tokenHandle, TokenPrivileges, nullptr, 0,
                            &tokenInfoLength);
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            privileges =
                static_cast<PTOKEN_PRIVILEGES>(malloc(tokenInfoLength));
            if (privileges != nullptr &&
                GetTokenInformation(tokenHandle, TokenPrivileges, privileges,
                                    tokenInfoLength, &tokenInfoLength) != 0) {
                LOG_F(INFO, "Privileges:");
                for (DWORD i = 0; i < privileges->PrivilegeCount; ++i) {
                    LUID_AND_ATTRIBUTES laa = privileges->Privileges[i];
                    std::array<char, BUFFER_SIZE> privilegeName;
                    auto nameSize = static_cast<DWORD>(privilegeName.size());

                    // Get the privilege name
                    if (LookupPrivilegeNameA(nullptr, &laa.Luid,
                                             privilegeName.data(),
                                             &nameSize) != 0) {
                        std::string privilege = privilegeName.data();
                        privilege +=
                            (laa.Attributes & SE_PRIVILEGE_ENABLED) != 0
                                ? " - Enabled"
                                : " - Disabled";
                        info.privileges.push_back(privilege);
                        LOG_F(INFO, "  {}", privilege);
                    } else {
                        LOG_F(ERROR,
                              "Failed to lookup privilege name. Error: {}",
                              GetLastError());
                    }
                }
            } else {
                LOG_F(ERROR, "Failed to get token information. Error: {}",
                      GetLastError());
            }
            free(privileges);
        } else {
            LOG_F(ERROR, "Failed to get token information length. Error: {}",
                  GetLastError());
        }
        CloseHandle(tokenHandle);
    } else {
        LOG_F(ERROR, "Failed to open process token. Error: {}", GetLastError());
    }

    // Check if the user is an administrator
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    PSID administratorsGroup;
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                                 &administratorsGroup) != 0) {
        CheckTokenMembership(nullptr, administratorsGroup, &isAdmin);
        FreeSid(administratorsGroup);
    } else {
        LOG_F(ERROR, "Failed to allocate and initialize SID. Error: {}",
              GetLastError());
    }
    info.isAdmin = isAdmin != 0;
    LOG_F(INFO, "User has {}Administrator privileges.",
          info.isAdmin ? "" : "no ");

    return info;
}

#else
// Get current user and group privileges on POSIX systems
auto get_posix_privileges() -> PrivilegesInfo {
    PrivilegesInfo info;
    uid_t uid = getuid();    // Real user ID
    gid_t gid = getgid();    // Real group ID
    uid_t euid = geteuid();  // Effective user ID
    gid_t egid = getegid();  // Effective group ID

    struct passwd *pw = getpwuid(uid);
    struct group *gr = getgrgid(gid);

    if (pw) {
        info.username = pw->pw_name;
        LOG_F(INFO, "User: {} (UID: {})", info.username, uid);
    } else {
        LOG_F(ERROR, "Failed to get user information for UID: {}", uid);
    }
    if (gr) {
        info.groupname = gr->gr_name;
        LOG_F(INFO, "Group: {} (GID: {})", info.groupname, gid);
    } else {
        LOG_F(ERROR, "Failed to get group information for GID: {}", gid);
    }

    // Display effective user and group IDs
    if (uid != euid) {
        struct passwd *epw = getpwuid(euid);
        if (epw) {
            LOG_F(INFO, "Effective User: {} (EUID: {})", epw->pw_name, euid);
        } else {
            LOG_F(ERROR,
                  "Failed to get effective user information for EUID: {}",
                  euid);
        }
    }

    if (gid != egid) {
        struct group *egr = getgrgid(egid);
        if (egr) {
            LOG_F(INFO, "Effective Group: {} (EGID: {})", egr->gr_name, egid);
        } else {
            LOG_F(ERROR,
                  "Failed to get effective group information for EGID: {}",
                  egid);
        }
    }

#ifdef __linux__
    // Check process capabilities on Linux systems
    cap_t caps = cap_get_proc();
    if (caps) {
        info.privileges.push_back(cap_to_text(caps, nullptr));
        LOG_F(INFO, "Capabilities: {}", cap_to_text(caps, nullptr));
        cap_free(caps);
    } else {
        LOG_F(ERROR, "Failed to get capabilities.");
    }
#endif

    return info;
}
#endif
}  // namespace atom::system
