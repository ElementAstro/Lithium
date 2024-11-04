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

#include "atom/macro.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/process.hpp"

#ifdef _WIN32
#include "atom/utils/convert.hpp"
#endif

namespace atom::system {

std::mutex envMutex;

auto executeCommandInternal(
    const std::string &command, bool openTerminal,
    const std::function<void(const std::string &)> &processLine, int &status,
    const std::string &input = "", const std::string &username = "",
    const std::string &domain = "",
    const std::string &password = "") -> std::string {
    LOG_F(INFO,
          "executeCommandInternal called with command: {}, openTerminal: {}, "
          "input: [hidden], username: {}, domain: {}, password: [hidden]",
          command, openTerminal, username, domain);

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
        if (!createProcessAsUser(command, username, domain, password)) {
            LOG_F(ERROR, "Failed to run command '{}' as user '{}\\{}'.",
                  command, domain, username);
            THROW_RUNTIME_ERROR("Failed to run command as user.");
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
        startupInfo.cb = sizeof(startupInfo);

        std::wstring commandW = atom::utils::StringToLPWSTR(command);
        if (CreateProcessW(nullptr, &commandW[0], nullptr, nullptr, FALSE, 0,
                           nullptr, nullptr, &startupInfo, &processInfo)) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
            status = 0;
            LOG_F(INFO, "Command '{}' executed in terminal.", command);
            return "";
        }
        LOG_F(ERROR, "Failed to run command '{}' in terminal.", command);
        THROW_FAIL_TO_CREATE_PROCESS("Failed to run command in terminal.");
    }
    pipe.reset(_popen(command.c_str(), "w"));
#else  // Non-Windows
    pipe.reset(popen(command.c_str(), "w"));
#endif

    if (!pipe) {
        LOG_F(ERROR, "Failed to run command '{}'.", command);
        THROW_FAIL_TO_CREATE_PROCESS("Failed to run command.");
    }

    // Write input if provided
    if (!input.empty()) {
        if (fwrite(input.c_str(), sizeof(char), input.size(), pipe.get()) !=
            input.size()) {
            LOG_F(ERROR, "Failed to write input to pipe for command '{}'.",
                  command);
            THROW_RUNTIME_ERROR("Failed to write input to pipe.");
        }
        if (fflush(pipe.get()) != 0) {
            LOG_F(ERROR, "Failed to flush pipe for command '{}'.", command);
            THROW_RUNTIME_ERROR("Failed to flush pipe.");
        }
    }

    constexpr std::size_t BUFFER_SIZE = 4096;
    std::array<char, BUFFER_SIZE> buffer{};
    std::ostringstream output;

    bool interrupted = false;

#ifdef _WIN32
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr &&
           !interrupted) {
        std::string line(buffer.data());
        output << line;

        if (_kbhit()) {
            int key = _getch();
            if (key == 3) {  // Ctrl+C
                interrupted = true;
            }
        }

        if (processLine) {
            processLine(line);
        }
    }
#else
    while (!interrupted &&
           fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());
        output << line;

        if (processLine) {
            processLine(line);
        }
    }
#endif

#ifdef _WIN32
    status = _pclose(pipe.release());
#else
    status = WEXITSTATUS(pclose(pipe.release()));
#endif
    LOG_F(INFO, "Command '{}' executed with status: {}", command, status);
    return output.str();
}

auto executeCommandStream(
    const std::string &command, bool openTerminal,
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
        startupInfo.cb = sizeof(startupInfo);

        std::wstring commandW = atom::utils::StringToLPWSTR(command);
        if (CreateProcessW(nullptr, &commandW[0], nullptr, nullptr, FALSE,
                           CREATE_NEW_CONSOLE, nullptr, nullptr, &startupInfo,
                           &processInfo)) {
            WaitForSingleObject(processInfo.hProcess, INFINITE);
            CloseHandle(processInfo.hProcess);
            CloseHandle(processInfo.hThread);
            status = 0;
            LOG_F(INFO, "Command '{}' executed in terminal.", command);
            return "";
        }
        LOG_F(ERROR, "Failed to run command '{}' in terminal.", command);
        THROW_FAIL_TO_CREATE_PROCESS("Failed to run command in terminal.");
    }
    pipe.reset(_popen(command.c_str(), "r"));
#else  // Non-Windows
    pipe.reset(popen(command.c_str(), "r"));
