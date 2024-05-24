/*
 * tcpclient.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-24

Description: TCP Client Class

*************************************************/

#include "tcpclient.hpp"

#include <cstring>
#include <stdexcept>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "atom/error/exception.hpp"

namespace atom::connection {
class TcpClient::Impl {
public:
    Impl() {
#ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            THROW_RUNTIME_ERROR("WSAStartup failed");
        }
#endif
        socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_ < 0) {
            THROW_RUNTIME_ERROR("Socket creation failed");
        }
    }

    ~Impl() {
        disconnect();
#ifdef _WIN32
        WSACleanup();
#endif
    }

    bool connect(const std::string& host, int port,
                 std::chrono::milliseconds timeout) {
        struct hostent* server = gethostbyname(host.c_str());
        if (server == nullptr) {
            errorMessage_ = "Host not found";
            return false;
        }

        struct sockaddr_in serverAddress {};
        serverAddress.sin_family = AF_INET;
        std::memcpy(&serverAddress.sin_addr.s_addr, server->h_addr,
                    server->h_length);
        serverAddress.sin_port = htons(port);

        if (timeout > std::chrono::milliseconds::zero()) {
            struct timeval tv;
            tv.tv_sec = timeout.count() / 1000;
            tv.tv_usec = (timeout.count() % 1000) * 1000;
            setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&tv), sizeof(tv));
            setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO,
                       reinterpret_cast<const char*>(&tv), sizeof(tv));
        }

        if (::connect(socket_,
                      reinterpret_cast<struct sockaddr*>(&serverAddress),
                      sizeof(serverAddress)) < 0) {
            errorMessage_ = "Connection failed";
            return false;
        }

        connected_ = true;
        return true;
    }

    void disconnect() {
        if (connected_) {
#ifdef _WIN32
            closesocket(socket_);
#else
            close(socket_);
#endif
            connected_ = false;
        }
    }

    bool send(const std::vector<char>& data) {
        if (!connected_) {
            errorMessage_ = "Not connected";
            return false;
        }

        if (::send(socket_, data.data(), data.size(), 0) < 0) {
            errorMessage_ = "Send failed";
            return false;
        }

        return true;
    }

    std::vector<char> receive(
        size_t size,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) {
        if (timeout > std::chrono::milliseconds::zero()) {
            struct timeval tv;
            tv.tv_sec = timeout.count() / 1000;
            tv.tv_usec = (timeout.count() % 1000) * 1000;
            setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&tv), sizeof(tv));
        }

        std::vector<char> data(size);
        ssize_t bytesRead = ::recv(socket_, data.data(), size, 0);
        if (bytesRead < 0) {
            errorMessage_ = "Receive failed";
            return {};
        }
        data.resize(bytesRead);
        return data;
    }

    [[nodiscard]] bool isConnected() const { return connected_; }

    [[nodiscard]] std::string getErrorMessage() const { return errorMessage_; }

    void setOnConnectedCallback(const OnConnectedCallback& callback) {
        onConnectedCallback_ = callback;
    }

    void setOnDisconnectedCallback(const OnDisconnectedCallback& callback) {
        onDisconnectedCallback_ = callback;
    }

    void setOnDataReceivedCallback(const OnDataReceivedCallback& callback) {
        onDataReceivedCallback_ = callback;
    }

    void setOnErrorCallback(const OnErrorCallback& callback) {
        onErrorCallback_ = callback;
    }

    void startReceiving(size_t bufferSize) {
        stopReceiving();
        receivingThread_ = std::thread(&Impl::receivingLoop, this, bufferSize);
    }

    void stopReceiving() {
        if (receivingThread_.joinable()) {
            receivingStopped_ = true;
            receivingThread_.join();
            receivingStopped_ = false;
        }
    }

private:
    void receivingLoop(size_t bufferSize) {
        while (!receivingStopped_) {
            std::vector<char> data = receive(bufferSize);
            if (!data.empty() && onDataReceivedCallback_) {
                onDataReceivedCallback_(data);
            }
        }
    }

#ifdef _WIN32
    SOCKET socket_;
#else
    int socket_;
#endif
    bool connected_ = false;
    std::string errorMessage_;

    OnConnectedCallback onConnectedCallback_;
    OnDisconnectedCallback onDisconnectedCallback_;
    OnDataReceivedCallback onDataReceivedCallback_;
    OnErrorCallback onErrorCallback_;

    std::thread receivingThread_;
    bool receivingStopped_ = false;
};

TcpClient::TcpClient() : impl_(std::make_unique<Impl>()) {}

TcpClient::~TcpClient() = default;

bool TcpClient::connect(const std::string& host, int port,
                        std::chrono::milliseconds timeout) {
    return impl_->connect(host, port, timeout);
}

void TcpClient::disconnect() { impl_->disconnect(); }

bool TcpClient::send(const std::vector<char>& data) {
    return impl_->send(data);
}

std::vector<char> TcpClient::receive(size_t size,
                                     std::chrono::milliseconds timeout) {
    return impl_->receive(size, timeout);
}

bool TcpClient::isConnected() const { return impl_->isConnected(); }

std::string TcpClient::getErrorMessage() const {
    return impl_->getErrorMessage();
}

void TcpClient::setOnConnectedCallback(const OnConnectedCallback& callback) {
    impl_->setOnConnectedCallback(callback);
}

void TcpClient::setOnDisconnectedCallback(
    const OnDisconnectedCallback& callback) {
    impl_->setOnDisconnectedCallback(callback);
}

void TcpClient::setOnDataReceivedCallback(
    const OnDataReceivedCallback& callback) {
    impl_->setOnDataReceivedCallback(callback);
}

void TcpClient::setOnErrorCallback(const OnErrorCallback& callback) {
    impl_->setOnErrorCallback(callback);
}

void TcpClient::startReceiving(size_t bufferSize) {
    impl_->startReceiving(bufferSize);
}

void TcpClient::stopReceiving() { impl_->stopReceiving(); }
}  // namespace atom::connection
