/*
 * fifoserver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Server

*************************************************/

#ifndef ATOM_CONNECTION_FIFOSERVER_HPP
#define ATOM_CONNECTION_FIFOSERVER_HPP

#include <memory>
#include <string>

namespace atom::connection {

/**
 * @brief A class representing a server for handling FIFO messages.
 */
class FIFOServer {
public:
    /**
     * @brief Constructs a new FIFOServer object.
     *
     * @param fifo_path The path to the FIFO pipe.
     */
    explicit FIFOServer(std::string_view fifo_path);

    /**
     * @brief Destroys the FIFOServer object.
     */
    ~FIFOServer();

    /**
     * @brief Sends a message through the FIFO pipe.
     *
     * @param message The message to be sent.
     */
    void sendMessage(std::string message);

    /**
     * @brief Starts the server.
     */
    void start();

    /**
     * @brief Stops the server.
     */
    void stop();

    /**
     * @brief Checks if the server is running.
     *
     * @return True if the server is running, false otherwise.
     */
    [[nodiscard]] bool isRunning() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace atom::connection

#endif  // ATOM_CONNECTION_FIFOSERVER_HPP
