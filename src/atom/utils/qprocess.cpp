#include "qprocess.hpp"

#include <condition_variable>
#include <mutex>
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

class QProcess::Impl {
public:
    Impl() = default;
    ~Impl();

    void setWorkingDirectory(const std::string& dir);
    void setEnvironment(const std::vector<std::string>& env);

    void start(const std::string& program,
               const std::vector<std::string>& args);
    auto waitForStarted(int timeoutMs) -> bool;
    auto waitForFinished(int timeoutMs) -> bool;
    [[nodiscard]] auto isRunning() const -> bool;

    void write(const std::string& data);
    auto readAllStandardOutput() -> std::string;
    auto readAllStandardError() -> std::string;
    void terminate();

private:
    void startWindowsProcess();
    void startPosixProcess();

    bool running_{};
    bool processStarted_{};
    std::string program_;
    std::vector<std::string> args_;
    std::optional<std::string> workingDirectory_;
    std::vector<std::string> environment_;

    std::mutex mutex_;
    std::condition_variable cv_;

#ifdef _WIN32
    PROCESS_INFORMATION procInfo_{};
    HANDLE childStdinWrite_{nullptr};
    HANDLE childStdoutRead_{nullptr};
    HANDLE childStderrRead_{nullptr};
#else
    pid_t childPid_{-1};
    int childStdin_{-1};
    int childStdout_{-1};
    int childStderr_{-1};
#endif
};

// Implementation of QProcess
QProcess::QProcess() : impl_(std::make_unique<Impl>()) {
    LOG_F(INFO, "QProcess constructor called");
}

QProcess::~QProcess() { LOG_F(INFO, "QProcess destructor called"); }

void QProcess::setWorkingDirectory(const std::string& dir) {
    LOG_F(INFO, "QProcess::setWorkingDirectory called with dir: {}", dir);
    impl_->setWorkingDirectory(dir);
}

void QProcess::setEnvironment(const std::vector<std::string>& env) {
    LOG_F(INFO, "QProcess::setEnvironment called");
    impl_->setEnvironment(env);
}

void QProcess::start(const std::string& program,
                     const std::vector<std::string>& args) {
    LOG_F(INFO, "QProcess::start called with program: {}", program);
    impl_->start(program, args);
}

auto QProcess::waitForStarted(int timeoutMs) -> bool {
    LOG_F(INFO, "QProcess::waitForStarted called with timeoutMs: {}",
          timeoutMs);
    return impl_->waitForStarted(timeoutMs);
}

auto QProcess::waitForFinished(int timeoutMs) -> bool {
    LOG_F(INFO, "QProcess::waitForFinished called with timeoutMs: {}",
          timeoutMs);
    return impl_->waitForFinished(timeoutMs);
}

auto QProcess::isRunning() const -> bool {
    LOG_F(INFO, "QProcess::isRunning called");
    return impl_->isRunning();
}

void QProcess::write(const std::string& data) {
    LOG_F(INFO, "QProcess::write called with data: {}", data);
    impl_->write(data);
}

auto QProcess::readAllStandardOutput() -> std::string {
    LOG_F(INFO, "QProcess::readAllStandardOutput called");
    return impl_->readAllStandardOutput();
}

auto QProcess::readAllStandardError() -> std::string {
    LOG_F(INFO, "QProcess::readAllStandardError called");
    return impl_->readAllStandardError();
}

void QProcess::terminate() {
    LOG_F(INFO, "QProcess::terminate called");
    impl_->terminate();
}

// Implementation details of QProcess::Impl
QProcess::Impl::~Impl() {
    LOG_F(INFO, "QProcess::Impl destructor called");
    if (running_) {
        terminate();
    }
}

void QProcess::Impl::setWorkingDirectory(const std::string& dir) {
    LOG_F(INFO, "QProcess::Impl::setWorkingDirectory called with dir: {}", dir);
    workingDirectory_ = dir;
}

void QProcess::Impl::setEnvironment(const std::vector<std::string>& env) {
    LOG_F(INFO, "QProcess::Impl::setEnvironment called");
    environment_ = env;
}

void QProcess::Impl::start(const std::string& program,
                           const std::vector<std::string>& args) {
    LOG_F(INFO, "QProcess::Impl::start called with program: {}", program);
    if (running_) {
        LOG_F(ERROR, "Process already running");
        THROW_RUNTIME_ERROR("Process already running");
    }

    this->program_ = program;
    this->args_ = args;

#ifdef _WIN32
    startWindowsProcess();
#else
    startPosixProcess();
#endif

    running_ = true;
    {
        std::lock_guard lock(mutex_);
        processStarted_ = true;
    }
    cv_.notify_all();
    LOG_F(INFO, "QProcess::Impl::start completed");
}

auto QProcess::Impl::waitForStarted(int timeoutMs) -> bool {
    LOG_F(INFO, "QProcess::Impl::waitForStarted called with timeoutMs: {}",
          timeoutMs);
    std::unique_lock lock(mutex_);
    if (timeoutMs < 0) {
        cv_.wait(lock, [this] { return processStarted_; });
    } else {
        if (!cv_.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                          [this] { return processStarted_; })) {
            LOG_F(WARNING, "QProcess::Impl::waitForStarted timed out");
            return false;
        }
    }
    LOG_F(INFO, "QProcess::Impl::waitForStarted completed");
    return true;
}

