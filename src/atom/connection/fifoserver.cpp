/*
 * fifoserver.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Server

*************************************************/

#include "fifoserver.hpp"

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <queue>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace atom::connection {

class FIFOServer::Impl {
public:
    explicit Impl(std::string fifo_path) : fifo_path_(std::move(fifo_path)) {
        // 创建 FIFO 文件
#ifdef _WIN32
        CreateNamedPipeA(fifo_path_.c_str(), PIPE_ACCESS_DUPLEX,
                         PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                         PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
#elif __APPLE__ || __linux__
        mkfifo(fifo_path_.c_str(), 0666);
#endif
    }

    ~Impl() {
        stop();
        // 删除 FIFO 文件
#ifdef _WIN32
        DeleteFileA(fifo_path_.c_str());
#elif __APPLE__ || __linux__
        std::filesystem::remove(fifo_path_);
#endif
    }

    void sendMessage(std::string message) {
        {
            std::scoped_lock lock(queue_mutex_);
            message_queue_.push(std::move(message));
        }
        message_cv_.notify_one();
    }

    void start() {
        if (!server_thread_.joinable()) {
            stop_server_ = false;
            server_thread_ = std::thread([this] { serverLoop(); });
        }
    }

    void stop() {
        if (server_thread_.joinable()) {
            stop_server_ = true;
            message_cv_.notify_one();
            server_thread_.join();
        }
    }

    [[nodiscard]] bool isRunning() const { return server_thread_.joinable(); }

private:
    void serverLoop() {
        while (!stop_server_) {
            std::string message;
            {
                std::unique_lock lock(queue_mutex_);
                message_cv_.wait(lock, [this] {
                    return stop_server_ || !message_queue_.empty();
                });
                if (stop_server_) {
                    break;
                }
                message = std::move(message_queue_.front());
                message_queue_.pop();
            }

#ifdef _WIN32
            HANDLE pipe = CreateFileA(fifo_path_.c_str(), GENERIC_WRITE, 0,
                                      NULL, OPEN_EXISTING, 0, NULL);
            DWORD bytes_written;
            WriteFile(pipe, message.c_str(),
                      static_cast<DWORD>(message.length()), &bytes_written,
                      NULL);
            CloseHandle(pipe);
#elif __APPLE__ || __linux__
            int fd = open(fifo_path_.c_str(), O_WRONLY);
            write(fd, message.c_str(), message.length());
            close(fd);
#endif
        }
    }

    std::string fifo_path_;
    std::thread server_thread_;
    std::atomic_bool stop_server_ = true;
    std::queue<std::string> message_queue_;
    std::mutex queue_mutex_;
    std::condition_variable message_cv_;
};

FIFOServer::FIFOServer(std::string_view fifo_path)
    : impl_(std::make_unique<Impl>(std::move(fifo_path))) {}

FIFOServer::~FIFOServer() = default;

void FIFOServer::sendMessage(std::string message) {
    impl_->sendMessage(std::move(message));
}

void FIFOServer::start() { impl_->start(); }

void FIFOServer::stop() { impl_->stop(); }

bool FIFOServer::isRunning() const { return impl_->isRunning(); }

}  // namespace atom::connection
