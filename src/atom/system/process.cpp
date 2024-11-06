#include "process.hpp"
#include "command.hpp"

#include <algorithm>
#include <fstream>
#include <optional>
#include <regex>

#if defined(_WIN32)
// clang-format off
#include <windows.h>
#include <tlhelp32.h>
#include <iphlpapi.h>
#include <tchar.h>
#include <psapi.h>
// clang-format on
#elif defined(__linux__) || defined(__ANDROID__)
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#if __has_include(<sys/capability.h>)
#include <sys/capability.h>
#endif
#elif defined(__APPLE__)
#include <libproc.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#else
#error "Unknown platform"
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
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
auto getProcessName(int pid) -> std::optional<std::string> {
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
                if (auto name = getProcessName(pid)) {
                    processes.emplace_back(pid, *name);
                }
            }
        }
    }

    closedir(procDir);
    return processes;
}

#elif defined(__APPLE__)
std::optional<std::string> getProcessName(int pid) {
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
        if (auto name = getProcessName(pid)) {
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

    try {
        for (const auto &entry : fs::directory_iterator(folderPath)) {
            const auto &path = entry.path();
            if (path.extension() == ".log") {
                logFiles.push_back(path);
            }
        }
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Error accessing directory {}: {}", folderPath, e.what());
        return "";
    }

    if (logFiles.empty()) {
        LOG_F(WARNING, "No log files found in directory {}", folderPath);
        return "";
    }

    auto latestFile = std::max_element(
        logFiles.begin(), logFiles.end(),
        [](const fs::path &a, const fs::path &b) {
            try {
                return fs::last_write_time(a) < fs::last_write_time(b);
            } catch (const fs::filesystem_error &e) {
                LOG_F(ERROR, "Error comparing file times: {}", e.what());
                return false;
            }
        });

    if (latestFile == logFiles.end()) {
        LOG_F(ERROR, "Failed to determine the latest log file in directory {}",
              folderPath);
        return "";
    }

    LOG_F(INFO, "Latest log file found: {}", latestFile->string());
    return latestFile->string();
}

auto getProcessInfo(int pid) -> Process {
    Process info;
    info.pid = pid;

    // 获取进程位置
#ifdef _WIN32
    HANDLE hProcess =
        OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess != nullptr) {
        wchar_t path[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, nullptr, path, MAX_PATH) != 0) {
            info.path = fs::path(path).string();
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

auto getSelfProcessInfo() -> Process {
#ifdef _WIN32
    return getProcessInfo(GetCurrentProcessId());
#else
    return getProcessInfo(getpid());
#endif
}

auto getProcessInfoByName(const std::string &processName)
    -> std::vector<Process> {
    std::vector<Process> processes;

#ifdef _WIN32
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Unable to create toolhelp snapshot.");
        return processes;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(snap, &entry)) {
        CloseHandle(snap);
        LOG_F(ERROR, "Unable to get the first process.");
        return processes;
    }

    do {
        std::string currentProcess =
            atom::utils::WCharArrayToString(entry.szExeFile);
        if (currentProcess == processName) {
            processes.push_back(getProcessInfo(entry.th32ProcessID));
        }
    } while (Process32NextW(snap, &entry));

    CloseHandle(snap);
#else
    std::string cmd = "pgrep -fl " + processName;
    auto [output, status] = executeCommandWithStatus(cmd);
    if (status != 0) {
        LOG_F(ERROR, "Failed to find process with name '{}'.", processName);
        return processes;
    }

    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        int pid;
        std::string name;
        if (lineStream >> pid >> name) {
            if (name == processName) {
                processes.push_back(getProcessInfo(pid));
            }
        }
    }
#endif

    return processes;
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

auto createProcessAsUser(const std::string &command, const std::string &user,
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
    try {
        for (const auto &entry : fs::directory_iterator("/proc")) {
            if (entry.is_directory()) {
                const std::string DIR_NAME = entry.path().filename().string();
                if (std::all_of(DIR_NAME.begin(), DIR_NAME.end(), ::isdigit)) {
                    std::ifstream cmdFile(entry.path() / "comm");
                    if (cmdFile) {
                        std::string cmdName;
                        std::getline(cmdFile, cmdName);
                        if (cmdName == processName) {
                            pids.push_back(
                                static_cast<pid_t>(std::stoi(DIR_NAME)));
                        }
                    }
                }
            }
        }
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Error reading /proc directory: {}", e.what());
    }
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
auto getPosixPrivileges(pid_t pid) -> PrivilegesInfo {
    PrivilegesInfo info;
    std::string procPath = "/proc/" + std::to_string(pid);

    // Read UID and GID from /proc/[pid]/status
    std::ifstream statusFile(procPath + "/status");
    if (!statusFile) {
        LOG_F(ERROR, "Failed to open /proc/{}/status", pid);
        return info;
    }

    std::string line;
    uid_t uid = -1;
    uid_t euid = -1;
    gid_t gid = -1;
    gid_t egid = -1;

    std::regex uidRegex(R"(Uid:\s+(\d+)\s+(\d+))");
    std::regex gidRegex(R"(Gid:\s+(\d+)\s+(\d+))");
    std::smatch match;

    while (std::getline(statusFile, line)) {
        if (std::regex_search(line, match, uidRegex)) {
            uid = std::stoi(match[1]);
            euid = std::stoi(match[2]);
        } else if (std::regex_search(line, match, gidRegex)) {
            gid = std::stoi(match[1]);
            egid = std::stoi(match[2]);
        }
    }

    struct passwd *pw = getpwuid(uid);
    struct group *gr = getgrgid(gid);

    if (pw != nullptr) {
        info.username = pw->pw_name;
        LOG_F(INFO, "User: {} (UID: {})", info.username, uid);
    } else {
        LOG_F(ERROR, "Failed to get user information for UID: {}", uid);
    }
    if (gr != nullptr) {
        info.groupname = gr->gr_name;
        LOG_F(INFO, "Group: {} (GID: {})", info.groupname, gid);
    } else {
        LOG_F(ERROR, "Failed to get group information for GID: {}", gid);
    }

    // Display effective user and group IDs
    if (uid != euid) {
        struct passwd *epw = getpwuid(euid);
        if (epw != nullptr) {
            LOG_F(INFO, "Effective User: {} (EUID: {})", epw->pw_name, euid);
        } else {
            LOG_F(ERROR, "Failed to get effective user information for EUID: {}",
                  euid);
        }
    }

    if (gid != egid) {
        struct group *egr = getgrgid(egid);
        if (egr != nullptr) {
            LOG_F(INFO, "Effective Group: {} (EGID: {})", egr->gr_name, egid);
        } else {
            LOG_F(ERROR, "Failed to get effective group information for EGID: {}",
                  egid);
        }
    }

#if defined(__linux__) && __has_include(<sys/capability.h>)
    // Check process capabilities on Linux systems
    std::ifstream capFile(procPath + "/status");
    if (capFile) {
        std::string capLine;
        while (std::getline(capFile, capLine)) {
            if (capLine.find("CapEff:") == 0) {
                info.privileges.push_back(capLine);
                LOG_F(INFO, "Capabilities: {}", capLine);
            }
        }
    } else {
        LOG_F(ERROR, "Failed to open /proc/{}/status", pid);
    }
#endif

    return info;
}
#endif

}  // namespace atom::system