auto QProcess::Impl::waitForFinished(int timeoutMs) -> bool {
    LOG_F(INFO, "QProcess::Impl::waitForFinished called with timeoutMs: {}",
          timeoutMs);
#ifdef _WIN32
    DWORD waitResult = WaitForSingleObject(
        procInfo_.hProcess, timeoutMs < 0 ? INFINITE : timeoutMs);
    bool result = waitResult == WAIT_OBJECT_0;
    LOG_F(INFO, "QProcess::Impl::waitForFinished completed with result: {}",
          result ? "true" : "false");
    return result;
#else
    if (childPid_ == -1) {
        LOG_F(WARNING,
              "QProcess::Impl::waitForFinished called with invalid childPid");
        return false;
    }

    int status;
    if (timeoutMs < 0) {
        waitpid(childPid_, &status, 0);
    } else {
        auto startTime = std::chrono::steady_clock::now();
        while (true) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsed =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    currentTime - startTime);
            if (elapsed.count() >= timeoutMs) {
                LOG_F(WARNING, "QProcess::Impl::waitForFinished timed out");
                return false;
            }
            if (waitpid(childPid_, &status, WNOHANG) > 0) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    LOG_F(INFO, "QProcess::Impl::waitForFinished completed");
    return true;
#endif
}

auto QProcess::Impl::isRunning() const -> bool {
    LOG_F(INFO, "QProcess::Impl::isRunning called");
#ifdef _WIN32
    DWORD exitCode;
    GetExitCodeProcess(procInfo_.hProcess, &exitCode);
    bool result = exitCode == STILL_ACTIVE;
    LOG_F(INFO, "QProcess::Impl::isRunning returning: {}",
          result ? "true" : "false");
    return result;
#else
    if (childPid_ == -1) {
        LOG_F(WARNING,
              "QProcess::Impl::isRunning called with invalid childPid");
        return false;
    }
    int status;
    bool result = waitpid(childPid_, &status, WNOHANG) == 0;
    LOG_F(INFO, "QProcess::Impl::isRunning returning: {}",
          result ? "true" : "false");
    return result;
#endif
}

void QProcess::Impl::write(const std::string& data) {
    LOG_F(INFO, "QProcess::Impl::write called with data: {}", data);
#ifdef _WIN32
    DWORD written;
    WriteFile(childStdinWrite_, data.c_str(), data.size(), &written, nullptr);
#else
    if (childStdin_ != -1) {
        ::write(childStdin_, data.c_str(), data.size());
    }
#endif
    LOG_F(INFO, "QProcess::Impl::write completed");
}

auto QProcess::Impl::readAllStandardOutput() -> std::string {
    LOG_F(INFO, "QProcess::Impl::readAllStandardOutput called");
#ifdef _WIN32
    std::string output;
    DWORD read;
    constexpr size_t BUFFER_SIZE = 4096;
    std::array<CHAR, BUFFER_SIZE> buffer;
    while ((ReadFile(childStdoutRead_, buffer.data(), buffer.size(), &read,
                     nullptr) != 0) &&
           read > 0) {
        output.append(buffer.data(), read);
    }
    LOG_F(INFO, "QProcess::Impl::readAllStandardOutput completed");
    return output;
#else
    std::string output;
    constexpr size_t bufferSize = 4096;
    std::array<char, bufferSize> buffer;
    ssize_t count;
    while ((count = ::read(childStdout_, buffer.data(), buffer.size())) > 0) {
        output.append(buffer.data(), count);
    }
    LOG_F(INFO, "QProcess::Impl::readAllStandardOutput completed");
    return output;
#endif
}

auto QProcess::Impl::readAllStandardError() -> std::string {
    LOG_F(INFO, "QProcess::Impl::readAllStandardError called");
#ifdef _WIN32
    std::string output;
    DWORD read;
    constexpr size_t BUFFER_SIZE = 4096;
    std::array<CHAR, BUFFER_SIZE> buffer;
    while ((ReadFile(childStderrRead_, buffer.data(), buffer.size(), &read,
                     nullptr) != 0) &&
           read > 0) {
        output.append(buffer.data(), read);
    }
    LOG_F(INFO, "QProcess::Impl::readAllStandardError completed");
    return output;
#else
    std::string output;
    constexpr size_t bufferSize = 4096;
    std::array<char, bufferSize> buffer;
    ssize_t count;
    while ((count = ::read(childStderr_, buffer.data(), buffer.size())) > 0) {
        output.append(buffer.data(), count);
    }
    LOG_F(INFO, "QProcess::Impl::readAllStandardError completed");
    return output;
#endif
}

void QProcess::Impl::terminate() {
    LOG_F(INFO, "QProcess::Impl::terminate called");
    if (running_) {
#ifdef _WIN32
        TerminateProcess(procInfo_.hProcess, 0);
        CloseHandle(procInfo_.hProcess);
        CloseHandle(procInfo_.hThread);
#else
        kill(childPid_, SIGTERM);
#endif
        running_ = false;
    }
    LOG_F(INFO, "QProcess::Impl::terminate completed");
}

