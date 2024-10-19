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
#include <future>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>
#include "env.hpp"

#ifdef _WIN32
#define SETENV(name, value) SetEnvironmentVariableA(name, value)
#define UNSETENV(name) SetEnvironmentVariableA(name, nullptr)
// clang-format off
#include <windows.h>
#include <conio.h>
#include <tlhelp32.h>
// clang-format on
#else
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#define SETENV(name, value) setenv(name, value, 1)
#define UNSETENV(name) unsetenv(name)
#endif

#include "macro.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/process.hpp"
#include "atom/utils/to_string.hpp"

#ifdef _WIN32
#include "atom/utils/convert.hpp"
#endif

namespace atom::system {

std::mutex envMutex;

auto executeCommandInternal(
    const std::string &command, [[maybe_unused]] bool openTerminal,
    const std::function<void(const std::string &)> &processLine, int &status,
    const std::string &input = "",  // 新增input参数
    const std::string &username = "", const std::string &domain = "",
    const std::string &password = "") -> std::string {
    LOG_F(INFO,
          "executeCommandInternal called with command: {}, openTerminal: {}, "
          "input: {}, username: {}, domain: {}, password: [hidden]",
          command, openTerminal, input, username, domain);

    if (command.empty()) {
        status = -1;
        LOG_F(ERROR, "Command is empty");
        return "";
    }

    auto pipeDeleter = [](FILE *pipe) {
        if (pipe != nullptr) {
#ifdef _MSC_VER
            _pclose(pipe);
#else
            pclose(pipe);
#endif
        }
    };

    std::unique_ptr<FILE, decltype(pipeDeleter)> pipe(nullptr, pipeDeleter);

    if (!username.empty() && !domain.empty() && !password.empty()) {
        if (!_CreateProcessAsUser(command, username, domain, password)) {
            LOG_F(ERROR, "Failed to run command '{}' as user '{}\\{}'.",
                  command, domain, username);
            THROW_RUNTIME_ERROR(std::format(
                "Error: failed to run command '{}' as user '{}\\{}'.", command,
                domain, username));
        }
        status = 0;
        LOG_F(INFO, "Command '{}' executed as user '{}\\{}'.", command, domain,
              username);
        return "";
    }

#ifdef _WIN32
    if (openTerminal) {
        STARTUPINFOW startupInfo{};
        PROCESS_INFORMATION processInfo{};
        if (CreateProcessW(nullptr, atom::utils::StringToLPWSTR(command),
                           nullptr, nullptr, FALSE, 0, nullptr, nullptr,
                           &startupInfo, &processInfo) != 0) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
            status = 0;
            LOG_F(INFO, "Command '{}' executed in terminal.", command);
            return "";
        }
        LOG_F(ERROR, "Failed to run command '{}' in terminal.", command);
        THROW_FAIL_TO_CREATE_PROCESS(std::format(
            "Error: failed to run command '{}' in terminal.", command));
    }
    pipe.reset(_popen(command.c_str(), "w"));
#else  // 非Windows平台
    pipe.reset(popen(command.c_str(), "w"));
#endif

    if (!pipe) {
        LOG_F(ERROR, "Failed to run command '{}'.", command);
        THROW_FAIL_TO_CREATE_PROCESS(
            std::format("Error: failed to run command '{}'.", command));
    }

    // 写入输入
    if (!input.empty()) {
        if (fwrite(input.c_str(), sizeof(char), input.size(), pipe.get()) !=
            input.size()) {
            LOG_F(ERROR, "Failed to write input to pipe for command '{}'.",
                  command);
            THROW_RUNTIME_ERROR(std::format(
                "Error: failed to write input to pipe for command '{}'.",
                command));
        }
        if (fflush(pipe.get()) != 0) {
            LOG_F(ERROR, "Failed to flush pipe for command '{}'.", command);
            THROW_RUNTIME_ERROR(std::format(
                "Error: failed to flush pipe for command '{}'.", command));
        }
    }

    constexpr std::size_t BUFFER_SIZE = 4096;
    std::array<char, BUFFER_SIZE> buffer{};
    std::ostringstream output;

