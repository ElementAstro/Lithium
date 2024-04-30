/*
 * protector.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: License Protector, Class for monitoring changes to license files or
directories and taking action upon changes.

**************************************************/

#include "protector.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <thread>


#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <sys/inotify.h>
#include <unistd.h>

#endif

#include "atom/log/loguru.hpp"

namespace lithium {
/**
 * @class LicenseProtectorImpl
 * @brief 实现文件或目录监控的具体逻辑。
 *        Implements the specific logic for monitoring a file or directory.
 *
 * @details
 * 这个类负责监控指定路径下的文件或目录变化，并在检测到变化时执行相应操作。它是LicenseProtector的具体实现部分，使用Pimpl模式与之分离。
 *          This class is responsible for monitoring changes in a specified file
 * or directory path and performing corresponding actions upon detection. It
 * serves as the concrete implementation part of LicenseProtector, separated by
 * the Pimpl idiom.
 */
class LicenseProtectorImpl {
public:
    /**
     * @brief 构造函数。Constructor.
     *
     * @param path 要监控的文件或目录的路径。Path to the file or directory to be
     * monitored.
     */
    explicit LicenseProtectorImpl(const std::string &path);

    /**
     * @brief 析构函数。Destructor.
     *
     * @details 负责清理资源，例如停止监控线程。Responsible for cleaning up
     * resources, such as stopping the monitoring thread.
     */
    ~LicenseProtectorImpl();

    /**
     * @brief 开始监控文件或目录的变化。Starts monitoring changes to the file or
     * directory.
     */
    void startMonitoring();

    /**
     * @brief 停止监控文件或目录的变化。Stops monitoring changes to the file or
     * directory.
     */
    void stopMonitoring();

private:
    std::string filePath;  ///< 监控的文件或目录路径。Path of the file or
                           ///< directory being monitored.
    std::atomic<bool> stopFlag;  ///< 控制监控线程停止的标志。Flag to control
                                 ///< the termination of the monitoring thread.

#if __cplusplus >= 201703L
    std::jthread monitorThread;  ///< C++20及以上版本使用的监控线程。The
                                 ///< monitoring thread used in C++20 and later.
#else
    std::thread monitorThread;  ///< C++11至C++17版本使用的监控线程。The
                                ///< monitoring thread used in C++11 to C++17.
#endif

#ifdef _WIN32
    HANDLE hStopEvent;  ///< Windows平台用于停止监控线程的事件句柄。Event handle
                        ///< used to stop the monitoring thread on Windows
                        ///< platform.
#endif

    /**
     * @brief 监控循环的主体函数。The main function of the monitoring loop.
     */
    void monitor();

#ifdef _WIN32
    /**
     * @brief Windows平台上的监控实现。Monitoring implementation on Windows
     * platform.
     */
    void monitorWindows();
#else
    /**
     * @brief Linux平台上的监控实现。Monitoring implementation on Linux
     * platform.
     */
    void monitorLinux();
#endif

