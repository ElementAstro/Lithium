/*
 * fifoserver.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Server

*************************************************/

#include "async_fifoserver.hpp"

#include <asio.hpp>
#include <filesystem>
#include <iostream>

namespace atom::async::connection {

class FifoServer::Impl {
public:
    explicit Impl(std::string_view fifo_path)
        : fifo_path_(fifo_path), io_context_(), fifo_stream_(io_context_) {
#if __APPLE__ || __linux__
        // Create FIFO if it doesn't exist
        if (!std::filesystem::exists(fifo_path_)) {
            mkfifo(fifo_path_.c_str(), 0666);
        }
#endif
    }

    ~Impl() {
        stop();
#if __APPLE__ || __linux__
        std::filesystem::remove(fifo_path_);
#endif
    }

    void start() {
        if (!isRunning()) {
            running_ = true;
            io_thread_ = std::thread([this]() { io_context_.run(); });
            acceptConnection();
        }
    }

    void stop() {
        if (isRunning()) {
            running_ = false;
            io_context_.stop();
            if (io_thread_.joinable()) {
                io_thread_.join();
            }
        }
    }

    [[nodiscard]] bool isRunning() const { return running_; }

private:
    void acceptConnection() {
#if __APPLE__ || __linux__
        fifo_stream_.assign(open(fifo_path_.c_str(), O_RDWR | O_NONBLOCK));
        readMessage();
#endif
    }

    void readMessage() {
#if __APPLE__ || __linux__
        asio::async_read_until(
            fifo_stream_, asio::dynamic_buffer(buffer_), '\n',
            [this](std::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string message(buffer_.substr(0, length));
                    buffer_.erase(0, length);
                    std::cout << "Received message: " << message << std::endl;
                    readMessage();  // Continue reading
                }
            });
#endif
    }

    std::string fifo_path_;
    asio::io_context io_context_;
#ifdef _WIN32
    asio::windows::stream_handle fifo_stream_;
#else
    asio::posix::stream_descriptor fifo_stream_;
#endif
    std::thread io_thread_;
    std::string buffer_;
    bool running_ = false;
};

FifoServer::FifoServer(std::string_view fifo_path)
    : impl_(std::make_unique<Impl>(fifo_path)) {}

FifoServer::~FifoServer() = default;

void FifoServer::start() { impl_->start(); }

void FifoServer::stop() { impl_->stop(); }

bool FifoServer::isRunning() const { return impl_->isRunning(); }

}  // namespace atom::async::connection
