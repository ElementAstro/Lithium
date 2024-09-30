/*
 * udp_server.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A simple Asio-based UDP server.

*************************************************/

#ifndef ATOM_CONNECTION_ASYNC_UDPSERVER_HPP
#define ATOM_CONNECTION_ASYNC_UDPSERVER_HPP

#include <functional>
#include <memory>
#include <string>

namespace atom::connection {

/**
 * @class UdpSocketHub
 * @brief Represents a hub for managing UDP sockets and message handling using
 * Asio.
 */
class UdpSocketHub {
public:
    using MessageHandler = std::function<void(
        const std::string&, const std::string&, unsigned short)>;

    UdpSocketHub();
    ~UdpSocketHub();

    UdpSocketHub(const UdpSocketHub&) = delete;
    UdpSocketHub& operator=(const UdpSocketHub&) = delete;

    void start(unsigned short port);
    void stop();
    bool isRunning() const;

    void addMessageHandler(MessageHandler handler);
    void removeMessageHandler(MessageHandler handler);
    void sendTo(const std::string& message, const std::string& ip,
                unsigned short port);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace atom::connection

#endif
