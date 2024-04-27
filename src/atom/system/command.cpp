/*
 * command.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-24

Description: Simple wrapper for executing commands.

**************************************************/

#include "command.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>
#ifdef _WIN32
#define SETENV(name, value) SetEnvironmentVariableA(name, value)
#define UNSETENV(name) SetEnvironmentVariableA(name, NULL)
#include <conio.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstring>
#define SETENV(name, value) setenv(name, value, 1)
#define UNSETENV(name) unsetenv(name)
#endif

namespace Atom::System {
std::string executeCommand(
    const std::string &command, bool openTerminal,
    std::function<void(const std::string &)> processLine) {
    if (command.empty()) {
        return "";
    }

    auto pipeDeleter = [](FILE *pipe) {
        if (pipe)
            pclose(pipe);
    };

    std::unique_ptr<FILE, decltype(pipeDeleter)> pipe(nullptr, pipeDeleter);

#ifdef _WIN32
    if (openTerminal) {
        // 在Windows下打开终端界面
        STARTUPINFO startupInfo{};
        PROCESS_INFORMATION processInfo{};
        if (CreateProcess(NULL, const_cast<char *>(command.c_str()), NULL, NULL,
                          FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
            return "";  // 因为终端界面会在新进程中执行，无法获得输出，所以这里返回空字符串。
        } else {
            throw std::runtime_error("Error: failed to run command '" +
                                     command + "'.");
        }
    } else {
        // 不打开终端界面时，使用_popen执行命令
        pipe.reset(_popen(command.c_str(), "r"));
    }
#else  // 非Windows平台
    pipe.reset(popen(command.c_str(), "r"));
#endif

    if (!pipe) {
        throw std::runtime_error("Error: failed to run command '" + command +
                                 "'.");
    }

    std::array<char, 4096> buffer{};
    std::ostringstream output;

    bool interrupted = false;  // 标记是否收到中断信号

    auto start = std::chrono::steady_clock::now();  // 记录开始时间

#ifdef _WIN32
    // Windows下无法捕获中断信号，因此只能在循环中检查键盘输入来模拟中断
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr &&
           !interrupted) {
        output << buffer.data();

        if (_kbhit()) {
            int key = _getch();
            if (key == 3)  // 检查Ctrl+C中断信号
            {
                interrupted = true;
            }
        }

        if (processLine) {
            processLine(buffer.data());
        }
    }
#else  // 非Windows平台

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr &&
           !interrupted) {
        std::string line = buffer.data();
        output << line;
        if (processLine) {
            processLine(line);
        }
    }
#endif

    return output.str();
}

void executeCommands(const std::vector<std::string> &commands) {
    std::vector<std::thread> threads;

    for (const auto &command : commands) {
        threads.emplace_back([command]() {
#ifdef _WIN32
            STARTUPINFO startupInfo{};
            PROCESS_INFORMATION processInfo{};
            if (CreateProcess(NULL, const_cast<char *>(command.c_str()), NULL,
                              NULL, FALSE, 0, NULL, NULL, &startupInfo,
                              &processInfo)) {
                WaitForSingleObject(processInfo.hProcess, INFINITE);
                CloseHandle(processInfo.hProcess);
                CloseHandle(processInfo.hThread);
            }
#else  // 非Windows平台
            int status = system(command.c_str());
            if (!WIFEXITED(status) || !(WEXITSTATUS(status) == 0)) {
                throw std::runtime_error("Error executing command: " + command);
            }
#endif
        });
    }
    for (auto &thread : threads) {
        thread.join();
    }
}

ProcessHandle executeCommand(const std::string &command) {
    ProcessHandle handle;

#ifdef _WIN32
    STARTUPINFO startupInfo{};
    PROCESS_INFORMATION processInfo{};
    if (CreateProcess(NULL, const_cast<char *>(command.c_str()), NULL, NULL,
                      FALSE, 0, NULL, NULL, &startupInfo, &processInfo)) {
        CloseHandle(processInfo.hThread);
        handle.handle = processInfo.hProcess;
    } else {
        throw std::runtime_error("Error: failed to run command '" + command +
                                 "'.");
    }
#else  // 非Windows平台
    pid_t pid = fork();
    if (pid == -1) {
        throw std::runtime_error("Error: failed to fork process for command '" +
                                 command + "'.");
    } else if (pid == 0) {
        // 子进程
        execl("/bin/sh", "sh", "-c", command.c_str(), NULL);
        _exit(EXIT_FAILURE);
    } else {
        handle.pid = pid;
    }
#endif

    return handle;
}

void killProcess(const ProcessHandle &handle) {
#ifdef _WIN32
    if (!handle.handle)
#else
    if (!handle.pid)
#endif
    {
        return;
    }
#ifdef _WIN32
    TerminateProcess(handle.handle, 0);
    CloseHandle(handle.handle);
#else
    int status;
    if (kill(handle.pid, SIGKILL) == -1) {
        throw std::runtime_error("Error: failed to kill process with PID " +
                                 std::to_string(handle.pid) + ".");
    } else {
        waitpid(handle.pid, &status, 0);
    }
#endif
}

std::string executeCommandWithEnv(
    const std::string &command,
    const std::map<std::string, std::string> &envVars) {
    if (command.empty()) {
        return "";
    }
    // 保存当前环境变量的状态
    std::map<std::string, std::string> oldEnvVars;
    for (const auto &var : envVars) {
        char *oldValue = std::getenv(var.first.c_str());
        if (oldValue) {
            oldEnvVars[var.first] = oldValue;
        }
#if defined(_WIN32) || defined(_WIN64)
        SETENV(var.first.c_str(), var.second.c_str());
#else
        SETENV(var.first.c_str(), var.second.c_str());
#endif
    }

    // 执行命令
    auto result = executeCommand(command, false);

    // 清理：恢复环境变量到之前的状态
    for (const auto &var : envVars) {
        if (oldEnvVars.find(var.first) != oldEnvVars.end()) {
            // 恢复旧值
            SETENV(var.first.c_str(), oldEnvVars[var.first].c_str());
        } else {
            // 如果之前不存在，则删除
            UNSETENV(var.first.c_str());
        }
    }

    return result;
}

std::pair<std::string, int> executeCommandWithStatus(
    const std::string &command) {
    if (command.empty()) {
        return {"", -1};
    }
    std::array<char, 4096> buffer{};
    std::ostringstream output;

#ifdef _WIN32
    // Windows implementation using _popen and _pclose
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command.c_str(), "r"),
                                                   _pclose);
#else
    // Linux implementation using popen and pclose
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
#endif

    if (!pipe) {
        throw std::runtime_error("Error: failed to run command '" + command +
                                 "'.");
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        output << buffer.data();
    }

    int status = -1;
#ifdef _WIN32
    // Windows exit code retrieval
    if (pipe) {
        status = 0;
    }
#else
    // Linux exit code retrieval
    if (pipe) {
        status = WEXITSTATUS(pclose(pipe.get()));
    }
#endif

    return {output.str(), status};
}
}  // namespace Atom::System
