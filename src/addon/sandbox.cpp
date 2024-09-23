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
#include <fcntl.h>
#include <seccomp.h>
#include <sys/ptrace.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "atom/log/loguru.hpp"

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
    bool applySeccomp();
#endif
};

Sandbox::Sandbox() : pimpl(std::make_unique<SandboxImpl>()) {
    LOG_F(INFO, "Sandbox created");
}

Sandbox::~Sandbox() { LOG_F(INFO, "Sandbox destroyed"); }

auto Sandbox::setTimeLimit(int timeLimitMs) -> bool {
    LOG_F(INFO, "Setting time limit to {} ms", timeLimitMs);
    return pimpl->setTimeLimit(timeLimitMs);
}

auto Sandbox::setMemoryLimit(long memoryLimitKb) -> bool {
    LOG_F(INFO, "Setting memory limit to {} KB", memoryLimitKb);
    return pimpl->setMemoryLimit(memoryLimitKb);
}

auto Sandbox::setRootDirectory(const std::string& rootDirectory) -> bool {
    LOG_F(INFO, "Setting root directory to {}", rootDirectory);
    return pimpl->setRootDirectory(rootDirectory);
}

auto Sandbox::setUserId(int userId) -> bool {
    LOG_F(INFO, "Setting user ID to {}", userId);
    return pimpl->setUserId(userId);
}

auto Sandbox::setProgramPath(const std::string& programPath) -> bool {
    LOG_F(INFO, "Setting program path to {}", programPath);
    return pimpl->setProgramPath(programPath);
}

auto Sandbox::setProgramArgs(const std::vector<std::string>& programArgs)
    -> bool {
    LOG_F(INFO, "Setting program arguments");
    for (const auto& arg : programArgs) {
        LOG_F(INFO, "Arg: {}", arg);
    }
    return pimpl->setProgramArgs(programArgs);
}

auto Sandbox::run() -> bool {
    LOG_F(INFO, "Running sandbox");
    return pimpl->run();
}

auto Sandbox::getTimeUsed() const -> int {
    LOG_F(INFO, "Getting time used: {} ms", pimpl->mTimeUsed);
    return pimpl->mTimeUsed;
}

auto Sandbox::getMemoryUsed() const -> long {
    LOG_F(INFO, "Getting memory used: {} KB", pimpl->mMemoryUsed);
    return pimpl->mMemoryUsed;
}

auto SandboxImpl::setTimeLimit(int timeLimitMs) -> bool {
    mTimeLimit = timeLimitMs;
#ifndef _WIN32
    rlimit limit{.rlim_cur = static_cast<rlim_t>(timeLimitMs) / 1000,
                 .rlim_max = static_cast<rlim_t>(timeLimitMs) / 1000};
    bool result = setrlimit(RLIMIT_CPU, &limit) == 0;
    LOG_F(INFO, "Set time limit result: {}", result);
    return result;
#endif
    return true;
}

auto SandboxImpl::setMemoryLimit(long memoryLimitKb) -> bool {
    mMemoryLimit = memoryLimitKb;
#ifndef _WIN32
    rlimit limit{.rlim_cur = static_cast<rlim_t>(memoryLimitKb) * 1024,
                 .rlim_max = static_cast<rlim_t>(memoryLimitKb) * 1024};
    bool result = setrlimit(RLIMIT_AS, &limit) == 0;
    LOG_F(INFO, "Set memory limit result: {}", result);
    return result;
#endif
    return true;
}

auto SandboxImpl::setRootDirectory(const std::string& rootDirectory) -> bool {
    mRootDirectory = rootDirectory;
#ifndef _WIN32
    bool result = (chdir(rootDirectory.c_str()) == 0) &&
                  (chroot(rootDirectory.c_str()) == 0);
    LOG_F(INFO, "Set root directory result: {}", result);
    return result;
#endif
    return true;
}

auto SandboxImpl::setUserId(int userId) -> bool {
    mUserId = userId;
#ifndef _WIN32
    bool result = (setuid(userId) == 0) && (setgid(userId) == 0);
    LOG_F(INFO, "Set user ID result: {}", result);
    return result;
#endif
    return true;
}

auto SandboxImpl::setProgramPath(const std::string& programPath) -> bool {
    mProgramPath = programPath;
    LOG_F(INFO, "Program path set to {}", programPath);
    return true;
}

auto SandboxImpl::setProgramArgs(const std::vector<std::string>& programArgs)
    -> bool {
    mProgramArgs = programArgs;
    LOG_F(INFO, "Program arguments set");
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

    LOG_F(INFO, "Windows limits set");
    return true;
}
#else
auto SandboxImpl::setUnixLimits() -> bool {
    if (mTimeLimit > 0) {
        rlimit limit{.rlim_cur = static_cast<rlim_t>(mTimeLimit) / 1000,
                     .rlim_max = static_cast<rlim_t>(mTimeLimit) / 1000};
        if (setrlimit(RLIMIT_CPU, &limit) != 0) {
            LOG_F(ERROR, "Failed to set CPU time limit");
            return false;
        }
    }

    if (mMemoryLimit > 0) {
        rlimit limit{.rlim_cur = static_cast<rlim_t>(mMemoryLimit) * 1024,
                     .rlim_max = static_cast<rlim_t>(mMemoryLimit) * 1024};
        if (setrlimit(RLIMIT_AS, &limit) != 0) {
            LOG_F(ERROR, "Failed to set memory limit");
            return false;
        }
    }

    LOG_F(INFO, "Unix limits set");
    return true;
}

auto SandboxImpl::applySeccomp() -> bool {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_KILL);  // Default action: kill
    if (ctx == nullptr) {
        LOG_F(ERROR, "Failed to initialize seccomp");
        return false;
    }

    // Allow necessary syscalls
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(munmap), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 1,
                     SCMP_A1(SCMP_CMP_MASKED_EQ, O_WRONLY, O_WRONLY));
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
    seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);

    // Load the filter
    if (seccomp_load(ctx) != 0) {
        seccomp_release(ctx);
        LOG_F(ERROR, "Failed to load seccomp filter");
        return false;
    }

    seccomp_release(ctx);
    LOG_F(INFO, "Seccomp filter applied");
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
        LOG_F(ERROR, "Failed to create process");
        return false;
    }

    if (!setWindowsLimits(processInfo)) {
        LOG_F(ERROR, "Failed to set Windows limits");
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

    LOG_F(INFO, "Process finished with exit code {}", exitCode);
    return exitCode == 0;
#else
    const pid_t PID = fork();
    if (PID < 0) {
        LOG_F(ERROR, "Failed to fork process");
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
            LOG_F(ERROR, "Failed to set Unix limits");
            exit(1);
        }

        if (!applySeccomp()) {
            LOG_F(ERROR, "Failed to apply seccomp");
            exit(1);
        }

        execvp(mProgramPath.c_str(), args.data());
        LOG_F(ERROR, "Failed to exec program");
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
        LOG_F(INFO, "Process finished with status {}", status);
        return WIFEXITED(status) && WEXITSTATUS(status) == 0;
    }
#endif
}

}  // namespace lithium