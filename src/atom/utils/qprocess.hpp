#ifndef ATOM_UTILS_QPROCESS_HPP
#define ATOM_UTILS_QPROCESS_HPP

#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace atom::utils {
class QProcess {
public:
    QProcess();
    ~QProcess();

    void setWorkingDirectory(const std::string &dir);
    void setEnvironment(const std::vector<std::string> &env);

    void start(const std::string &program,
               const std::vector<std::string> &args);
    bool waitForStarted(int timeoutMs = -1);
    bool waitForFinished(int timeoutMs = -1);
    bool isRunning() const;

    void write(const std::string &data);
    std::string readAllStandardOutput();
    std::string readAllStandardError();
    void terminate();

private:
    bool running;
    bool processStarted;
    std::string program;
    std::vector<std::string> args;
    std::optional<std::string> workingDirectory;
    std::vector<std::string> environment;

    std::mutex mutex;
    std::condition_variable cv;

#ifdef _WIN32
    struct PROCESS_INFORMATION procInfo;
    void startWindowsProcess();
    HANDLE childStdinWrite, childStdoutRead, childStderrRead;
#else
    pid_t childPid;
    int childStdin, childStdout, childStderr;
    void startPosixProcess();
#endif
};
}  // namespace atom::utils

#endif  // ATOM_UTILS_QPROCESS_HPP