    /**
     * @brief 当检测到删除事件时调用的处理函数。Handler function called upon
     * detection of a delete event.
     */
    void onDelete();
};

LicenseProtectorImpl::LicenseProtectorImpl(const std::string &path)
    : filePath(path), stopFlag(false) {
    if (!std::filesystem::is_directory(filePath)) {
        LOG_F(ERROR, "Path must be a directory.");
        throw std::runtime_error("Path must be a directory.");
    }
#ifdef _WIN32
    hStopEvent = nullptr;
#endif
}

LicenseProtectorImpl::~LicenseProtectorImpl() { stopMonitoring(); }

void LicenseProtectorImpl::startMonitoring() {
    if (monitorThread.joinable()) {
        LOG_F(ERROR, "Monitoring is already running.");
        return;
    }
    stopFlag = false;
#ifdef _WIN32
    if (!hStopEvent) {
        hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (hStopEvent == NULL) {
            LOG_F(ERROR, "Failed to create stop event.");
            return;
        }
    }
#endif
#if __cplusplus >= 202002L
    monitorThread = std::jthread(&LicenseProtectorImpl::monitor, this);
#else
    monitorThread = std::thread(&LicenseProtectorImpl::monitor, this);
#endif
}

void LicenseProtectorImpl::stopMonitoring() {
    stopFlag = true;
    if (monitorThread.joinable()) {
#ifdef _WIN32
        SetEvent(hStopEvent);
#endif
        monitorThread.join();
    }
}

void LicenseProtectorImpl::monitor() {
#ifdef _WIN32
    monitorWindows();
#else
    monitorLinux();
#endif
}
void LicenseProtectorImpl::onDelete() {
    try {
        std::filesystem::remove(filePath);
        DLOG_F(INFO, "File deleted: {}", filePath);
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Error deleting file: {}", e.what());
    }
}

#ifdef _WIN32
void LicenseProtectorImpl::monitorWindows() {
    HANDLE hDir = CreateFile(
        filePath.c_str(),  // 注意：这里假设filePath是目录路径
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

    if (hDir == INVALID_HANDLE_VALUE) {
        LOG_F(ERROR, "Failed to open directory handle.");
        return;
    }

    // 创建一个事件用于停止监控
    hStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hStopEvent == NULL) {
        LOG_F(ERROR, "Failed to create stop event.");
        CloseHandle(hDir);
        return;
    }

    char buffer[1024];
    DWORD bytesReturned;
    OVERLAPPED overlapped = {};
    overlapped.hEvent =
        CreateEvent(NULL, TRUE, FALSE, NULL);  // 用于目录更改通知的事件

    BOOL result = ReadDirectoryChangesW(
        hDir, &buffer, sizeof(buffer),
        TRUE,  // 监控目录及其子目录
        FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
        &bytesReturned, &overlapped,
        NULL);  // 使用重叠I/O

    if (!result) {
        LOG_F(ERROR, "Failed to start directory changes monitoring.");
        CloseHandle(hDir);
        CloseHandle(hStopEvent);
        CloseHandle(overlapped.hEvent);
        return;
    }

    HANDLE handles[2] = {overlapped.hEvent, hStopEvent};

    while (true) {
        DWORD waitStatus = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        if (waitStatus == WAIT_OBJECT_0 + 1) {
            // hStopEvent 被触发，退出循环
            break;
        } else if (waitStatus == WAIT_OBJECT_0) {
            // 处理目录更改通知
            onDelete();  // 或者根据实际情况处理通知
            ResetEvent(overlapped.hEvent);

            // 重新启动目录更改监控
            ReadDirectoryChangesW(
                hDir, &buffer, sizeof(buffer), TRUE,
                FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE,
                &bytesReturned, &overlapped, NULL);
        }
    }

    CloseHandle(hDir);
    CloseHandle(hStopEvent);
    CloseHandle(overlapped.hEvent);
}

#else
void LicenseProtectorImpl::monitorLinux() {
    int fd = inotify_init1(IN_NONBLOCK);  // 使用非阻塞模式
    if (fd < 0) {
        LOG_F(ERROR, "Failed to initialize inotify.");
        return;
    }

    int wd =
        inotify_add_watch(fd, filePath.c_str(), IN_DELETE_SELF | IN_MODIFY);
    if (wd < 0) {
        LOG_F(ERROR, "Failed to add inotify watch.");
        close(fd);
        return;
    }

    const size_t buf_len = sizeof(struct inotify_event) + NAME_MAX + 1;
    char buf[buf_len];

    while (!stopFlag.load()) {
        ssize_t len = read(fd, buf, buf_len);
        if (len < 0) {
            if (errno == EAGAIN) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(100));  // 等待一段时间再尝试
                continue;
            }
            LOG_F(ERROR, "Failed to read inotify events.");
            break;
        }

        struct inotify_event *event =
            reinterpret_cast<struct inotify_event *>(buf);
        if ((event->mask & IN_DELETE_SELF) || (event->mask & IN_MODIFY)) {
            onDelete();  // Call onDelete if the file is deleted or modified
        }
    }

    inotify_rm_watch(fd, wd);
    close(fd);
}

#endif

LicenseProtector::LicenseProtector(const std::string &path)
    : pImpl(std::make_unique<LicenseProtectorImpl>(path)) {}

LicenseProtector::~LicenseProtector() =
    default;  // Default destructor is fine due to unique_ptr

void LicenseProtector::startMonitoring() { pImpl->startMonitoring(); }

void LicenseProtector::stopMonitoring() { pImpl->stopMonitoring(); }

}  // namespace lithium