    bool interrupted = false;

#ifdef _WIN32
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr &&
           !interrupted) {
        output << buffer.data();

        if (_kbhit() != 0) {
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
    while (!interrupted) {
#pragma unroll
        for (int i = 0; i < 4; ++i) {
            if (fgets(buffer.data(), buffer.size(), pipe.get()) == nullptr) {
                break;
            }
            std::string line = buffer.data();
            output << line;
            if (processLine) {
                processLine(line);
            }
        }
    }
#endif

#ifdef _WIN32
    status = 0;
#else
    status = WEXITSTATUS(pclose(pipe.get()));
#endif
    LOG_F(INFO, "Command '{}' executed with status: {}", command, status);
    return output.str();
}

auto executeCommandStream(
    const std::string &command, [[maybe_unused]] bool openTerminal,
    const std::function<void(const std::string &)> &processLine, int &status,
    const std::function<bool()> &terminateCondition) -> std::string {
    LOG_F(INFO,
          "executeCommandStream called with command: {}, openTerminal: {}",
          command, openTerminal);

    if (command.empty()) {
        status = -1;
        LOG_F(ERROR, "Command is empty");
        return "";
    }

    auto pipeDeleter = [](FILE *pipe) {
        if (pipe != nullptr) {
#ifdef _MSC_VER
            _pclose(pipe);
#else
            pclose(pipe);
#endif
        }
    };

    std::unique_ptr<FILE, decltype(pipeDeleter)> pipe(nullptr, pipeDeleter);

#ifdef _WIN32
    if (openTerminal) {
        STARTUPINFO startupInfo{};
        PROCESS_INFORMATION processInfo{};
        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        ZeroMemory(&processInfo, sizeof(processInfo));

        if (CreateProcess(nullptr, const_cast<LPSTR>(command.c_str()), nullptr,
                          nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr,
                          &startupInfo, &processInfo)) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
            status = 0;
            LOG_F(INFO, "Command '{}' executed in terminal.", command);
            return "";  // Since terminal window will execute in new process, we
                        // can't get output here.
        }
        LOG_F(ERROR, "Failed to run command '{}' in terminal.", command);
        THROW_FAIL_TO_CREATE_PROCESS(std::format(
            "Error: failed to run command '{}' in terminal.", command));
    }
    pipe.reset(_popen(command.c_str(), "r"));
#else
    pipe.reset(popen(command.c_str(), "r"));
#endif

    if (!pipe) {
        LOG_F(ERROR, "Failed to run command '{}'.", command);
        THROW_FAIL_TO_CREATE_PROCESS(
            std::format("Error: failed to run command '{}'.", command));
    }

    constexpr std::size_t BUFFER_SIZE = 4096;
    std::array<char, BUFFER_SIZE> buffer{};
    std::ostringstream output;

    std::promise<void> exitSignal;
    std::future<void> futureObj = exitSignal.get_future();
    std::atomic<bool> stopReading{false};

    std::thread readerThread(
        [&pipe, &buffer, &output, &processLine, &futureObj, &stopReading]() {
#pragma GCC unroll 4
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                if (stopReading) {
                    break;
                }

                std::string line = buffer.data();
                output << line;
                if (processLine) {
                    processLine(line);
                }

                if (futureObj.wait_for(std::chrono::milliseconds(1)) !=
                    std::future_status::timeout) {
                    break;
                }
            }
        });

    // Monitor for termination condition
    while (!terminateCondition()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    stopReading = true;
    exitSignal.set_value();

    if (readerThread.joinable()) {
        readerThread.join();
    }

#ifdef _WIN32
    status = _pclose(pipe.release());
#else
    status =
        WEXITSTATUS(pclose(pipe.release()));  // Release ownership to ensure the
                                              // pipe is closed correctly
#endif

    LOG_F(INFO, "Command '{}' executed with status: {}", command, status);
    return output.str();
}

auto executeCommand(const std::string &command, bool openTerminal,
                    const std::function<void(const std::string &)> &processLine)
    -> std::string {
    LOG_F(INFO, "executeCommand called with command: {}, openTerminal: {}",
          command, openTerminal);
    int status = 0;
    auto result =
        executeCommandInternal(command, openTerminal, processLine, status);
    LOG_F(INFO, "executeCommand completed with status: {}", status);
    return result;
}

auto executeCommandWithStatus(const std::string &command)
    -> std::pair<std::string, int> {
    LOG_F(INFO, "executeCommandWithStatus called with command: {}", command);
    int status = 0;
    std::string output =
        executeCommandInternal(command, false, nullptr, status);
    LOG_F(INFO, "executeCommandWithStatus completed with status: {}", status);
    return {output, status};
}

auto executeCommandWithInput(const std::string &command,
                             const std::string &input,
                             const std::function<void(const std::string &)>
                                 &processLine) -> std::string {
    LOG_F(INFO, "executeCommandWithInput called with command: {}, input: {}",
          command, input);
    int status = 0;
    auto result = executeCommandInternal(command, /*openTerminal=*/false,
                                         processLine, status, input);
    LOG_F(INFO, "executeCommandWithInput completed with status: {}", status);
    return result;
}

