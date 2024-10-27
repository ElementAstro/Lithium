#include "standalone.hpp"
#include <minwindef.h>

#include <array>
#include <chrono>
#include <format>
#include <optional>
#include <span>
#include <thread>

#include "atom/macro.hpp"

#if defined(_WIN32) || defined(_WIN64)
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <windows.h>
#define pipe _pipe
#define popen _popen
#define pclose _pclose
#else
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

struct LocalDriver {
    int processHandle{};
    std::variant<std::pair<int, int>, std::pair<int, int*>> io;
    std::string name;
    bool isListening{};
    InteractionMethod method;
};

#if !defined(_WIN32) && !defined(_WIN64)
constexpr char SEM_NAME[] = "/driver_semaphore";
constexpr char SHM_NAME[] = "/driver_shm";
#endif
constexpr char FIFO_NAME[] = "/tmp/driver_fifo";

class StandAloneComponentImpl {
public:
    LocalDriver driver;
    std::atomic<bool> shouldExit{false};
    std::jthread driverThread;

    void handleDriverOutput(std::string_view driver_name,
                            std::span<const char> buffer) {
        LOG_F(INFO, "Output from driver {}: {}", driver_name,
              std::string_view(buffer.data(), buffer.size()));
    }
};

StandAloneComponent::StandAloneComponent(std::string name)
    : Component(std::move(name)),
      impl_(std::make_shared<StandAloneComponentImpl>()) {
    doc("A standalone component that can be used to run a local driver");
    def("start", &StandAloneComponent::startLocalDriver);
    def("stop", &StandAloneComponent::stopLocalDriver);
    def("listen", &StandAloneComponent::toggleDriverListening);
    def("send", &StandAloneComponent::sendMessageToDriver);
    def("print", &StandAloneComponent::printDriver);
    def("monitor", &StandAloneComponent::monitorDrivers);
}

StandAloneComponent::~StandAloneComponent() {
    LOG_F(INFO, "Component {} destroyed", getName());
    impl_->shouldExit = true;
    if (impl_->driverThread.joinable()) {
        impl_->driverThread.join();
    }
}

void StandAloneComponent::startLocalDriver(const std::string& driver_name,
                                           InteractionMethod method) {
    std::variant<std::pair<int, int>, std::pair<int, int*>> io;

    switch (method) {
        case InteractionMethod::Pipe:
            if (auto pipes = createPipes()) {
                io = *pipes;
            } else {
                return;
            }
            break;
        case InteractionMethod::FIFO:
            if (auto fifo = createFIFO()) {
                io = *fifo;
            } else {
                return;
            }
            break;
        case InteractionMethod::SharedMemory:
            if (auto shm = createSharedMemory()) {
                io = *shm;
            } else {
                return;
            }
            break;
    }

#if defined(_WIN32) || defined(_WIN64)
    startWindowsProcess(driver_name, io);
#else
    startUnixProcess(driver_name, io);
#endif
    impl_->driver.method = method;
    impl_->driverThread =
        std::jthread(&StandAloneComponent::backgroundProcessing, this);
}

