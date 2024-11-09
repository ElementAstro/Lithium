// process.cpp
/*
 * process.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-24

Description: Enhanced Process Manager Implementation

**************************************************/

#include "process_manager.hpp"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <sstream>

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

constexpr size_t BUFFER_SIZE = 256;

class ProcessManager::ProcessManagerImpl {
public:
    explicit ProcessManagerImpl(int maxProcess) : m_maxProcesses(maxProcess) {}

    ~ProcessManagerImpl() {
        // Ensure all processes are cleaned up
        waitForCompletion();
    }

    ProcessManagerImpl(const ProcessManagerImpl &) = delete;
    ProcessManagerImpl &operator=(const ProcessManagerImpl &) = delete;
    ProcessManagerImpl(ProcessManagerImpl &&) = delete;
    ProcessManagerImpl &operator=(ProcessManagerImpl &&) = delete;

    auto createProcess(const std::string &command,
                       const std::string &identifier,
                       bool isBackground) -> bool {
        if (processes.size() >= static_cast<size_t>(m_maxProcesses)) {
            LOG_F(ERROR, "Maximum number of managed processes reached.");
            THROW_PROCESS_ERROR("Maximum number of managed processes reached.");
        }

        pid_t pid;
#ifdef _WIN32
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // Create the child process.
        BOOL success = CreateProcessA(
            NULL,  // No module name (use command line)
            const_cast<char *>(command.c_str()),  // Command line
            NULL,   // Process handle not inheritable
            NULL,   // Thread handle not inheritable
            FALSE,  // Set handle inheritance to FALSE
            isBackground ? CREATE_NO_WINDOW : 0,  // Creation flags
            NULL,  // Use parent's environment block
            NULL,  // Use parent's starting directory
            &si,   // Pointer to STARTUPINFO structure
            &pi    // Pointer to PROCESS_INFORMATION structure
        );

        if (!success) {
            DWORD error = GetLastError();
            LOG_F(ERROR, "CreateProcess failed with error code: {}", error);
            THROW_PROCESS_ERROR("Failed to create process.");
        }

        pid = pi.dwProcessId;
#else
        pid = fork();
        if (pid == 0) {
            // Child process
            if (isBackground) {
                // Detach from terminal
                if (setsid() < 0) {
                    _exit(EXIT_FAILURE);
                }
            }
            execlp(command.c_str(), command.c_str(), nullptr);
            // If execlp fails
            LOG_F(ERROR, "execlp failed for command: {}", command);
            _exit(EXIT_FAILURE);
        } else if (pid < 0) {
            LOG_F(ERROR, "Failed to fork process for command: {}", command);
            THROW_PROCESS_ERROR("Failed to fork process.");
        }
#endif
        std::unique_lock lock(mtx);
        Process process;
        process.pid = pid;
        process.name = identifier;
        process.command = command;
        process.isBackground = isBackground;
#ifdef _WIN32
        process.handle = pi.hProcess;
#endif
        processes.emplace_back(process);
        LOG_F(INFO, "Process created: PID={}, Name={}", pid, identifier);
        return true;
    }

    auto terminateProcess(int pid, int signal) -> bool {
        std::unique_lock lock(mtx);
        auto processIt = std::find_if(
            processes.begin(), processes.end(),
            [pid](const Process &process) { return process.pid == pid; });

        if (processIt != processes.end()) {
#ifdef _WIN32
            // Windows-specific process termination
            if (!TerminateProcess(processIt->handle, 1)) {
                DWORD error = GetLastError();
                LOG_F(ERROR, "TerminateProcess failed with error code: {}",
                      error);
                THROW_PROCESS_ERROR("Failed to terminate process.");
            }
            CloseHandle(processIt->handle);
#else
            if (kill(pid, signal) != 0) {
                LOG_F(ERROR, "Failed to send signal {} to PID {}", signal, pid);
                THROW_PROCESS_ERROR("Failed to terminate process.");
            }
#endif
            LOG_F(INFO, "Process terminated: PID={}, Signal={}", pid, signal);
            processes.erase(processIt);
            cv.notify_all();
            return true;
        }
        LOG_F(WARNING, "Attempted to terminate non-existent PID: {}", pid);
        return false;
    }

