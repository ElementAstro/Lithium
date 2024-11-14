#include "async_system_command.hpp"
#include "atom/log/loguru.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cerrno>
#include <csignal>
#include <cstring>
#endif

AsyncSystemCommand::AsyncSystemCommand(const std::string& cmd)
    : cmd_(cmd), pid_(0), running_(false) {
    LOG_F(INFO, "AsyncSystemCommand created with command: {}", cmd);
}

AsyncSystemCommand::~AsyncSystemCommand() {
    LOG_F(INFO, "AsyncSystemCommand destructor called");
    terminate();
}

void AsyncSystemCommand::run() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (running_) {
        LOG_F(WARNING, "Command already running");
        return;
    }

#ifdef _WIN32
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    if (!CreateProcess(NULL, const_cast<char*>(cmd_.c_str()), NULL, NULL, FALSE,
                       0, NULL, NULL, &si, &pi)) {
        LOG_F(ERROR, "CreateProcess failed: {}", GetLastError());
        return;
    }

    pid_ = pi.dwProcessId;
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
#else
    pid_ = fork();
    if (pid_ < 0) {
        LOG_F(ERROR, "Fork failed: {}", strerror(errno));
        return;
    }

    if (pid_ == 0) {  // Child process
        // Create new process group
        setsid();

        // Execute command
        execl("/bin/sh", "sh", "-c", cmd_.c_str(), nullptr);

        // If execl fails
        LOG_F(ERROR, "Exec failed: {}", strerror(errno));
        _exit(1);
    }
#endif

    // Parent process
    running_ = true;
    LOG_F(INFO, "Started command with PID {}", pid_.load());
}

void AsyncSystemCommand::terminate() {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!running_) {
        LOG_F(INFO, "No running command to terminate");
        return;
    }

    pid_t pid = pid_.load();
    if (pid <= 0) {
        running_ = false;
        LOG_F(WARNING, "Invalid PID: {}", pid);
        return;
    }

#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        LOG_F(ERROR, "OpenProcess failed: {}", GetLastError());
        return;
    }

    if (!TerminateProcess(hProcess, 0)) {
        LOG_F(ERROR, "TerminateProcess failed: {}", GetLastError());
    } else {
        LOG_F(INFO, "Process {} terminated", pid);
    }

    CloseHandle(hProcess);
#else
    // Kill entire process group
    if (kill(-pid, SIGTERM) == 0) {
        int status;
        waitpid(pid, &status, 0);
        LOG_F(INFO, "Process {} terminated", pid);
    } else {
        LOG_F(ERROR, "Failed to terminate process {}: {}", pid,
              strerror(errno));
    }
#endif

    pid_ = 0;
    running_ = false;
}

bool AsyncSystemCommand::isRunning() const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!running_) {
        LOG_F(INFO, "No running command");
        return false;
    }

    pid_t pid = pid_.load();
    if (pid <= 0) {
        LOG_F(WARNING, "Invalid PID: {}", pid);
        return false;
    }

#ifdef _WIN32
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        LOG_F(ERROR, "OpenProcess failed: {}", GetLastError());
        return false;
    }

    DWORD exitCode;
    if (!GetExitCodeProcess(hProcess, &exitCode)) {
        LOG_F(ERROR, "GetExitCodeProcess failed: {}", GetLastError());
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    if (exitCode == STILL_ACTIVE) {
        LOG_F(INFO, "Process {} is still running", pid);
        return true;
    }
#else
    // Check if process exists
    if (kill(pid, 0) == 0) {
        LOG_F(INFO, "Process {} is still running", pid);
        return true;
    }
#endif

    // Process no longer exists
    const_cast<AsyncSystemCommand*>(this)->running_ = false;
    LOG_F(INFO, "Process {} is no longer running", pid);
    return false;
}