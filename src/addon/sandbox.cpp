/*
 * sandbox.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A sandbox for alone componnents, such as executables.

**************************************************/

#include "sandbox.hpp"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#endif

namespace Lithium
{
    bool Sandbox::setTimeLimit(int timeLimitMs)
    {
#ifdef _WIN32
        m_timeLimit = timeLimitMs;
#else
        struct rlimit limit;
        limit.rlim_cur = timeLimitMs / 1000;
        limit.rlim_max = timeLimitMs / 1000;
        if (setrlimit(RLIMIT_CPU, &limit) != 0)
        {
            return false;
        }
        m_timeLimit = timeLimitMs;
#endif
        return true;
    }

    bool Sandbox::setMemoryLimit(long memoryLimitKb)
    {
#ifdef _WIN32
        m_memoryLimit = memoryLimitKb;
#else
        struct rlimit limit;
        limit.rlim_cur = memoryLimitKb * 1024;
        limit.rlim_max = memoryLimitKb * 1024;
        if (setrlimit(RLIMIT_AS, &limit) != 0)
        {
            return false;
        }
        m_memoryLimit = memoryLimitKb;
#endif
        return true;
    }

    bool Sandbox::setRootDirectory(const std::string &rootDirectory)
    {
#ifdef _WIN32
        return false; // Not supported on Windows
#else
        if (chdir(rootDirectory.c_str()) != 0 ||
            chroot(rootDirectory.c_str()) != 0)
        {
            return false;
        }
        m_rootDirectory = rootDirectory;
        return true;
#endif
    }

    bool Sandbox::setUserId(int userId)
    {
#ifdef _WIN32
        return false; // Not supported on Windows
#else
        if (setuid(userId) != 0 || setgid(userId) != 0)
        {
            return false;
        }
        m_userId = userId;
        return true;
#endif
    }

    bool Sandbox::setProgramPath(const std::string &programPath)
    {
        m_programPath = programPath;
        return true;
    }

    bool Sandbox::setProgramArgs(const std::vector<std::string> &programArgs)
    {
        m_programArgs = programArgs;
        return true;
    }

    bool Sandbox::run()
    {
#ifdef _WIN32
        STARTUPINFO startupInfo;
        PROCESS_INFORMATION processInfo;
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        ZeroMemory(&processInfo, sizeof(processInfo));

        std::string commandLine = m_programPath + " ";
        for (auto &arg : m_programArgs)
        {
            commandLine += arg + " ";
        }

        BOOL success = CreateProcess(NULL, const_cast<char *>(commandLine.c_str()), NULL, NULL,
                                     FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo);
        if (!success)
        {
            return false;
        }

        HANDLE processHandle = processInfo.hProcess;

        // 设置时间和内存限制
        if (m_timeLimit > 0)
        {
            DWORD_PTR timeLimitMs = static_cast<DWORD_PTR>(m_timeLimit);
            success = SetProcessAffinityMask(processHandle, timeLimitMs);
            if (!success)
            {
                return false;
            }
        }

        if (m_memoryLimit > 0)
        {
            SIZE_T memoryLimitKb = static_cast<SIZE_T>(m_memoryLimit);
            success = SetProcessWorkingSetSize(processHandle, memoryLimitKb, memoryLimitKb);
            if (!success)
            {
                return false;
            }
        }

        // 恢复子进程并等待它完成
        ResumeThread(processInfo.hThread);
        WaitForSingleObject(processHandle, INFINITE);

        // 获取子进程的退出状态和资源使用情况
        DWORD exitCode = 0;
        GetExitCodeProcess(processHandle, &exitCode);

        PROCESS_MEMORY_COUNTERS memoryCounters;
        GetProcessMemoryInfo(processHandle, &memoryCounters, sizeof(memoryCounters));

        m_timeUsed = GetTickCount();
        m_memoryUsed = static_cast<long>(memoryCounters.WorkingSetSize / 1024);

        CloseHandle(processHandle);
        CloseHandle(processInfo.hThread);

        return exitCode == 0;
#else
        pid_t pid = fork();
        if (pid < 0)
        {
            return false;
        }
        else if (pid == 0)
        { // child process
            ptrace(PTRACE_TRACEME, 0, NULL, NULL);
            execvp(m_programPath.c_str(), const_cast<char **>(&m_programArgs[0]));
            exit(0);
        }
        else
        { // parent process
            int status = 0;
            struct rusage usage;
            while (wait4(pid, &status, 0, &usage) != pid)
            {
            }
            m_timeUsed = usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000;
            m_memoryUsed = usage.ru_maxrss;
            return WIFEXITED(status) && WEXITSTATUS(status) == 0;
        }
#endif
    }

    int Sandbox::getTimeUsed() const
    {
        return m_timeUsed;
    }

    long Sandbox::getMemoryUsed() const
    {
        return m_memoryUsed;
    }
}

/*
int main()
{
    std::vector<std::thread> threads;
    std::vector<int> timeUsedList;
    std::vector<long> memoryUsedList;

    int numThreads = 5; // 设置线程数量

    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([i, &timeUsedList, &memoryUsedList]()
                             {
      Sandbox sandbox;
      sandbox.setTimeLimit(10000); // 设置时间限制为1秒
      sandbox.setMemoryLimit(1024102); // 设置内存限制为1MB
      sandbox.setProgramPath("E:\\python3.10\\python.exe"); // 设置程序路径
      sandbox.setProgramArgs({}); // 设置程序参数
      if (sandbox.run()) {
        timeUsedList[i] = sandbox.getTimeUsed();
        memoryUsedList[i] = sandbox.getMemoryUsed();
      } });
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    // 打印每个线程的时间和内存使用情况
    for (int i = 0; i < numThreads; ++i)
    {
        std::cout << "Thread " << i << ": " << std::endl;
        std::cout << "Time used: " << timeUsedList[i] << "ms" << std::endl;
        std::cout << "Memory used: " << memoryUsedList[i] << "KB" << std::endl;
    }

    return 0;
}

*/