#endif

    if (!pipe) {
        LOG_F(ERROR, "Failed to run command '{}'.", command);
        THROW_FAIL_TO_CREATE_PROCESS("Failed to run command.");
    }

    constexpr std::size_t BUFFER_SIZE = 4096;
    std::array<char, BUFFER_SIZE> buffer{};
    std::ostringstream output;

    std::promise<void> exitSignal;
    std::future<void> futureObj = exitSignal.get_future();
    std::atomic<bool> stopReading{false};

    std::thread readerThread(
        [&pipe, &buffer, &output, &processLine, &futureObj, &stopReading]() {
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                if (stopReading) {
                    break;
                }

                std::string line(buffer.data());
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
    status = WEXITSTATUS(pclose(pipe.release()));
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
    LOG_F(INFO,
          "executeCommandWithInput called with command: {}, input: [hidden]",
          command);
    int status = 0;
    auto result =
        executeCommandInternal(command, false, processLine, status, input);
    LOG_F(INFO, "executeCommandWithInput completed with status: {}", status);
    return result;
}

void executeCommands(const std::vector<std::string> &commands) {
    LOG_F(INFO, "executeCommands called with {} commands", commands.size());
    std::vector<std::thread> threads;
    std::vector<std::string> errors;
    std::mutex errorMutex;

    for (const auto &command : commands) {
        threads.emplace_back([&command, &errors, &errorMutex]() {
            try {
                int status = 0;
                executeCommand(command, false, nullptr);
                if (status != 0) {
                    throw std::runtime_error("Error executing command: " +
                                             command);
                }
            } catch (const std::runtime_error &e) {
                std::lock_guard lock(errorMutex);
                errors.emplace_back(e.what());
            }
        });
    }

    for (auto &thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    if (!errors.empty()) {
        std::ostringstream oss;
        for (const auto &err : errors) {
            oss << err << "\n";
        }
        THROW_INVALID_ARGUMENT("One or more commands failed:\n" + oss.str());
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
    std::shared_ptr<utils::Env> env;
    GET_OR_CREATE_PTR(env, utils::Env, "LITHIUM.ENV");
    {
        std::lock_guard lock(envMutex);
        for (const auto &var : envVars) {
            auto oldValue = env->getEnv(var.first);
            if (!oldValue.empty()) {
                oldEnvVars[var.first] = oldValue;
            }
            env->setEnv(var.first, var.second);
        }
    }

    auto result = executeCommand(command, false, nullptr);

    {
        std::lock_guard lock(envMutex);
        for (const auto &var : envVars) {
            if (oldEnvVars.find(var.first) != oldEnvVars.end()) {
                env->setEnv(var.first, oldEnvVars[var.first]);
            } else {
                env->unsetEnv(var.first);
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

void killProcessByName(const std::string &processName, int signal) {
    LOG_F(INFO, "killProcessByName called with processName: {}, signal: {}",
          processName, signal);
#ifdef _WIN32
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Unable to create toolhelp snapshot.");
        THROW_SYSTEM_COLLAPSE("Unable to create toolhelp snapshot.");
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(snap, &entry)) {
        CloseHandle(snap);
        LOG_F(ERROR, "Unable to get the first process.");
        THROW_SYSTEM_COLLAPSE("Unable to get the first process.");
    }

    do {
        std::string currentProcess =
            atom::utils::WCharArrayToString(entry.szExeFile);
        if (currentProcess == processName) {
            HANDLE hProcess =
                OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
            if (hProcess) {
                if (!TerminateProcess(hProcess, 0)) {
                    LOG_F(ERROR, "Failed to terminate process '{}'.",
                          processName);
                    CloseHandle(hProcess);
                    THROW_SYSTEM_COLLAPSE("Failed to terminate process.");
                }
                CloseHandle(hProcess);
                LOG_F(INFO, "Process '{}' terminated.", processName);
            }
        }
    } while (Process32NextW(snap, &entry));

    CloseHandle(snap);
#else
    std::string cmd = "pkill -" + std::to_string(signal) + " -f " + processName;
    auto [output, status] = executeCommandWithStatus(cmd);
    if (status != 0) {
        LOG_F(ERROR, "Failed to kill process with name '{}'.", processName);
        THROW_SYSTEM_COLLAPSE("Failed to kill process by name.");
    }
    LOG_F(INFO, "Process '{}' terminated with signal {}.", processName, signal);
#endif
}

void killProcessByPID(int pid, int signal) {
    LOG_F(INFO, "killProcessByPID called with pid: {}, signal: {}", pid,
          signal);
#ifdef _WIN32
    HANDLE hProcess =
        OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (!hProcess) {
        LOG_F(ERROR, "Unable to open process with PID {}.", pid);
        THROW_SYSTEM_COLLAPSE("Unable to open process.");
    }
    if (!TerminateProcess(hProcess, 0)) {
        LOG_F(ERROR, "Failed to terminate process with PID {}.", pid);
        CloseHandle(hProcess);
        THROW_SYSTEM_COLLAPSE("Failed to terminate process by PID.");
    }
    CloseHandle(hProcess);
    LOG_F(INFO, "Process with PID {} terminated.", pid);
#else
    if (kill(pid, signal) == -1) {
        LOG_F(ERROR, "Failed to kill process with PID {}.", pid);
        THROW_SYSTEM_COLLAPSE("Failed to kill process by PID.");
    }
    int status;
    waitpid(pid, &status, 0);
    LOG_F(INFO, "Process with PID {} terminated with signal {}.", pid, signal);
#endif
}

auto startProcess(const std::string &command) -> std::pair<int, void *> {
    LOG_F(INFO, "startProcess called with command: {}", command);
#ifdef _WIN32
    STARTUPINFO startupInfo{};
    PROCESS_INFORMATION processInfo{};
    startupInfo.cb = sizeof(startupInfo);

    std::wstring commandW = atom::utils::StringToLPWSTR(command);
    if (CreateProcessW(nullptr, &commandW[0], nullptr, nullptr, FALSE, 0,
                       nullptr, nullptr, &startupInfo, &processInfo)) {
        CloseHandle(processInfo.hThread);
        LOG_F(INFO, "Process '{}' started with PID: {}", command,
              processInfo.dwProcessId);
        return {processInfo.dwProcessId, processInfo.hProcess};
    } else {
        LOG_F(ERROR, "Failed to start process '{}'.", command);
        THROW_FAIL_TO_CREATE_PROCESS("Failed to start process.");
    }
#else
    pid_t pid = fork();
    if (pid == -1) {
        LOG_F(ERROR, "Failed to fork process for command '{}'.", command);
        THROW_FAIL_TO_CREATE_PROCESS("Failed to fork process.");
    }
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", command.c_str(), (char *)nullptr);
        _exit(EXIT_FAILURE);  // If execl fails
    } else {
        LOG_F(INFO, "Process '{}' started with PID: {}", command, pid);
        return {pid, nullptr};
    }
#endif
}

}  // namespace atom::system
