/*
 * sandbox.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A sandbox for isolated components, such as executables.

**************************************************/

#include "sandbox.hpp"

#ifdef _WIN32
// clang-format off
#include <windows.h>
#include <psapi.h>
// clang-format on
#else
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace lithium {

bool Sandbox::setTimeLimit(int timeLimitMs) {
    m_timeLimit = timeLimitMs;
#ifndef _WIN32
    rlimit limit{.rlim_cur = static_cast<rlim_t>(timeLimitMs) / 1000,
                 .rlim_max = static_cast<rlim_t>(timeLimitMs) / 1000};
    return setrlimit(RLIMIT_CPU, &limit) == 0;
#endif
    return true;
}

bool Sandbox::setMemoryLimit(long memoryLimitKb) {
    m_memoryLimit = memoryLimitKb;
#ifndef _WIN32
    rlimit limit{.rlim_cur = static_cast<rlim_t>(memoryLimitKb) * 1024,
                 .rlim_max = static_cast<rlim_t>(memoryLimitKb) * 1024};
    return setrlimit(RLIMIT_AS, &limit) == 0;
#endif
    return true;
}

bool Sandbox::setRootDirectory(const std::string& rootDirectory) {
    m_rootDirectory = rootDirectory;
#ifndef _WIN32
    return (chdir(rootDirectory.c_str()) == 0) &&
           (chroot(rootDirectory.c_str()) == 0);
#endif
    return true;
}

bool Sandbox::setUserId(int userId) {
    m_userId = userId;
#ifndef _WIN32
    return (setuid(userId) == 0) && (setgid(userId) == 0);
#endif
    return true;
}

bool Sandbox::setProgramPath(const std::string& programPath) {
    m_programPath = programPath;
    return true;
}

bool Sandbox::setProgramArgs(const std::vector<std::string>& programArgs) {
    m_programArgs = programArgs;
    return true;
}

#ifdef _WIN32
bool Sandbox::setWindowsLimits(PROCESS_INFORMATION& processInfo) {
    const HANDLE processHandle = processInfo.hProcess;

    if (m_timeLimit > 0) {
        SetProcessAffinityMask(processHandle,
                               static_cast<DWORD_PTR>(m_timeLimit));
    }

    if (m_memoryLimit > 0) {
        SetProcessWorkingSetSize(processHandle,
                                 static_cast<SIZE_T>(m_memoryLimit),
                                 static_cast<SIZE_T>(m_memoryLimit));
    }

    return true;
}
#else
bool Sandbox::setUnixLimits() {
    if (m_timeLimit > 0) {
        rlimit limit{.rlim_cur = static_cast<rlim_t>(m_timeLimit) / 1000,
                     .rlim_max = static_cast<rlim_t>(m_timeLimit) / 1000};
        if (setrlimit(RLIMIT_CPU, &limit) != 0) {
            return false;
        }
    }

    if (m_memoryLimit > 0) {
        rlimit limit{.rlim_cur = static_cast<rlim_t>(m_memoryLimit) * 1024,
                     .rlim_max = static_cast<rlim_t>(m_memoryLimit) * 1024};
        if (setrlimit(RLIMIT_AS, &limit) != 0) {
            return false;
        }
    }

    return true;
}
#endif

auto Sandbox::run() -> bool {
#ifdef _WIN32
    STARTUPINFO startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};

    std::string commandLine = m_programPath;
    for (const auto& arg : m_programArgs) {
        commandLine += ' ' + arg;
    }

    if (!CreateProcess(nullptr, commandLine.data(), nullptr, nullptr, FALSE,
                       CREATE_SUSPENDED, nullptr, nullptr, &startupInfo,
                       &processInfo)) {
        return false;
    }

    if (!setWindowsLimits(processInfo)) {
        return false;
    }

    ResumeThread(processInfo.hThread);
    WaitForSingleObject(processInfo.hProcess, INFINITE);

    DWORD exitCode = 0;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);

    PROCESS_MEMORY_COUNTERS memoryCounters{};
    GetProcessMemoryInfo(processInfo.hProcess, &memoryCounters,
                         sizeof(memoryCounters));

    m_timeUsed = GetTickCount();
    m_memoryUsed = static_cast<long>(memoryCounters.WorkingSetSize / 1024);

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return exitCode == 0;
#else
    const pid_t PID = fork();
    if (PID < 0) {
        return false;
    }
    if (PID == 0) {
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);

        std::vector<char*> args;
        args.reserve(m_programArgs.size() + 2);
        args.emplace_back(m_programPath.data());
        for (const auto& arg : m_programArgs) {
            args.emplace_back(const_cast<char*>(arg.c_str()));
        }
        args.emplace_back(nullptr);

        if (!setUnixLimits()) {
            exit(1);
        }

        execvp(m_programPath.c_str(), args.data());
        exit(0);
    } else {
        int status = 0;
        rusage usage{};
        while (wait4(PID, &status, 0, &usage) != PID) {
            ;
        }
        m_timeUsed = static_cast<int>(usage.ru_utime.tv_sec * 1000 +
                                      usage.ru_utime.tv_usec / 1000);
        m_memoryUsed = static_cast<long>(usage.ru_maxrss);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
#endif
}

}  // namespace lithium
