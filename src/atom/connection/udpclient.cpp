/*
 * udpclient.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-24

Description: UDP Client Class

*************************************************/

#include "udpclient.hpp"
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
class UdpClient::Impl {
public:
    Impl() {
#ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            THROW_RUNTIME_ERROR("WSAStartup failed");
        }
#endif
        socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (socket_ < 0) {
            THROW_RUNTIME_ERROR("Socket creation failed");
        }
    }

    ~Impl() {
#ifdef _WIN32
        closesocket(socket_);
        WSACleanup();
#else
        close(socket_);
#endif
    }

    bool bind(int port) {
        struct sockaddr_in address {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (::bind(socket_, reinterpret_cast<struct sockaddr*>(&address),
                   sizeof(address)) < 0) {
            errorMessage_ = "Bind failed";
            return false;
        }

        return true;
    }

    bool send(const std::string& host, int port,
              const std::vector<char>& data) {
        struct hostent* server = gethostbyname(host.c_str());
        if (server == nullptr) {
            errorMessage_ = "Host not found";
            return false;
        }

        struct sockaddr_in address {};
        address.sin_family = AF_INET;
        std::memcpy(&address.sin_addr.s_addr, server->h_addr, server->h_length);
        address.sin_port = htons(port);

        if (sendto(socket_, data.data(), data.size(), 0,
                   reinterpret_cast<struct sockaddr*>(&address),
                   sizeof(address)) < 0) {
            errorMessage_ = "Send failed";
            return false;
        }

        return true;
    }

    std::vector<char> receive(
        size_t size, std::string& remoteHost, int& remotePort,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) {
        if (timeout > std::chrono::milliseconds::zero()) {
            struct timeval tv;
            tv.tv_sec = timeout.count() / 1000;
            tv.tv_usec = (timeout.count() % 1000) * 1000;
            setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                       reinterpret_cast<const char*>(&tv), sizeof(tv));
        }

        std::vector<char> data(size);
        struct sockaddr_in clientAddress {};
        socklen_t clientAddressLength = sizeof(clientAddress);

        ssize_t bytesRead =
            recvfrom(socket_, data.data(), size, 0,
                     reinterpret_cast<struct sockaddr*>(&clientAddress),
                     &clientAddressLength);
        if (bytesRead < 0) {
            errorMessage_ = "Receive failed";
            return {};
        }

        data.resize(bytesRead);
        remoteHost = inet_ntoa(clientAddress.sin_addr);
        remotePort = ntohs(clientAddress.sin_port);

        return data;
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
            std::string remoteHost;
            int remotePort;
            std::vector<char> data =
                receive(bufferSize, remoteHost, remotePort);
            if (!data.empty() && onDataReceivedCallback_) {
                onDataReceivedCallback_(data, remoteHost, remotePort);
            }
        }
    }

#ifdef _WIN32
    SOCKET socket_;
#else
    int socket_;
#endif
    std::string errorMessage_;

    OnDataReceivedCallback onDataReceivedCallback_;
    OnErrorCallback onErrorCallback_;

    std::thread receivingThread_;
    bool receivingStopped_ = false;
};

UdpClient::UdpClient() : impl_(std::make_unique<Impl>()) {}

UdpClient::~UdpClient() = default;

bool UdpClient::bind(int port) { return impl_->bind(port); }

bool UdpClient::send(const std::string& host, int port,
                     const std::vector<char>& data) {
    return impl_->send(host, port, data);
}

std::vector<char> UdpClient::receive(size_t size, std::string& remoteHost,
                                     int& remotePort,
                                     std::chrono::milliseconds timeout) {
    return impl_->receive(size, remoteHost, remotePort, timeout);
}

void UdpClient::setOnDataReceivedCallback(
    const OnDataReceivedCallback& callback) {
    impl_->setOnDataReceivedCallback(callback);
}

void UdpClient::setOnErrorCallback(const OnErrorCallback& callback) {
    impl_->setOnErrorCallback(callback);
}

void UdpClient::startReceiving(size_t bufferSize) {
    impl_->startReceiving(bufferSize);
}

void UdpClient::stopReceiving() { impl_->stopReceiving(); }
}  // namespace atom::connection