    auto terminateProcessByName(const std::string &name, int signal) -> bool {
        std::unique_lock lock(mtx);
        bool success = false;
        for (auto processIt = processes.begin();
             processIt != processes.end();) {
            if (processIt->name == name) {
                try {
                    terminateProcess(processIt->pid, signal);
                    success = true;
                } catch (const ProcessException &e) {
                    LOG_F(ERROR, "Failed to terminate process {}: {}", name,
                          e.what());
                }
                processIt = processes.erase(processIt);
            } else {
                ++processIt;
            }
        }
        return success;
    }

    void waitForCompletion() {
        std::unique_lock lock(mtx);
        // TODO: Implement a more efficient way to wait for all processes to
        // complete cv.wait(lock, [this] { return processes.empty(); });
        LOG_F(INFO, "All managed processes have completed.");
    }

    auto runScript(const std::string &script, const std::string &identifier,
                   bool isBackground) -> bool {
        // Assuming the script is executable
        return createProcess(script, identifier, isBackground);
    }

    auto monitorProcesses() -> bool {
#ifdef _WIN32
        // Windows-specific monitoring can be implemented using
        // WaitForSingleObject or similar APIs For simplicity, not implemented
        // here
        LOG_F(WARNING, "Process monitoring not implemented for Windows.");
        return false;
#elif defined(__linux__) || defined(__APPLE__)
        std::unique_lock lock(mtx);
        for (auto processIt = processes.begin();
             processIt != processes.end();) {
            int status;
            pid_t result = waitpid(processIt->pid, &status, WNOHANG);
            if (result == 0) {
                // Process is still running
                ++processIt;
            } else if (result == -1) {
                LOG_F(ERROR, "Error monitoring PID {}: {}", processIt->pid,
                      [&] {
                          std::array<char, BUFFER_SIZE> buffer;
                          strerror_r(errno, buffer.data(), buffer.size());
                          return std::string(buffer.data());
                      }());
                processIt = processes.erase(processIt);
            } else {
                // Process has terminated
                LOG_F(INFO, "Process terminated: PID={}, Status={}",
                      processIt->pid, status);
                processIt = processes.erase(processIt);
                cv.notify_all();
            }
        }
        return true;
#else
        LOG_F(WARNING, "Process monitoring not implemented for this platform.");
        return false;
#endif
    }

    auto getProcessInfo(int pid) -> Process {
        std::shared_lock lock(mtx);
        auto processIt = std::find_if(
            processes.begin(), processes.end(),
            [pid](const Process &process) { return process.pid == pid; });
        if (processIt != processes.end()) {
            return *processIt;
        }
        LOG_F(ERROR, "Process with PID {} not found.", pid);
        THROW_PROCESS_ERROR("Process not found.");
    }

#ifdef _WIN32
    auto getProcessHandle(int pid) const -> void * {
        std::shared_lock lock(mtx);
        auto processIt = std::find_if(
            processes.begin(), processes.end(),
            [pid](const Process &process) { return process.pid == pid; });
        if (processIt != processes.end()) {
            return processIt->handle;
        }
        LOG_F(ERROR, "Process handle for PID {} not found.", pid);
        THROW_PROCESS_ERROR("Process handle not found.");
    }
#else
    static auto getProcFilePath(int pid,
                                const std::string &file) -> std::string {
        std::string path = "/proc/" + std::to_string(pid) + "/" + file;
        if (access(path.c_str(), F_OK) != 0) {
            LOG_F(ERROR, "File {} not found for PID {}.", file, pid);
            THROW_PROCESS_ERROR("Process file path not found.");
        }
        return path;
    }
#endif

    auto getRunningProcesses() const -> std::vector<Process> {
        std::shared_lock lock(mtx);
        return processes;
    }

