/*
 * udp_server.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A simple Asio-based UDP server.

*************************************************/

#include "async_udpserver.hpp"

#include <algorithm>
#include <asio.hpp>
#include <iostream>
#include <thread>


namespace atom::connection {

constexpr std::size_t BUFFER_SIZE = 1024;

class UdpSocketHub::Impl {
public:
    Impl() : socket_(io_context_), running_(false), data_{} {}

    ~Impl() { stop(); }

    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
    Impl(Impl&&) = delete;
    Impl& operator=(Impl&&) = delete;

    void start(unsigned short port) {
        if (running_) {
            return;
        }

        asio::ip::udp::endpoint endpoint(asio::ip::udp::v4(), port);
        socket_.open(endpoint.protocol());
        socket_.bind(endpoint);

        running_ = true;
        doReceive();

        io_thread_ = std::thread([this] { io_context_.run(); });
    }

    void stop() {
        if (!running_) {
            return;
        }

        running_ = false;
        socket_.close();
        io_context_.stop();

        if (io_thread_.joinable()) {
            io_thread_.join();
        }
    }

    [[nodiscard]] auto isRunning() const -> bool { return running_; }

    void addMessageHandler(MessageHandler handler) {
        handlers_.push_back(std::move(handler));
    }

    void removeMessageHandler(MessageHandler handler) {
        handlers_.erase(
            std::remove_if(
                handlers_.begin(), handlers_.end(),
                [&](const MessageHandler& handlerToRemove) {
                    return handler.target<void(const std::string&,
                                               const std::string&,
                                               unsigned short)>() ==
                           handlerToRemove.target<void(const std::string&,
                                                       const std::string&,
                                                       unsigned short)>();
                }),
            handlers_.end());
    }

    void sendTo(const std::string& message, const std::string& ipAddress,
                unsigned short port) {
        if (!running_) {
            std::cerr << "Server is not running." << std::endl;
            return;
        }

        asio::ip::udp::endpoint endpoint(asio::ip::make_address(ipAddress),
                                         port);
        socket_.async_send_to(
            asio::buffer(message), endpoint,
            [](std::error_code /*errorCode*/, std::size_t /*bytesSent*/) {});
    }

private:
    void doReceive() {
        socket_.async_receive_from(
            asio::buffer(data_), senderEndpoint_,
            [this](std::error_code errorCode, std::size_t bytesReceived) {
                if (!errorCode && bytesReceived > 0) {
                    std::string message(data_.data(), bytesReceived);
                    std::string senderIp =
                        senderEndpoint_.address().to_string();
                    unsigned short senderPort = senderEndpoint_.port();

                    for (const auto& handler : handlers_) {
                        handler(message, senderIp, senderPort);
                    }
                    doReceive();
                }
            });
    }

    asio::io_context io_context_;
    asio::ip::udp::socket socket_;
    asio::ip::udp::endpoint senderEndpoint_;
    std::array<char, BUFFER_SIZE> data_;
    std::vector<MessageHandler> handlers_;
    std::thread io_thread_;
    bool running_ = false;
};

UdpSocketHub::UdpSocketHub() : impl_(std::make_unique<Impl>()) {}

UdpSocketHub::~UdpSocketHub() = default;

void UdpSocketHub::start(unsigned short port) { impl_->start(port); }

void UdpSocketHub::stop() { impl_->stop(); }

auto UdpSocketHub::isRunning() const -> bool { return impl_->isRunning(); }

void UdpSocketHub::addMessageHandler(MessageHandler handler) {
    impl_->addMessageHandler(std::move(handler));
}

void UdpSocketHub::removeMessageHandler(MessageHandler handler) {
    impl_->removeMessageHandler(std::move(handler));
}

void UdpSocketHub::sendTo(const std::string& message,
                          const std::string& ipAddress, unsigned short port) {
    impl_->sendTo(message, ipAddress, port);
}

}  // namespace atom::connection