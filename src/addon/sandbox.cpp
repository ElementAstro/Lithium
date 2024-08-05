/*
 * sandbox.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

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

class SandboxImpl {
public:
    int mTimeLimit{0};
    long mMemoryLimit{0};
    std::string mRootDirectory;
    int mUserId{0};
    std::string mProgramPath;
    std::vector<std::string> mProgramArgs;
    int mTimeUsed{0};
    long mMemoryUsed{0};

    auto setTimeLimit(int timeLimitMs) -> bool;
    auto setMemoryLimit(long memoryLimitKb) -> bool;
    auto setRootDirectory(const std::string& rootDirectory) -> bool;
    auto setUserId(int userId) -> bool;
    auto setProgramPath(const std::string& programPath) -> bool;
    auto setProgramArgs(const std::vector<std::string>& programArgs) -> bool;
    auto run() -> bool;
#ifdef _WIN32
    auto setWindowsLimits(PROCESS_INFORMATION& processInfo) const -> bool;
#else
    bool setUnixLimits();
#endif
};

Sandbox::Sandbox() : pimpl(std::make_unique<SandboxImpl>()) {}

Sandbox::~Sandbox() = default;

auto Sandbox::setTimeLimit(int timeLimitMs) -> bool {
    return pimpl->setTimeLimit(timeLimitMs);
}

auto Sandbox::setMemoryLimit(long memoryLimitKb) -> bool {
    return pimpl->setMemoryLimit(memoryLimitKb);
}

auto Sandbox::setRootDirectory(const std::string& rootDirectory) -> bool {
    return pimpl->setRootDirectory(rootDirectory);
}

auto Sandbox::setUserId(int userId) -> bool { return pimpl->setUserId(userId); }

auto Sandbox::setProgramPath(const std::string& programPath) -> bool {
    return pimpl->setProgramPath(programPath);
}

auto Sandbox::setProgramArgs(const std::vector<std::string>& programArgs)
    -> bool {
    return pimpl->setProgramArgs(programArgs);
}

auto Sandbox::run() -> bool { return pimpl->run(); }

auto Sandbox::getTimeUsed() const -> int { return pimpl->mTimeUsed; }

auto Sandbox::getMemoryUsed() const -> long { return pimpl->mMemoryUsed; }

auto SandboxImpl::setTimeLimit(int timeLimitMs) -> bool {
    mTimeLimit = timeLimitMs;
#ifndef _WIN32
    rlimit limit{.rlim_cur = static_cast<rlim_t>(timeLimitMs) / 1000,
                 .rlim_max = static_cast<rlim_t>(timeLimitMs) / 1000};
    return setrlimit(RLIMIT_CPU, &limit) == 0;
#endif
    return true;
}

auto SandboxImpl::setMemoryLimit(long memoryLimitKb) -> bool {
    mMemoryLimit = memoryLimitKb;
#ifndef _WIN32
    rlimit limit{.rlim_cur = static_cast<rlim_t>(memoryLimitKb) * 1024,
                 .rlim_max = static_cast<rlim_t>(memoryLimitKb) * 1024};
    return setrlimit(RLIMIT_AS, &limit) == 0;
#endif
    return true;
}

auto SandboxImpl::setRootDirectory(const std::string& rootDirectory) -> bool {
    mRootDirectory = rootDirectory;
#ifndef _WIN32
    return (chdir(rootDirectory.c_str()) == 0) &&
           (chroot(rootDirectory.c_str()) == 0);
#endif
    return true;
}

auto SandboxImpl::setUserId(int userId) -> bool {
    mUserId = userId;
#ifndef _WIN32
    return (setuid(userId) == 0) && (setgid(userId) == 0);
#endif
    return true;
}

auto SandboxImpl::setProgramPath(const std::string& programPath) -> bool {
    mProgramPath = programPath;
    return true;
}

auto SandboxImpl::setProgramArgs(const std::vector<std::string>& programArgs)
    -> bool {
    mProgramArgs = programArgs;
    return true;
}

#ifdef _WIN32
auto SandboxImpl::setWindowsLimits(PROCESS_INFORMATION& processInfo) const
    -> bool {
    const HANDLE PROCESS_HANDLE = processInfo.hProcess;

    if (mTimeLimit > 0) {
        SetProcessAffinityMask(PROCESS_HANDLE,
                               static_cast<DWORD_PTR>(mTimeLimit));
    }

    if (mMemoryLimit > 0) {
        SetProcessWorkingSetSize(PROCESS_HANDLE,
                                 static_cast<SIZE_T>(mMemoryLimit),
                                 static_cast<SIZE_T>(mMemoryLimit));
    }

    return true;
}
#else
auto SandboxImpl::setUnixLimits() -> bool {
    if (mTimeLimit > 0) {
        rlimit limit{.rlim_cur = static_cast<rlim_t>(mTimeLimit) / 1000,
                     .rlim_max = static_cast<rlim_t>(mTimeLimit) / 1000};
        if (setrlimit(RLIMIT_CPU, &limit) != 0) {
            return false;
        }
    }

    if (mMemoryLimit > 0) {
        rlimit limit{.rlim_cur = static_cast<rlim_t>(mMemoryLimit) * 1024,
                     .rlim_max = static_cast<rlim_t>(mMemoryLimit) * 1024};
        if (setrlimit(RLIMIT_AS, &limit) != 0) {
            return false;
        }
    }

    return true;
}
#endif

auto SandboxImpl::run() -> bool {
#ifdef _WIN32
    STARTUPINFO startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    PROCESS_INFORMATION processInfo{};

    std::string commandLine = mProgramPath;
    for (const auto& arg : mProgramArgs) {
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

    mTimeUsed = GetTickCount();
    mMemoryUsed = static_cast<long>(memoryCounters.WorkingSetSize / 1024);

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
        args.reserve(mProgramArgs.size() + 2);
        args.emplace_back(mProgramPath.data());
        for (const auto& arg : mProgramArgs) {
            args.emplace_back(const_cast<char*>(arg.c_str()));
        }
        args.emplace_back(nullptr);

        if (!setUnixLimits()) {
            exit(1);
        }

        execvp(mProgramPath.c_str(), args.data());
        exit(0);
    } else {
        int status = 0;
        rusage usage{};
        while (wait4(PID, &status, 0, &usage) != PID) {
            ;
        }
        mTimeUsed = static_cast<int>(usage.ru_utime.tv_sec * 1000 +
                                     usage.ru_utime.tv_usec / 1000);
        mMemoryUsed = static_cast<long>(usage.ru_maxrss);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
#endif
}

}  // namespace lithium
