/*
 * udpclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-24

Description: UDP Client Class

*************************************************/

#ifndef ATOM_CONNECTION_UDPCLIENT_HPP
#define ATOM_CONNECTION_UDPCLIENT_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace atom::connection {
/**
 * @class UdpClient
 * @brief Represents a UDP client for sending and receiving datagrams.
 */
class UdpClient {
public:
    using OnDataReceivedCallback = std::function<void(
        const std::vector<char>&, const std::string&,
        int)>; /**< Type definition for data received callback function. */
    using OnErrorCallback =
        std::function<void(const std::string&)>; /**< Type definition for error
                                                    callback function. */

    /**
     * @brief Constructor.
     */
    UdpClient();

    /**
     * @brief Destructor.
     */
    ~UdpClient();

    /**
     * @brief Deleted copy constructor to prevent copying.
     */
    UdpClient(const UdpClient&) = delete;

    /**
     * @brief Deleted copy assignment operator to prevent copying.
     */
    UdpClient& operator=(const UdpClient&) = delete;

    /**
     * @brief Binds the client to a specific port for receiving data.
     * @param port The port number to bind to.
     * @return True if the binding is successful, false otherwise.
     */
    bool bind(int port);

    /**
     * @brief Sends data to a specified host and port.
     * @param host The destination host address.
     * @param port The destination port number.
     * @param data The data to be sent.
     * @return True if the data is sent successfully, false otherwise.
     */
    bool send(const std::string& host, int port, const std::vector<char>& data);

    /**
     * @brief Receives data from a remote host.
     * @param size The number of bytes to receive.
     * @param remoteHost The hostname or IP address of the remote host.
     * @param remotePort The port number of the remote host.
     * @param timeout The receive timeout duration.
     * @return The received data.
     */
    std::vector<char> receive(
        size_t size, std::string& remoteHost, int& remotePort,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());

    /**
     * @brief Sets the callback function to be called when data is received.
     * @param callback The callback function.
     */
    void setOnDataReceivedCallback(const OnDataReceivedCallback& callback);

    /**
     * @brief Sets the callback function to be called when an error occurs.
     * @param callback The callback function.
     */
    void setOnErrorCallback(const OnErrorCallback& callback);

    /**
     * @brief Starts receiving data asynchronously.
     * @param bufferSize The size of the receive buffer.
     */
    void startReceiving(size_t bufferSize);

    /**
     * @brief Stops receiving data.
     */
    void stopReceiving();

private:
    class Impl; /**< Forward declaration of the implementation class. */
    std::unique_ptr<Impl> impl_; /**< Pointer to the implementation object. */
};
}  // namespace atom::connection
#endif  // ATOM_CONNECTION_UDPCLIENT_HPP
