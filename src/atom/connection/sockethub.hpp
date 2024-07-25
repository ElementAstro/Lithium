/*
 * sockethub.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: SocketHub class for managing socket connections.

*************************************************/

#ifndef ATOM_CONNECTION_SOCKETHUB_HPP
#define ATOM_CONNECTION_SOCKETHUB_HPP

#include <functional>
#include <memory>
#include <string>

namespace atom::connection {

class SocketHubImpl;

/**
 * @class SocketHub
 * @brief Manages socket connections.
 *
 * The SocketHub class is responsible for managing socket connections.
 * It provides functionality to start and stop the socket service, and
 * handles multiple client connections. For each client, it spawns a
 * thread to handle incoming messages. The class allows for adding
 * custom message handlers that are called when a message is received
 * from a client.
 */
class SocketHub {
public:
    /**
     * @brief Constructs a SocketHub instance.
     */
    SocketHub();

    /**
     * @brief Destroys the SocketHub instance.
     *
     * Cleans up resources and stops any ongoing socket operations.
     */
    ~SocketHub();

    /**
     * @brief Starts the socket service.
     * @param port The port number on which the socket service will listen.
     *
     * Initializes the socket service and starts listening for incoming
     * connections on the specified port. It spawns threads to handle
     * each connected client.
     */
    void start(int port);

    /**
     * @brief Stops the socket service.
     *
     * Shuts down the socket service, closes all client connections,
     * and stops any running threads associated with handling client
     * messages.
     */
    void stop();

    /**
     * @brief Adds a message handler.
     * @param handler A function to handle incoming messages from clients.
     *
     * The provided handler function will be called with the received
     * message as a string parameter. Multiple handlers can be added
     * and will be called in the order they are added.
     */
    void addHandler(std::function<void(std::string)> handler);

    /**
     * @brief Checks if the socket service is currently running.
     * @return True if the socket service is running, false otherwise.
     *
     * This method returns the status of the socket service, indicating
     * whether it is currently active and listening for connections.
     */
    [[nodiscard]] auto isRunning() const -> bool;

private:
    std::unique_ptr<SocketHubImpl>
        impl_;  ///< Pointer to the implementation details of SocketHub.
};

}  // namespace atom::connection

#endif  // ATOM_CONNECTION_SOCKETHUB_HPP
