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

#include <filesystem>
#include <iostream>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace atom::connection {

FIFOServer::FIFOServer(const std::string& fifo_path) : fifo_path_(fifo_path) {
// 创建 FIFO 文件
#ifdef _WIN32
    CreateNamedPipeA(fifo_path_.c_str(), PIPE_ACCESS_DUPLEX,
                     PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                     PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, NULL);
#elif __APPLE__ || __linux__
    mkfifo(fifo_path_.c_str(), 0666);
#endif

    // 启动服务器线程
    server_thread_ = std::thread([this] { serverLoop(); });
}

FIFOServer::~FIFOServer() {
    // 停止服务器线程
    stop_server_ = true;
    server_thread_.join();

// 删除 FIFO 文件
#ifdef _WIN32
    DeleteFileA(fifo_path_.c_str());
#elif __APPLE__ || __linux__
    std::filesystem::remove(fifo_path_);
#endif
}

void FIFOServer::sendMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    message_queue_.push(message);
    message_cv_.notify_one();
}

void FIFOServer::serverLoop() {
    while (!stop_server_) {
        std::string message;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            message_cv_.wait(lock, [this] { return !message_queue_.empty(); });
            message = message_queue_.front();
            message_queue_.pop();
        }

#ifdef _WIN32
        HANDLE pipe = CreateFileA(fifo_path_.c_str(), GENERIC_WRITE, 0, NULL,
                                  OPEN_EXISTING, 0, NULL);
        DWORD bytes_written;
        WriteFile(pipe, message.c_str(), message.length(), &bytes_written,
                  NULL);
        CloseHandle(pipe);
#elif __APPLE__
        int fd = open(fifo_path_.c_str(), O_WRONLY);
        write(fd, message.c_str(), message.length());
        close(fd);
#elif __linux__
        int fd = open(fifo_path_.c_str(), O_WRONLY);
        write(fd, message.c_str(), message.length());
        close(fd);
#endif
    }
}
}  // namespace atom::connection
