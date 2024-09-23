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

constexpr int kDaemonRestartInterval = 10;
const std::string kPidFilePath = "lithium-daemon";

bool gIsDaemon = false;

namespace atom::async {
auto DaemonGuard::toString() const -> std::string {
    std::stringstream stringStream;
    stringStream << "[DaemonGuard parentId=" << m_parentId
                 << " mainId=" << m_mainId << " parentStartTime="
                 << utils::timeStampToString(m_parentStartTime)
                 << " mainStartTime="
                 << utils::timeStampToString(m_mainStartTime)
                 << " restartCount=" << m_restartCount.load() << "]";
    return stringStream.str();
}

auto DaemonGuard::realStart(int /*argc*/, char **argv,
                            const std::function<int(int, char **)> &mainCb)
    -> int {
#ifdef _WIN32
    m_mainId = reinterpret_cast<HANDLE>(static_cast<intptr_t>(getpid()));
#else
    m_mainId = getpid();
#endif
    m_mainStartTime = time(nullptr);
    return mainCb(0, argv);
}

auto DaemonGuard::realDaemon(int /*argc*/, char **argv,
                             const std::function<int(int, char **)> &mainCb)
    -> int {
#ifdef _WIN32
    // 在 Windows 平台下模拟守护进程
    FreeConsole();
    m_parentId =
        reinterpret_cast<HANDLE>(static_cast<intptr_t>(GetCurrentProcessId()));
    m_parentStartTime = time(nullptr);
    while (true) {
        PROCESS_INFORMATION processInfo;
        STARTUPINFO startupInfo;
        memset(&processInfo, 0, sizeof(processInfo));
        memset(&startupInfo, 0, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        if (!CreateProcess(nullptr, argv[0], nullptr, nullptr, FALSE,
                           CREATE_NEW_CONSOLE, nullptr, nullptr, &startupInfo,
                           &processInfo)) {
            LOG_F(ERROR, "Create process failed with error code {}",
                  GetLastError());
            return -1;
        }
        WaitForSingleObject(processInfo.hProcess, INFINITE);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);

        // 等待一段时间后重新启动子进程
        m_restartCount++;
        Sleep(kDaemonRestartInterval * 1000);
    }
#else
    if (daemon(1, 0) == -1) {
        perror("daemon");
        exit(EXIT_FAILURE);
    }

    m_parentId = getpid();
    m_parentStartTime = time(nullptr);
    while (true) {
        pid_t pid = fork();  // 创建子进程
        if (pid == 0) {      // 子进程
            m_mainId = getpid();
            m_mainStartTime = time(nullptr);
            LOG_F(INFO, "daemon process start pid={}",
                  reinterpret_cast<int>(getpid()));
            return realStart(0, argv, mainCb);
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
        sleep(kDaemonRestartInterval);
    }
#endif
    return 0;
}

// 启动进程，如果需要创建守护进程，则先创建守护进程
auto DaemonGuard::startDaemon(int argc, char **argv,
                              const std::function<int(int, char **)> &mainCb,
                              bool isDaemon) -> int {
#ifdef _WIN32
    if (isDaemon) {
        AllocConsole();
        if (!freopen("CONOUT$", "w", stdout)) {
            LOG_F(ERROR, "Failed to redirect stdout");
            return -1;
        }
        if (!freopen("CONOUT$", "w", stderr)) {
            LOG_F(ERROR, "Failed to redirect stderr");
            return -1;
        }
    }
#endif

    if (!isDaemon) {  // 不需要创建守护进程
#ifdef _WIN32
        m_parentId = reinterpret_cast<HANDLE>(static_cast<intptr_t>(getpid()));
#else
        m_parentId = getpid();
#endif
        m_parentStartTime = time(nullptr);
        return realStart(argc, argv, mainCb);
    }
    // 创建守护进程
    return realDaemon(argc, argv, mainCb);
}

void signalHandler(int signum) {
#ifdef _WIN32
    if (signum == SIGTERM || signum == SIGINT) {
        if (remove(kPidFilePath.c_str()) != 0) {
            LOG_F(ERROR, "Failed to remove PID file");
        }
        exit(0);
    }
#else
    if (signum == SIGTERM || signum == SIGINT) {
        ATOM_UNREF_PARAM(remove(kPidFilePath.c_str()));
        exit(0);
    }
#endif
}

void writePidFile() {
    std::ofstream ofs(kPidFilePath);
    if (!ofs) {
        LOG_F(ERROR, "open pid file {} failed", kPidFilePath);
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
    if (stat(kPidFilePath.c_str(), &st) != 0) {
        return false;
    }
    std::ifstream ifs(kPidFilePath);
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