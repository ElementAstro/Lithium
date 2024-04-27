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

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

namespace Atom::Connection {

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
    explicit FIFOServer(const std::string& fifo_path);

    /**
     * @brief Destroys the FIFOServer object.
     */
    ~FIFOServer();

    /**
     * @brief Sends a message through the FIFO pipe.
     *
     * @param message The message to be sent.
     */
    void sendMessage(const std::string& message);

private:
    /**
     * @brief The main server loop function.
     */
    void serverLoop();

    std::string fifo_path_;                 /**< The path to the FIFO pipe. */
    std::thread server_thread_;             /**< The server thread. */
    std::atomic_bool stop_server_ = false;  /**< Flag to stop the server. */
    std::queue<std::string> message_queue_; /**< Queue for storing messages. */
    std::mutex queue_mutex_; /**< Mutex for message queue synchronization. */
    std::condition_variable
        message_cv_; /**< Condition variable for message synchronization. */
};

}  // namespace Atom::Connection

#endif  // ATOM_CONNECTION_FIFOSERVER_HPP
