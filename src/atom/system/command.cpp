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
// clang-format off
#include <windows.h>
#include <conio.h>
#include <tlhelp32.h>
// clang-format on
#else
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#include <cstdio>
#include <cstring>
#define SETENV(name, value) setenv(name, value, 1)
#define UNSETENV(name) unsetenv(name)
#endif

#include "atom/error/exception.hpp"
#include "atom/system/process.hpp"
#include "atom/utils/convert.hpp"
#include "atom/utils/string.hpp"
#include "atom/utils/to_string.hpp"

namespace atom::system {
std::string executeCommandInternal(
    const std::string &command, bool openTerminal,
    std::function<void(const std::string &)> processLine, int &status,
    const std::string &username = "", const std::string &domain = "",
    const std::string &password = "") {
    if (command.empty()) {
        status = -1;
        return "";
    }

    auto pipeDeleter = [](FILE *pipe) {
        if (pipe)
            pclose(pipe);
    };

    std::unique_ptr<FILE, decltype(pipeDeleter)> pipe(nullptr, pipeDeleter);

    if (!username.empty() && !domain.empty() && !password.empty()) {
        if (!_CreateProcessAsUser(command, username, domain, password)) {
            THROW_RUNTIME_ERROR("Error: failed to run command '" + command +
                                "' as user.");
        }
        status = 0;
        return "";
    }

#ifdef _WIN32
    if (openTerminal) {
        STARTUPINFOW startupInfo{};
        PROCESS_INFORMATION processInfo{};
        if (CreateProcessW(NULL, atom::utils::StringToLPWSTR(command), NULL,
                           NULL, FALSE, 0, NULL, NULL, &startupInfo,
                           &processInfo)) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
            status = 0;
            return "";  // 因为终端界面会在新进程中执行，无法获得输出，所以这里返回空字符串。
        } else {
            THROW_RUNTIME_ERROR("Error: failed to run command '" + command +
                                "'.");
        }
    } else {
        pipe.reset(_popen(command.c_str(), "r"));
    }
#else  // 非Windows平台
    pipe.reset(popen(command.c_str(), "r"));
#endif

    if (!pipe) {
        THROW_RUNTIME_ERROR("Error: failed to run command '" + command + "'.");
    }

    std::array<char, 4096> buffer{};
    std::ostringstream output;

    bool interrupted = false;

#ifdef _WIN32
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr &&
           !interrupted) {
        output << buffer.data();

        if (_kbhit()) {
            int key = _getch();
            if (key == 3) {
                interrupted = true;
            }
        }

        if (processLine) {
            processLine(buffer.data());
        }
    }
#else
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr &&
           !interrupted) {
        std::string line = buffer.data();
        output << line;
        if (processLine) {
            processLine(line);
        }
    }
#endif

#ifdef _WIN32
    status = 0;
#else
    status = WEXITSTATUS(pclose(pipe.get()));
#endif

    return output.str();
}

std::string executeCommand(
    const std::string &command, bool openTerminal,
    std::function<void(const std::string &)> processLine) {
    int status = 0;
    return executeCommandInternal(command, openTerminal, processLine, status);
}

std::pair<std::string, int> executeCommandWithStatus(
    const std::string &command) {
    int status = 0;
    std::string output =
        executeCommandInternal(command, false, nullptr, status);
    return {output, status};
}

void executeCommands(const std::vector<std::string> &commands) {
    std::vector<std::thread> threads;
    std::vector<std::string> errors;

    for (const auto &command : commands) {
        threads.emplace_back([&command, &errors]() {
            try {
                int status = 0;
                std::string output = executeCommand(command, false, nullptr);
                if (status != 0) {
                    THROW_RUNTIME_ERROR("Error executing command: " + command);
                }
            } catch (const std::runtime_error &e) {
                errors.push_back(e.what());
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    if (!errors.empty()) {
        THROW_RUNTIME_ERROR("One or more commands failed." + toString(errors));
    }
}

std::string executeCommandWithEnv(
    const std::string &command,
    const std::unordered_map<std::string, std::string> &envVars) {
    if (command.empty()) {
        return "";
    }
    std::unordered_map<std::string, std::string> oldEnvVars;
    for (const auto &var : envVars) {
        char *oldValue = std::getenv(var.first.c_str());
        if (oldValue) {
            oldEnvVars[var.first] = oldValue;
        }
        SETENV(var.first.c_str(), var.second.c_str());
    }

    auto result = executeCommand(command, false);

    for (const auto &var : envVars) {
        if (oldEnvVars.find(var.first) != oldEnvVars.end()) {
            SETENV(var.first.c_str(), oldEnvVars[var.first].c_str());
        } else {
            UNSETENV(var.first.c_str());
        }
    }

    return result;
}

void killProcessByName(const std::string &processName,
                       [[maybe_unused]] int signal) {
#ifdef _WIN32
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        THROW_SYSTEM_COLLAPSE("Error: unable to create toolhelp snapshot.");
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32FirstW(snap, &entry)) {
        CloseHandle(snap);
        THROW_SYSTEM_COLLAPSE("Error: unable to get the first process.");
    }

    do {
        if (strcmp(atom::utils::WCharArrayToString(entry.szExeFile).c_str(),
                   processName.c_str()) == 0) {
            HANDLE hProcess =
                OpenProcess(PROCESS_TERMINATE, 0, entry.th32ProcessID);
            if (hProcess != NULL) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
        }
    } while (Process32NextW(snap, &entry));

    CloseHandle(snap);
#else
    std::string command =
        "pkill -" + std::to_string(signal) + " -f " + processName;
    int result = system(command.c_str());
    if (result != 0) {
        THROW_SYSTEM_COLLAPSE("Error: failed to kill process with name " +
                              processName);
    }
#endif
}

void killProcessByPID(int pid, [[maybe_unused]] int signal) {
#ifdef _WIN32
    HANDLE hProcess =
        OpenProcess(PROCESS_TERMINATE, 0, static_cast<DWORD>(pid));
    if (hProcess == NULL) {
        THROW_SYSTEM_COLLAPSE("Error: unable to open process with PID " +
                              std::to_string(pid));
    }
    TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
#else
    if (kill(pid, signal) == -1) {
        THROW_SYSTEM_COLLAPSE("Error: failed to kill process with PID " +
                              std::to_string(pid));
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
#endif
}

}  // namespace atom::system
