/*
 * udpclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************
Date: 2024-5-24
Description: UDP Client Class
*************************************************/

#ifndef ATOM_CONNECTION_ASYNC_UDPCLIENT_HPP
#define ATOM_CONNECTION_ASYNC_UDPCLIENT_HPP

#include <asio.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace atom::async::connection {

/**
 * @class UdpClient
 * @brief Represents a UDP client for sending and receiving datagrams.
 */
class UdpClient {
public:
    using OnDataReceivedCallback =
        std::function<void(const std::vector<char>&, const std::string&, int)>;
    using OnErrorCallback = std::function<void(const std::string&)>;

    UdpClient();
    ~UdpClient();

    UdpClient(const UdpClient&) = delete;
    UdpClient& operator=(const UdpClient&) = delete;

    bool bind(int port);
    bool send(const std::string& host, int port, const std::vector<char>& data);
    std::vector<char> receive(
        size_t size, std::string& remoteHost, int& remotePort,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());

    void setOnDataReceivedCallback(const OnDataReceivedCallback& callback);
    void setOnErrorCallback(const OnErrorCallback& callback);

    void startReceiving(size_t bufferSize);
    void stopReceiving();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace atom::async::connection
#endif  // ATOM_CONNECTION_ASYNC_UDPCLIENT_HPP
