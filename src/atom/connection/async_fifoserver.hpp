/*
 * fifoserver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Server

*************************************************/

#ifndef ATOM_CONNECTION_ASYNC_FIFOSERVER_HPP
#define ATOM_CONNECTION_ASYNC_FIFOSERVER_HPP

#include <memory>
#include <string>

namespace atom::async::connection {

/**
 * @brief A class representing a server for handling FIFO messages.
 */
class FifoServer {
public:
    /**
     * @brief Constructs a new FifoServer object.
     *
     * @param fifo_path The path to the FIFO pipe.
     */
    explicit FifoServer(std::string_view fifo_path);

    /**
     * @brief Destroys the FifoServer object.
     */
    ~FifoServer();

    /**
     * @brief Starts the server to listen for messages.
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

}  // namespace atom::async::connection

#endif  // ATOM_CONNECTION_ASYNC_FIFOSERVER_HPP
