/*
 * daemon.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-11

Description: Daemon process implementation for Linux and Windows. But there is
still some problems on Windows, especially the console.

**************************************************/

#include "daemon.hpp"

#include <fstream>
#include <ostream>
#include <sstream>
#include <thread>
#include "macro.hpp"

#ifndef _WIN32
#include <unistd.h>
#endif

#include "atom/log/loguru.hpp"
#include "atom/utils/time.hpp"

// 定义 g_daemonRestartInterval 变量
int gDaemonRestartInterval = 10;

// 定义 g_pidFilePath 变量
std::string gPidFilePath = "lithium-daemon";

// 定义 g_isDaemon 变量
bool gIsDaemon = false;

namespace atom::async {
auto DaemonGuard::toString() const -> std::string {
    std::stringstream ss;
    ss << "[DaemonGuard parentId=" << m_parentId << " mainId=" << m_mainId
       << " parentStartTime=" << utils::timeStampToString(m_parentStartTime)
       << " mainStartTime=" << utils::timeStampToString(m_mainStartTime)
       << " restartCount=" << m_restartCount.load() << "]";
    return ss.str();
}

auto DaemonGuard::realStart(
    int argc, char **argv,
    const std::function<int(int argc, char **argv)> &mainCb) -> int {
#ifdef _WIN32
    m_mainId = reinterpret_cast<HANDLE>(getpid());
#else
    m_mainId = getpid();
#endif
    m_mainStartTime = time(0);
    return mainCb(argc, argv);
}

auto DaemonGuard::realDaemon(
    int argc, char **argv,
    const std::function<int(int argc, char **argv)> &mainCb) -> int {
#ifdef _WIN32
    // 在 Windows 平台下模拟守护进程
    FreeConsole();
    m_parentId = reinterpret_cast<HANDLE>(GetCurrentProcessId());
    m_parentStartTime = time(0);
    while (true) {
        PROCESS_INFORMATION DaemonGuard;
        STARTUPINFO startupInfo;
        memset(&DaemonGuard, 0, sizeof(DaemonGuard));
        memset(&startupInfo, 0, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        if (!CreateProcess(nullptr, argv[0], nullptr, nullptr, FALSE,
                           CREATE_NEW_CONSOLE, nullptr, nullptr, &startupInfo,
                           &DaemonGuard)) {
            LOG_F(ERROR, "Create process failed with error code {}",
                  GetLastError());
            return -1;
        }
        WaitForSingleObject(DaemonGuard.hProcess, INFINITE);
        CloseHandle(DaemonGuard.hProcess);
        CloseHandle(DaemonGuard.hThread);

        // 等待一段时间后重新启动子进程
        m_restartCount++;
        Sleep(g_daemonRestartInterval * 1000);
    }
#else
    if (daemon(1, 0) == -1) {
        perror("daemon");
        exit(EXIT_FAILURE);
    }

    m_parentId = getpid();
    m_parentStartTime = time(0);
    while (true) {
        pid_t pid = fork();  // 创建子进程
        if (pid == 0) {      // 子进程
            m_mainId = getpid();
            m_mainStartTime = time(0);
            LOG_F(INFO, "daemon process start pid={}",
                  reinterpret_cast<int>(getpid()));
            return realStart(argc, argv, mainCb);
        }
        if (pid < 0) {  // 创建子进程失败
            LOG_F(ERROR, "fork fail return={} errno={} errstr={}", pid, errno,
                  strerror(errno));
            return -1;
        }  // 父进程
        int status = 0;
        waitpid(pid, &status, 0);  // 等待子进程退出

        // 子进程异常退出
        if (status != 0) {
            if (status == 9) {  // SIGKILL 信号杀死子进程，不需要重新启动
                LOG_F(INFO, "daemon process killed pid={}", getpid());
                break;
            }  // 记录日志并重新启动子进程
            LOG_F(ERROR, "child crash pid={} status={}", pid, status);

        } else {  // 正常退出，直接退出程序
            LOG_F(INFO, "daemon process exit pid={}", getpid());
            break;
        }

        // 等待一段时间后重新启动子进程
        m_restartCount++;
        sleep(gDaemonRestartInterval);
    }
#endif
    return 0;
}

// 启动进程，如果需要创建守护进程，则先创建守护进程
auto DaemonGuard::startDaemon(
    int argc, char **argv,
    const std::function<int(int argc, char **argv)> &mainCb,
    bool isDaemon) -> int {
#ifdef _WIN32
    if (isDaemon) {
        AllocConsole();
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif

    if (!isDaemon) {  // 不需要创建守护进程
#ifdef _WIN32
        m_parentId = reinterpret_cast<HANDLE>(getpid());
#else
        m_parentId = getpid();
#endif
        m_parentStartTime = time(0);
        return realStart(argc, argv, mainCb);
    }
    // 创建守护进程
    return realDaemon(argc, argv, mainCb);
}

void signalHandler(int signum) {
#ifdef _WIN32
    if (signum == SIGTERM || signum == SIGINT) {
        remove(g_pidFilePath.c_str());
        exit(0);
    }
#else
    if (signum == SIGTERM || signum == SIGINT) {
        ATOM_UNREF_PARAM(remove(gPidFilePath.c_str()));
        exit(0);
    }
#endif
}

void writePidFile() {
    std::ofstream ofs(gPidFilePath);
    if (!ofs) {
        LOG_F(ERROR, "open pid file {} failed", gPidFilePath);
        exit(-1);
    }
    ofs << getpid();
    ofs.close();
}

// 检查 PID 文件是否存在，并检查文件中的 PID 是否有效
auto checkPidFile() -> bool {
#ifdef _WIN32
    // Windows 平台下不检查 PID 文件是否存在以及文件中的 PID 是否有效
    return false;
#else
    struct stat st {};
    if (stat(gPidFilePath.c_str(), &st) != 0) {
        return false;
    }
    std::ifstream ifs(gPidFilePath);
    if (!ifs) {
        return false;
    }
    pid_t pid = -1;
    ifs >> pid;
    ifs.close();
    return kill(pid, 0) != -1 || errno != ESRCH;
#endif
}
}  // namespace atom::async