#ifdef _WIN32
void QProcess::Impl::startWindowsProcess() {
    LOG_F(INFO, "QProcess::Impl::startWindowsProcess called");
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = nullptr;

    HANDLE childStdoutWrite = nullptr;
    HANDLE childStdinRead = nullptr;
    HANDLE childStderrWrite = nullptr;

    if ((CreatePipe(&childStdoutRead_, &childStdoutWrite, &saAttr, 0) == 0) ||
        (SetHandleInformation(childStdoutRead_, HANDLE_FLAG_INHERIT, 0) == 0)) {
        LOG_F(ERROR, "Failed to create stdout pipe");
        THROW_SYSTEM_COLLAPSE("Failed to create stdout pipe");
    }

    if ((CreatePipe(&childStdinRead, &childStdinWrite_, &saAttr, 0) == 0) ||
        (SetHandleInformation(childStdinWrite_, HANDLE_FLAG_INHERIT, 0) == 0)) {
        LOG_F(ERROR, "Failed to create stdin pipe");
        THROW_SYSTEM_COLLAPSE("Failed to create stdin pipe");
    }

    if ((CreatePipe(&childStderrRead_, &childStderrWrite, &saAttr, 0) == 0) ||
        (SetHandleInformation(childStderrRead_, HANDLE_FLAG_INHERIT, 0) == 0)) {
        LOG_F(ERROR, "Failed to create stderr pipe");
        THROW_SYSTEM_COLLAPSE("Failed to create stderr pipe");
    }

    STARTUPINFO siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = childStderrWrite;
    siStartInfo.hStdOutput = childStdoutWrite;
    siStartInfo.hStdInput = childStdinRead;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&procInfo_, sizeof(PROCESS_INFORMATION));

    std::string cmdLine = program_;
    for (const auto& arg : args_) {
        cmdLine += " " + arg;
    }

    char* envBlock = nullptr;
    if (!environment_.empty()) {
        std::string envString;
        for (const auto& envVar : environment_) {
            envString += envVar + '\0';
        }
        envString += '\0';
        envBlock = const_cast<char*>(envString.c_str());
    }

    if (!CreateProcess(nullptr, cmdLine.data(), nullptr, nullptr, TRUE, 0,
                       envBlock,
                       workingDirectory_ ? workingDirectory_->c_str() : nullptr,
                       &siStartInfo, &procInfo_)) {
        LOG_F(ERROR, "Failed to start process");
        THROW_SYSTEM_COLLAPSE("Failed to start process");
    }

    CloseHandle(childStdoutWrite);
    CloseHandle(childStdinRead);
    CloseHandle(childStderrWrite);
    LOG_F(INFO, "QProcess::Impl::startWindowsProcess completed");
}
#else
void QProcess::Impl::startPosixProcess() {
    LOG_F(INFO, "QProcess::Impl::startPosixProcess called");
    int stdinPipe[2];
    int stdoutPipe[2];
    int stderrPipe[2];

    // Create pipes
    if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1 ||
        pipe(stderrPipe) == -1) {
        LOG_F(ERROR, "Failed to create pipes");
        THROW_SYSTEM_COLLAPSE("Failed to create pipes");
    }

    // Fork process
    childPid_ = fork();

    if (childPid_ == 0) {  // Child process
        // Close unnecessary pipe ends
        close(stdinPipe[1]);
        close(stdoutPipe[0]);
        close(stderrPipe[0]);

        // Redirect standard input/output/error
        dup2(stdinPipe[0], STDIN_FILENO);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);

        // Close original pipe descriptors
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        // Change directory if set
        if (workingDirectory_) {
            if (chdir(workingDirectory_->c_str()) != 0) {
                LOG_F(ERROR, "Failed to change directory");
                exit(1);
            }
        }

        // Set environment variables
        if (!environment_.empty()) {
            for (const auto& envVar : environment_) {
                putenv(const_cast<char*>(envVar.c_str()));
            }
        }

        // Build exec argument list
        std::vector<char*> execArgs;
        execArgs.push_back(const_cast<char*>(program_.c_str()));
        for (const auto& arg : args_) {
            execArgs.push_back(const_cast<char*>(arg.c_str()));
        }
        execArgs.push_back(nullptr);

        // Execute new program
        execvp(execArgs[0], execArgs.data());
        LOG_F(ERROR, "Failed to execute process");
        exit(1);                 // Exit if execvp fails
    } else if (childPid_ > 0) {  // Parent process
        // Close unnecessary pipe ends
        close(stdinPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        // Store pipe file descriptors for communication
        childStdin_ = stdinPipe[1];
        childStdout_ = stdoutPipe[0];
        childStderr_ = stderrPipe[0];
    } else {
        LOG_F(ERROR, "Failed to fork process");
        THROW_SYSTEM_COLLAPSE("Failed to fork process");
    }
    LOG_F(INFO, "QProcess::Impl::startPosixProcess completed");
}
#endif

}  // namespace atom::utils