    int m_maxProcesses;
    std::condition_variable cv;
    std::vector<Process> processes;
    mutable std::shared_timed_mutex mtx;
};

ProcessManager::ProcessManager(int maxProcess)
    : impl(std::make_unique<ProcessManagerImpl>(maxProcess)) {}

ProcessManager::~ProcessManager() = default;

auto ProcessManager::createShared(int maxProcess)
    -> std::shared_ptr<ProcessManager> {
    return std::make_shared<ProcessManager>(maxProcess);
}

auto ProcessManager::createProcess(const std::string &command,
                                   const std::string &identifier,
                                   bool isBackground) -> bool {
    try {
        return impl->createProcess(command, identifier, isBackground);
    } catch (const ProcessException &e) {
        LOG_F(ERROR, "Failed to create process {}: {}", identifier, e.what());
        THROW_NESTED_PROCESS_ERROR(e.what());
    }
}

auto ProcessManager::terminateProcess(int pid, int signal) -> bool {
    try {
        return impl->terminateProcess(pid, signal);
    } catch (const ProcessException &e) {
        LOG_F(ERROR, "Failed to terminate PID {}: {}", pid, e.what());
        return false;
    }
}

auto ProcessManager::terminateProcessByName(const std::string &name,
                                            int signal) -> bool {
    try {
        return impl->terminateProcessByName(name, signal);
    } catch (const ProcessException &e) {
        LOG_F(ERROR, "Failed to terminate process {}: {}", name, e.what());
        return false;
    }
}

auto ProcessManager::hasProcess(const std::string &identifier) -> bool {
    std::shared_lock lock(impl->mtx);
    return std::any_of(impl->processes.begin(), impl->processes.end(),
                       [&identifier](const Process &process) {
                           return process.name == identifier;
                       });
}

void ProcessManager::waitForCompletion() { impl->waitForCompletion(); }

auto ProcessManager::getRunningProcesses() const -> std::vector<Process> {
    return impl->getRunningProcesses();
}

auto ProcessManager::getProcessOutput(const std::string &identifier)
    -> std::vector<std::string> {
    std::shared_lock lock(impl->mtx);
    auto processIt =
        std::find_if(impl->processes.begin(), impl->processes.end(),
                     [&identifier](const Process &process) {
                         return process.name == identifier;
                     });

    if (processIt != impl->processes.end()) {
        std::vector<std::string> outputLines;
        std::stringstream sss(processIt->output);
        std::string line;

        while (std::getline(sss, line)) {
            outputLines.emplace_back(line);
        }

        return outputLines;
    }
    LOG_F(WARNING, "No output found for process identifier: {}", identifier);
    return {};
}

auto ProcessManager::runScript(const std::string &script,
                               const std::string &identifier,
                               bool isBackground) -> bool {
    try {
        return impl->runScript(script, identifier, isBackground);
    } catch (const ProcessException &e) {
        LOG_F(ERROR, "Failed to run script {}: {}", identifier, e.what());
        return false;
    }
}

auto ProcessManager::monitorProcesses() -> bool {
    return impl->monitorProcesses();
}

auto ProcessManager::getProcessInfo(int pid) -> Process {
    try {
        return impl->getProcessInfo(pid);
    } catch (const ProcessException &e) {
        LOG_F(ERROR, "Failed to get info for PID {}: {}", pid, e.what());
        throw;
    }
}

#ifdef _WIN32
auto ProcessManager::getProcessHandle(int pid) const -> void * {
    try {
        return impl->getProcessHandle(pid);
    } catch (const ProcessException &e) {
        LOG_F(ERROR, "Failed to get handle for PID {}: {}", pid, e.what());
        throw;
    }
}
#else
auto ProcessManager::getProcFilePath(int pid,
                                     const std::string &file) -> std::string {
    try {
        return ProcessManagerImpl::getProcFilePath(pid, file);
    } catch (const ProcessException &e) {
        LOG_F(ERROR, "Failed to get file path for PID {}: {}", pid, e.what());
        throw;
    }
}
#endif

}  // namespace atom::system