void StandAloneComponent::backgroundProcessing() {
    while (!impl_->shouldExit) {
        monitorDrivers();
        processMessages();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

auto StandAloneComponent::createPipes() -> std::optional<std::pair<int, int>> {
    int stdinPipe[2], stdoutPipe[2];

#if defined(_WIN32) || defined(_WIN64)
    if (_pipe(stdinPipe, 4096, _O_BINARY | _O_NOINHERIT) == 0 &&
        _pipe(stdoutPipe, 4096, _O_BINARY | _O_NOINHERIT) == 0) {
        return std::make_pair(stdinPipe[1], stdoutPipe[0]);
    }
#else
    if (pipe(stdinPipe) == 0 && pipe(stdoutPipe) == 0) {
        return std::make_pair(stdinPipe[1], stdoutPipe[0]);
    }
#endif
    LOG_F(ERROR, "Failed to create pipes");
    return std::nullopt;
}

auto StandAloneComponent::createFIFO() -> std::optional<std::pair<int, int>> {
#if defined(_WIN32) || defined(_WIN64)
    LOG_F(ERROR, "FIFO not supported on Windows");
    return std::nullopt;
#else
    if (mkfifo(FIFO_NAME, 0666) == -1 && errno != EEXIST) {
        LOG_F(ERROR, "Failed to create FIFO");
        return std::nullopt;
    }

    int readFd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
    int writeFd = open(FIFO_NAME, O_WRONLY);

    if (readFd == -1 || writeFd == -1) {
        LOG_F(ERROR, "Failed to open FIFO");
        if (readFd != -1)
            close(readFd);
        if (writeFd != -1)
            close(writeFd);
        return std::nullopt;
    }

    return std::make_pair(writeFd, readFd);
#endif
}

auto StandAloneComponent::createSharedMemory()
    -> std::optional<std::pair<int, int*>> {
#if defined(_WIN32) || defined(_WIN64)
    LOG_F(ERROR, "Shared memory not implemented for Windows");
    return std::nullopt;
#else
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        LOG_F(ERROR, "Failed to create shared memory");
        return std::nullopt;
    }

    if (ftruncate(shm_fd, sizeof(int)) == -1) {
        LOG_F(ERROR, "Failed to set size of shared memory");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return std::nullopt;
    }

    int* shm_ptr = static_cast<int*>(mmap(
        nullptr, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (shm_ptr == MAP_FAILED) {
        LOG_F(ERROR, "Failed to map shared memory");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return std::nullopt;
    }

    *shm_ptr = 0;  // Initialize shared memory to 0
    return std::make_pair(shm_fd, shm_ptr);
#endif
}

#if defined(_WIN32) || defined(_WIN64)
void StandAloneComponent::startWindowsProcess(
    const std::string& driver_name,
    std::variant<std::pair<int, int>, std::pair<int, int*>> ioVariant) {
    auto [inHandle, outHandle] = std::get<std::pair<int, int>>(ioVariant);

    SECURITY_ATTRIBUTES securityAttributes = {sizeof(SECURITY_ATTRIBUTES),
                                              nullptr, TRUE};
    HANDLE hStdinRead;
    HANDLE hStdinWrite;
    HANDLE hStdoutRead;
    HANDLE hStdoutWrite;

    if (CreatePipe(&hStdinRead, &hStdinWrite, &securityAttributes, 0) ==
            FALSE ||
        CreatePipe(&hStdoutRead, &hStdoutWrite, &securityAttributes, 0) ==
            FALSE) {
        LOG_F(ERROR, "Failed to create pipes");
        return;
    }

    SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hStdinWrite, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO startupInfo = {};
    startupInfo.cb = sizeof(STARTUPINFO);
    startupInfo.hStdError = hStdoutWrite;
    startupInfo.hStdOutput = hStdoutWrite;
    startupInfo.hStdInput = hStdinRead;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION processInfo;
    std::string cmd = driver_name;

    if (!CreateProcess(nullptr, cmd.data(), nullptr, nullptr, TRUE,
                       CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo,
                       &processInfo)) {
        LOG_F(ERROR, "Failed to start process");
        return;
    }

    CloseHandle(hStdoutWrite);
    CloseHandle(hStdinRead);

    impl_->driver.processHandle =
        static_cast<int>(reinterpret_cast<intptr_t>(processInfo.hProcess));
    impl_->driver.io = std::make_pair(
        _open_osfhandle(reinterpret_cast<intptr_t>(hStdinWrite), 0),
        _open_osfhandle(reinterpret_cast<intptr_t>(hStdoutRead), 0));
    impl_->driver.name = driver_name;
}
#else
void StandAloneComponent::startUnixProcess(
    const std::string& driver_name,
    std::variant<std::pair<int, int>, std::pair<int, int*>> io) {
    auto sem = createSemaphore().value_or(nullptr);
    if (sem == nullptr) {
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        handleChildProcess(driver_name, io, nullptr, sem, -1);
    } else if (pid > 0) {
        handleParentProcess(pid, io, nullptr, sem, -1);
    } else {
        LOG_F(ERROR, "Failed to fork driver process");
        sem_close(sem);
    }
}

void StandAloneComponent::handleChildProcess(
    const std::string& driver_name,
    std::variant<std::pair<int, int>, std::pair<int, int*>> io, int* shm_ptr,
    sem_t* sem, int shm_fd) {
    if (std::holds_alternative<std::pair<int, int>>(io)) {
        auto [inFd, outFd] = std::get<std::pair<int, int>>(io);
        dup2(inFd, STDIN_FILENO);
        dup2(outFd, STDOUT_FILENO);
    }

    execlp(driver_name.data(), driver_name.data(), nullptr);

    if (shm_ptr)
        *shm_ptr = -1;
    sem_post(sem);
    LOG_F(ERROR, "Failed to exec driver process");
    if (shm_fd != -1) {
        close(shm_fd);
        munmap(shm_ptr, sizeof(int));
    }
    sem_close(sem);
    std::exit(1);
}

void StandAloneComponent::handleParentProcess(
    pid_t pid, std::variant<std::pair<int, int>, std::pair<int, int*>> io,
    int* shm_ptr, sem_t* sem, int shm_fd) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1;

    if (sem_timedwait(sem, &ts) == -1) {
        LOG_F(ERROR, errno == ETIMEDOUT ? "Driver process start timed out"
                                        : "Failed to wait on semaphore");
        kill(pid, SIGKILL);
        waitpid(pid, nullptr, 0);
    } else if ((shm_ptr != nullptr) && *shm_ptr == -1) {
        LOG_F(ERROR, "Driver process failed to start");
        waitpid(pid, nullptr, 0);
    } else {
        impl_->driver.processHandle = pid;
        impl_->driver.io = io;
    }

    if (shm_fd != -1) {
        closeSharedMemory(shm_fd, shm_ptr);
    }
    sem_close(sem);
}
#endif

auto StandAloneComponent::createSemaphore() -> std::optional<sem_t*> {
#if defined(_WIN32) || defined(_WIN64)
    LOG_F(ERROR, "Semaphore not implemented for Windows");
    return std::nullopt;
#else
    sem_t* sem = sem_open(SEM_NAME, O_CREAT | O_EXCL, 0644, 0);
    if (sem == SEM_FAILED) {
        LOG_F(ERROR, "Failed to create semaphore");
        return std::nullopt;
    }
    sem_unlink(SEM_NAME);  // Ensure the semaphore is removed once it's no
                           // longer needed
    return sem;
#endif
}

void StandAloneComponent::closeSharedMemory(int shm_fd, int* shm_ptr) {
#if defined(_WIN32) || defined(_WIN64)
    ATOM_UNREF_PARAM(shm_fd);
    ATOM_UNREF_PARAM(shm_ptr);
#else
    munmap(shm_ptr, sizeof(int));
    close(shm_fd);
    shm_unlink(SHM_NAME);
#endif
}

void StandAloneComponent::stopLocalDriver() {
    std::visit(
        [](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::pair<int, int>>) {
                close(arg.first);
                close(arg.second);
            } else if constexpr (std::is_same_v<T, std::pair<int, int*>>) {
#if defined(_WIN32) || defined(_WIN64)
                close(arg.first);
                UnmapViewOfFile(arg.second);
#else
                close(arg.first);
                munmap(arg.second, sizeof(int));
#endif
            }
        },
        impl_->driver.io);

#if defined(_WIN32) || defined(_WIN64)
    TerminateProcess(reinterpret_cast<HANDLE>(impl_->driver.processHandle), 0);
    CloseHandle(reinterpret_cast<HANDLE>(impl_->driver.processHandle));
#else
    kill(impl_->driver.processHandle, SIGTERM);
    waitpid(impl_->driver.processHandle, nullptr, 0);
#endif
    impl_->shouldExit = true;
    if (impl_->driverThread.joinable()) {
        impl_->driverThread.join();
    }

    // Clean up FIFO if used
    if (impl_->driver.method == InteractionMethod::FIFO) {
        unlink(FIFO_NAME);
    }
}

void StandAloneComponent::monitorDrivers() {
#if defined(_WIN32) || defined(_WIN64)
    DWORD exitCode;
    if (GetExitCodeProcess(
            reinterpret_cast<HANDLE>(impl_->driver.processHandle), &exitCode) &&
        exitCode == STILL_ACTIVE) {
        return;
    }
#else
    int status;
    pid_t result = waitpid(impl_->driver.processHandle, &status, WNOHANG);
    if (result == 0)
        return;
    if (result == -1) {
        LOG_F(ERROR, "Failed to wait for driver process");
        return;
    }
#endif
    LOG_F(INFO, "Driver {} exited, restarting...", impl_->driver.name);
    startLocalDriver(impl_->driver.name, impl_->driver.method);
}

void StandAloneComponent::processMessages() {
    std::array<char, 1024> buffer;
    if (impl_->driver.isListening) {
        int bytesRead = 0;
        std::visit(
            [&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::pair<int, int>>) {
#if defined(_WIN32) || defined(_WIN64)
                    bytesRead = _read(arg.second, buffer.data(),
                                      static_cast<unsigned int>(buffer.size()));
#else
                    int flags = fcntl(arg.second, F_GETFL, 0);
                    fcntl(arg.second, F_SETFL, flags | O_NONBLOCK);
                    bytesRead = read(arg.second, buffer.data(), buffer.size());
#endif
                } else if constexpr (std::is_same_v<T, std::pair<int, int*>>) {
                    // For shared memory, we need to implement a custom protocol
                    // This is a simple example, you might want to implement a
                    // more robust solution
                    if (*arg.second != 0) {
                        bytesRead = snprintf(buffer.data(), buffer.size(), "%d",
                                             *arg.second);
                        *arg.second = 0;  // Reset the shared memory
                    }
                }
            },
            impl_->driver.io);

        if (bytesRead > 0) {
            impl_->handleDriverOutput(impl_->driver.name,
                                      std::span(buffer.data(), bytesRead));
        }
    }
}

