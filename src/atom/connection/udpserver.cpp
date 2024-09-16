/*
 * udp_server.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A simple UDP server.

*************************************************/

#include "udpserver.hpp"

#include <algorithm>
#include <mutex>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "atom/log/loguru.hpp"

namespace atom::connection {
class UdpSocketHub::Impl {
public:
    Impl() : running_(false), socket_(INVALID_SOCKET) {}

    ~Impl() { stop(); }

    void start(int port) {
        if (running_.load()) {
            return;
        }

        if (!initNetworking()) {
            LOG_F(ERROR, "Networking initialization failed.");
            return;
        }

        socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_ == INVALID_SOCKET) {
            LOG_F(ERROR, "Failed to create socket.");
            cleanupNetworking();
            return;
        }

        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (bind(socket_, reinterpret_cast<sockaddr*>(&serverAddr),
                 sizeof(serverAddr)) == SOCKET_ERROR) {
            LOG_F(ERROR, "Bind failed with error.");
            closeSocket();
            cleanupNetworking();
            return;
        }

        running_.store(true);
        receiverThread_ = std::jthread([this] { receiveMessages(); });
    }

    void stop() {
        if (!running_.load()) {
            return;
        }

        running_.store(false);
        closeSocket();
        cleanupNetworking();

        if (receiverThread_.joinable()) {
            receiverThread_.join();
        }
    }

    bool isRunning() const { return running_.load(); }

    void addMessageHandler(MessageHandler handler) {
        std::scoped_lock lock(handlersMutex_);
        handlers_.push_back(std::move(handler));
    }

    void removeMessageHandler(MessageHandler handler) {
        std::scoped_lock lock(handlersMutex_);
        auto it = std::find_if(
            handlers_.begin(), handlers_.end(),
            [&handler](const MessageHandler& h) {
                return handler.target_type() == h.target_type() &&
                       handler.target<void(const std::string&,
                                           const std::string&, int)>() ==
                           h.target<void(const std::string&, const std::string&,
                                         int)>();
            });
        if (it != handlers_.end()) {
            handlers_.erase(it);
        }
    }

    void sendTo(const std::string& message, const std::string& ip, int port) {
        if (!running_.load()) {
            LOG_F(ERROR, "Server is not running.");
            return;
        }

        sockaddr_in targetAddr{};
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &targetAddr.sin_addr);

        if (sendto(socket_, message.data(), message.size(), 0,
                   reinterpret_cast<sockaddr*>(&targetAddr),
                   sizeof(targetAddr)) == SOCKET_ERROR) {
            LOG_F(ERROR, "Failed to send message.");
        }
    }

private:
    bool initNetworking() {
#ifdef _WIN32
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
        return true;
#endif
    }

    void cleanupNetworking() {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void closeSocket() {
#ifdef _WIN32
        closesocket(socket_);
#else
        close(socket_);
#endif
        socket_ = INVALID_SOCKET;
    }

    void receiveMessages() {
        char buffer[1024];
        sockaddr_in clientAddr{};
        socklen_t clientAddrSize = sizeof(clientAddr);

        while (running_.load()) {
            const auto bytesReceived = recvfrom(
                socket_, buffer, sizeof(buffer), 0,
                reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrSize);
            if (bytesReceived == SOCKET_ERROR) {
                LOG_F(ERROR, "recvfrom failed with error.");
                continue;
            }

            std::string message(buffer, bytesReceived);
            std::string clientIp = inet_ntoa(clientAddr.sin_addr);
            int clientPort = ntohs(clientAddr.sin_port);

            std::scoped_lock lock(handlersMutex_);
            for (const auto& handler : handlers_) {
                handler(message, clientIp, clientPort);
            }
        }
    }

    std::atomic<bool> running_;
    SOCKET socket_;
    std::jthread receiverThread_;
    std::vector<MessageHandler> handlers_;
    std::mutex handlersMutex_;
};

UdpSocketHub::UdpSocketHub() : impl_(std::make_unique<Impl>()) {}

UdpSocketHub::~UdpSocketHub() = default;

void UdpSocketHub::start(int port) { impl_->start(port); }

void UdpSocketHub::stop() { impl_->stop(); }

bool UdpSocketHub::isRunning() const { return impl_->isRunning(); }

void UdpSocketHub::addMessageHandler(MessageHandler handler) {
    impl_->addMessageHandler(std::move(handler));
}

void UdpSocketHub::removeMessageHandler(MessageHandler handler) {
    impl_->removeMessageHandler(std::move(handler));
}

void UdpSocketHub::sendTo(const std::string& message, const std::string& ip,
                          int port) {
    impl_->sendTo(message, ip, port);
}
}  // namespace atom::connection
