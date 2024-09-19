/*
 * udp_server.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A simple UDP server.

*************************************************/

#ifndef ATOM_CONNECTION_UDP_HPP
#define ATOM_CONNECTION_UDP_HPP

#include <functional>
#include <memory>
#include <string>

namespace atom::connection {
/**
 * @class UdpSocketHub
 * @brief Represents a hub for managing UDP sockets and message handling.
 */
class UdpSocketHub {
public:
    /**
     * @brief Type definition for message handler function.
     * @param message The message received.
     * @param ip The IP address of the sender.
     * @param port The port of the sender.
     */
    using MessageHandler =
        std::function<void(const std::string&, const std::string&, int)>;

    /**
     * @brief Constructor.
     */
    UdpSocketHub();

    /**
     * @brief Destructor.
     */
    ~UdpSocketHub();

    UdpSocketHub(const UdpSocketHub&) =
        delete; /**< Deleted copy constructor to prevent copying. */
    UdpSocketHub& operator=(const UdpSocketHub&) =
        delete; /**< Deleted copy assignment operator to prevent copying. */

    /**
     * @brief Starts the UDP socket hub and binds it to the specified port.
     * @param port The port on which the UDP socket hub will listen for incoming
     * messages.
     */
    void start(int port);

    /**
     * @brief Stops the UDP socket hub.
     */
    void stop();

    /**
     * @brief Checks if the UDP socket hub is currently running.
     * @return True if the UDP socket hub is running, false otherwise.
     */
    bool isRunning() const;

    /**
     * @brief Adds a message handler function to the UDP socket hub.
     * @param handler The message handler function to add.
     */
    void addMessageHandler(MessageHandler handler);

    /**
     * @brief Removes a message handler function from the UDP socket hub.
     * @param handler The message handler function to remove.
     */
    void removeMessageHandler(MessageHandler handler);

    /**
     * @brief Sends a message to the specified IP address and port.
     * @param message The message to send.
     * @param ip The IP address of the recipient.
     * @param port The port of the recipient.
     */
    void sendTo(const std::string& message, const std::string& ip, int port);

private:
    class Impl; /**< Forward declaration of the implementation class. */
    std::unique_ptr<Impl> impl_; /**< Pointer to the implementation object. */
};
}  // namespace atom::connection

#endif