void StandAloneComponent::sendMessageToDriver(std::string_view message) {
    std::visit(
        [&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::pair<int, int>>) {
#if defined(_WIN32) || defined(_WIN64)
                _write(arg.first, message.data(),
                       static_cast<unsigned int>(message.size()));
#else
                write(arg.first, message.data(), message.size());
#endif
            } else if constexpr (std::is_same_v<T, std::pair<int, int*>>) {
                // For shared memory, we need to implement a custom protocol
                // This is a simple example, you might want to implement a more
                // robust solution
                *arg.second = std::atoi(message.data());
            } else if constexpr (std::is_same_v<T,
                                                std::pair<int, std::string>>) {
                // For string-based communication
                arg.second = message;
            } else if constexpr (std::is_same_v<
                                     T, std::pair<int, std::vector<char>>>) {
                // For vector-based communication
                arg.second.assign(message.begin(), message.end());
            } else if constexpr (std::is_same_v<
                                     T,
                                     std::pair<int,
                                               std::shared_ptr<std::string>>>) {
                // For shared pointer to string
                *arg.second = message;
            } else if constexpr (std::is_same_v<
                                     T,
                                     std::pair<int,
                                               std::unique_ptr<std::string>>>) {
                // For unique pointer to string
                *arg.second = message;
            }
        },
        impl_->driver.io);
}

void StandAloneComponent::printDriver() const {
    std::string interactionMethod;
    switch (impl_->driver.method) {
        case InteractionMethod::Pipe:
            interactionMethod = "Pipe";
            break;
        case InteractionMethod::FIFO:
            interactionMethod = "FIFO";
            break;
        case InteractionMethod::SharedMemory:
            interactionMethod = "Shared Memory";
            break;
    }

    LOG_F(INFO, "{} (PID: {}) {} [{}]", impl_->driver.name,
          impl_->driver.processHandle,
          impl_->driver.isListening ? "[Listening]" : "", interactionMethod);
}

void StandAloneComponent::toggleDriverListening() {
    impl_->driver.isListening = !impl_->driver.isListening;
    LOG_F(INFO, "Driver {} listening status: {}", impl_->driver.name,
          impl_->driver.isListening ? "ON" : "OFF");
}
