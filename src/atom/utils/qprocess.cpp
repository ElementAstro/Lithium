#include "qprocess.hpp"

#include <thread>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::utils {
QProcess::QProcess()
    : running(false),
      processStarted(false),
      childPid(-1),
      childStdin(-1),
      childStdout(-1),
      childStderr(-1) {}

QProcess::~QProcess() {
    if (running) {
        terminate();
    }
}

void QProcess::setWorkingDirectory(const std::string &dir) {
    workingDirectory = dir;
}

void QProcess::setEnvironment(const std::vector<std::string> &env) {
    environment = env;
}

void QProcess::start(const std::string &program,
                     const std::vector<std::string> &args) {
    if (running) {
        THROW_RUNTIME_ERROR("Process already running");
    }

    this->program = program;
    this->args = args;

#ifdef _WIN32
    startWindowsProcess();
#else
    startPosixProcess();
#endif

    running = true;
    {
        std::lock_guard lock(mutex);
        processStarted = true;
    }
    cv.notify_all();
}

auto QProcess::waitForStarted(int timeoutMs) -> bool {
    std::unique_lock lock(mutex);
    if (timeoutMs < 0) {
        cv.wait(lock, [this] { return processStarted; });
    } else {
        if (!cv.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                         [this] { return processStarted; })) {
            return false;
        }
    }
    return true;
}