void executeCommands(const std::vector<std::string> &commands) {
    LOG_F(INFO, "executeCommands called with {} commands", commands.size());
    std::vector<std::thread> threads;
    std::vector<std::string> errors;

    threads.reserve(commands.size());
    for (const auto &command : commands) {
        threads.emplace_back([&command, &errors]() {
            try {
                int status = 0;
                std::string output = executeCommand(command, false, nullptr);
                if (status != 0) {
                    THROW_RUNTIME_ERROR("Error executing command: " + command);
                }
            } catch (const std::runtime_error &e) {
                errors.emplace_back(e.what());
            }
        });
    }

    for (auto &thread : threads) {
        thread.join();
    }

    if (!errors.empty()) {
        THROW_INVALID_ARGUMENT("One or more commands failed." +
                               atom::utils::toString(errors));
    }
    LOG_F(INFO, "executeCommands completed");
}

auto executeCommandWithEnv(const std::string &command,
                           const std::unordered_map<std::string, std::string>
                               &envVars) -> std::string {
    LOG_F(INFO, "executeCommandWithEnv called with command: {}", command);
    if (command.empty()) {
        LOG_F(WARNING, "Command is empty");
        return "";
    }

    std::unordered_map<std::string, std::string> oldEnvVars;

    {
        // Lock the mutex to ensure thread safety
        std::lock_guard lock(envMutex);

        for (const auto &var : envVars) {
            std::lock_guard lock(envMutex);
            std::shared_ptr<utils::Env> env;
            GET_OR_CREATE_PTR(env, utils::Env, "LITHIUM.ENV");
            auto oldValue = env->getEnv(var.first);
            if (!oldValue.empty()) {
                oldEnvVars[var.first] = std::string(oldValue);
            }
            SETENV(var.first.c_str(), var.second.c_str());
        }
    }

    auto result = executeCommand(command, false);

    {
        // Lock the mutex to ensure thread safety
        std::lock_guard lock(envMutex);

        for (const auto &var : envVars) {
            if (oldEnvVars.find(var.first) != oldEnvVars.end()) {
                SETENV(var.first.c_str(), oldEnvVars[var.first].c_str());
            } else {
                UNSETENV(var.first.c_str());
            }
        }
    }

    LOG_F(INFO, "executeCommandWithEnv completed");
    return result;
}

auto executeCommandSimple(const std::string &command) -> bool {
    LOG_F(INFO, "executeCommandSimple called with command: {}", command);
    auto result = executeCommandWithStatus(command).second == 0;
    LOG_F(INFO, "executeCommandSimple completed with result: {}", result);
    return result;
}

void killProcessByName(const std::string &processName, ATOM_UNUSED int signal) {
    LOG_F(INFO, "killProcessByName called with processName: {}, signal: {}",
          processName, signal);
#ifdef _WIN32
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Error: unable to create toolhelp snapshot.");
        THROW_SYSTEM_COLLAPSE("Error: unable to create toolhelp snapshot.");
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32FirstW(snap, &entry) == 0) {
        CloseHandle(snap);
        LOG_F(ERROR, "Error: unable to get the first process.");
        THROW_SYSTEM_COLLAPSE("Error: unable to get the first process.");
    }

    do {
        if (strcmp(atom::utils::WCharArrayToString(entry.szExeFile).c_str(),
                   processName.c_str()) == 0) {
            HANDLE hProcess =
                OpenProcess(PROCESS_TERMINATE, 0, entry.th32ProcessID);
            if (hProcess != nullptr) {
                TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
                LOG_F(INFO, "Process '{}' terminated.", processName);
            }
        }
    } while (Process32NextW(snap, &entry) != 0);

    CloseHandle(snap);
#else
    std::string command =
        "pkill -" + std::to_string(signal) + " -f " + processName;
    int result = std::system(command.c_str());
    if (result != 0) {
        LOG_F(ERROR, "Error: failed to kill process with name {}", processName);
        THROW_SYSTEM_COLLAPSE("Error: failed to kill process with name " +
                              processName);
    }
    LOG_F(INFO, "Process '{}' terminated with signal {}.", processName, signal);
#endif
}

void killProcessByPID(int pid, ATOM_UNUSED int signal) {
    LOG_F(INFO, "killProcessByPID called with pid: {}, signal: {}", pid,
          signal);
#ifdef _WIN32
    HANDLE hProcess =
        OpenProcess(PROCESS_TERMINATE, 0, static_cast<DWORD>(pid));
    if (hProcess == nullptr) {
        LOG_F(ERROR, "Error: unable to open process with PID {}", pid);
        THROW_SYSTEM_COLLAPSE("Error: unable to open process with PID " +
                              std::to_string(pid));
    }
    TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);
    LOG_F(INFO, "Process with PID {} terminated.", pid);
#else
    if (kill(pid, signal) == -1) {
        LOG_F(ERROR, "Error: failed to kill process with PID {}", pid);
        THROW_SYSTEM_COLLAPSE("Error: failed to kill process with PID " +
                              std::to_string(pid));
    }
    int status;
    waitpid(pid, &status, 0);
    LOG_F(INFO, "Process with PID {} terminated with signal {}.", pid, signal);
#endif
}

}  // namespace atom::system
