/*
 * tcpclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-24

Description: TCP Client Class

*************************************************/

#ifndef ATOM_CONNECTION_TCPCLIENT_HPP
#define ATOM_CONNECTION_TCPCLIENT_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "atom/type/noncopyable.hpp"

namespace atom::connection {
/**
 * @class TcpClient
 * @brief Represents a TCP client for connecting to a server and
 * sending/receiving data.
 */
class TcpClient : public NonCopyable {
public:
    using OnConnectedCallback =
        std::function<void()>; /**< Type definition for connected callback
                                  function. */
    using OnDisconnectedCallback =
        std::function<void()>; /**< Type definition for disconnected callback
                                  function. */
    using OnDataReceivedCallback = std::function<void(
        const std::vector<char>&)>; /**< Type definition for data received
                                       callback function. */
    using OnErrorCallback =
        std::function<void(const std::string&)>; /**< Type definition for error
                                                    callback function. */

    /**
     * @brief Constructor.
     */
    TcpClient();

    /**
     * @brief Destructor.
     */
    ~TcpClient();

    /**
     * @brief Connects to a TCP server.
     * @param host The hostname or IP address of the server.
     * @param port The port number of the server.
     * @param timeout The connection timeout duration.
     * @return True if the connection is successful, false otherwise.
     */
    bool connect(
        const std::string& host, int port,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());

    /**
     * @brief Disconnects from the server.
     */
    void disconnect();

    /**
     * @brief Sends data to the server.
     * @param data The data to be sent.
     * @return True if the data is sent successfully, false otherwise.
     */
    bool send(const std::vector<char>& data);

    /**
     * @brief Receives data from the server.
     * @param size The number of bytes to receive.
     * @param timeout The receive timeout duration.
     * @return The received data.
     */
    std::vector<char> receive(
        size_t size,
        std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());

    /**
     * @brief Checks if the client is connected to the server.
     * @return True if connected, false otherwise.
     */
    bool isConnected() const;

    /**
     * @brief Gets the error message in case of any error.
     * @return The error message.
     */
    std::string getErrorMessage() const;

    /**
     * @brief Sets the callback function to be called when connected to the
     * server.
     * @param callback The callback function.
     */
    void setOnConnectedCallback(const OnConnectedCallback& callback);

    /**
     * @brief Sets the callback function to be called when disconnected from the
     * server.
     * @param callback The callback function.
     */
    void setOnDisconnectedCallback(const OnDisconnectedCallback& callback);

    /**
     * @brief Sets the callback function to be called when data is received from
     * the server.
     * @param callback The callback function.
     */
    void setOnDataReceivedCallback(const OnDataReceivedCallback& callback);

    /**
     * @brief Sets the callback function to be called when an error occurs.
     * @param callback The callback function.
     */
    void setOnErrorCallback(const OnErrorCallback& callback);

    /**
     * @brief Starts receiving data from the server.
     * @param bufferSize The size of the receive buffer.
     */
    void startReceiving(size_t bufferSize);

    /**
     * @brief Stops receiving data from the server.
     */
    void stopReceiving();

private:
    class Impl; /**< Forward declaration of the implementation class. */
    std::unique_ptr<Impl> impl_; /**< Pointer to the implementation object. */
};
}  // namespace atom::connection

#endif  // ATOM_CONNECTION_TCPCLIENT_HPP