auto QProcess::waitForFinished(int timeoutMs) -> bool {
#ifdef _WIN32
    DWORD waitResult = WaitForSingleObject(
        procInfo.hProcess, timeoutMs < 0 ? INFINITE : timeoutMs);
    return waitResult == WAIT_OBJECT_0;
#else
    if (childPid == -1) {
        return false;
}

    int status;
    if (timeoutMs < 0) {
        waitpid(childPid, &status, 0);
    } else {
        auto startTime = std::chrono::steady_clock::now();
        while (true) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    currentTime - startTime);
            if (elapsed.count() >= timeoutMs) {
                return false;
            }
            if (waitpid(childPid, &status, WNOHANG) > 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    return true;
#endif
}

bool QProcess::isRunning() const {
#ifdef _WIN32
    DWORD exitCode;
    GetExitCodeProcess(procInfo.hProcess, &exitCode);
    return exitCode == STILL_ACTIVE;
#else
    if (childPid == -1) {
        return false;
}
    int status;
    return waitpid(childPid, &status, WNOHANG) == 0;
#endif
}

void QProcess::write(const std::string &data) {
#ifdef _WIN32
    DWORD written;
    WriteFile(childStdinWrite, data.c_str(), data.size(), &written, nullptr);
#else
    if (childStdin != -1) {
        ::write(childStdin, data.c_str(), data.size());
    }
#endif
}

auto QProcess::readAllStandardOutput() -> std::string {
#ifdef _WIN32
    std::string output;
    DWORD read;
    CHAR buffer[4096];
    while (ReadFile(childStdoutRead, buffer, sizeof(buffer), &read, nullptr) &&
           read > 0) {
        output.append(buffer, read);
    }
    return output;
#else
    std::string output;
    char buffer[4096];
    ssize_t count;
    while ((count = ::read(childStdout, buffer, sizeof(buffer))) > 0) {
        output.append(buffer, count);
    }
    return output;
#endif
}

auto QProcess::readAllStandardError() -> std::string {
#ifdef _WIN32
    std::string output;
    DWORD read;
    CHAR buffer[4096];
    while (ReadFile(childStderrRead, buffer, sizeof(buffer), &read, nullptr) &&
           read > 0) {
        output.append(buffer, read);
    }
    return output;
#else
    std::string output;
    char buffer[4096];
    ssize_t count;
    while ((count = ::read(childStderr, buffer, sizeof(buffer))) > 0) {
        output.append(buffer, count);
    }
    return output;
#endif
}

void QProcess::terminate() {
    if (running) {
#ifdef _WIN32
        TerminateProcess(procInfo.hProcess, 0);
        CloseHandle(procInfo.hProcess);
        CloseHandle(procInfo.hThread);
#else
        kill(childPid, SIGTERM);
#endif
        running = false;
    }
}

#ifdef _WIN32
void QProcess::startWindowsProcess() {
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE childStdoutWrite = NULL;
    HANDLE childStdinRead = NULL;
    HANDLE childStderrWrite = NULL;

    if (!CreatePipe(&childStdoutRead, &childStdoutWrite, &saAttr, 0) ||
        !SetHandleInformation(childStdoutRead, HANDLE_FLAG_INHERIT, 0)) {
        THROW_SYSTEM_COLLAPSE("Failed to create stdout pipe");
    }

    if (!CreatePipe(&childStdinRead, &childStdinWrite, &saAttr, 0) ||
        !SetHandleInformation(childStdinWrite, HANDLE_FLAG_INHERIT, 0)) {
        THROW_SYSTEM_COLLAPSE("Failed to create stdin pipe");
    }

    if (!CreatePipe(&childStderrRead, &childStderrWrite, &saAttr, 0) ||
        !SetHandleInformation(childStderrRead, HANDLE_FLAG_INHERIT, 0)) {
        THROW_SYSTEM_COLLAPSE("Failed to create stderr pipe");
    }

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = childStderrWrite;
    siStartInfo.hStdOutput = childStdoutWrite;
    siStartInfo.hStdInput = childStdinRead;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

    std::string cmdLine = program;
    for (const auto &arg : args) {
        cmdLine += " " + arg;
    }

    char *envBlock = nullptr;
    if (!environment.empty()) {
        std::string envString;
        for (const auto &envVar : environment) {
            envString += envVar + '\0';
        }
        envString += '\0';
        envBlock = const_cast<char *>(envString.c_str());
    }

    if (!CreateProcess(NULL, cmdLine.data(), NULL, NULL, TRUE, 0, envBlock,
                       workingDirectory ? workingDirectory->c_str() : NULL,
                       &siStartInfo, &procInfo)) {
        THROW_SYSTEM_COLLAPSE("Failed to start process");
    }

    CloseHandle(childStdoutWrite);
    CloseHandle(childStdinRead);
    CloseHandle(childStderrWrite);
}
#else
void QProcess::startPosixProcess() {
    int stdinPipe[2];
    int stdoutPipe[2];
    int stderrPipe[2];

    // 创建管道
    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1 ||
        pipe(stderrPipe) == -1) {
        THROW_SYSTEM_COLLAPSE("Failed to create pipes");
    }

    // 创建子进程
    childPid = fork();

    if (childPid == 0) {  // 子进程
        // 关闭不需要的管道端
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stderrPipe[0]);

        // 重定向标准输入/输出/错误
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);

        // 关闭原始的管道描述符
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        // 如果设置了工作目录，切换到该目录
        if (workingDirectory) {
            if (chdir(workingDirectory->c_str()) != 0) {
                LOG_F(ERROR, "Failed to change directory");
                exit(1);
            }
        }

        // 设置环境变量
        if (!environment.empty()) {
            for (const auto &envVar : environment) {
                putenv(const_cast<char *>(envVar.c_str()));
            }
        }

        // 构建exec需要的参数列表
        std::vector<char *> execArgs;
        execArgs.push_back(const_cast<char *>(program.c_str()));
        for (const auto &arg : args) {
            execArgs.push_back(const_cast<char *>(arg.c_str()));
        }
        execArgs.push_back(nullptr);

        // 使用execvp执行新程序
        execvp(execArgs[0], execArgs.data());
        LOG_F(ERROR, "Failed to execute process");
        exit(1);                // execvp失败则退出子进程
    } else if (childPid > 0) {  // 父进程
        // 关闭不需要的管道端
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        // 保存管道文件描述符，用于后续通信
        childStdin = stdinPipe[1];
        childStdout = stdoutPipe[0];
        childStderr = stderrPipe[0];
    } else {
        THROW_SYSTEM_COLLAPSE("Failed to fork process");
    }
}
#endif

}  // namespace atom::utils
