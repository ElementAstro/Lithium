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

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace Atom::Connection {
/**
 * @brief The FifoServer class provides functionality to start a server that
 * listens on a FIFO (First In First Out) pipe, receive messages from the pipe,
 * and stop the server.
 */
class FifoServer {
public:
    /**
     * @brief Constructor for FifoServer.
     * @param fifoPath The path to the FIFO pipe.
     */
    FifoServer(const std::string &fifoPath);

    /**
     * @brief Starts the FIFO server to listen for incoming messages.
     */
    void start();

    /**
     * @brief Receives a message from the FIFO pipe.
     * @return The received message as a string.
     */
    std::string receiveMessage();

    /**
     * @brief Stops the FIFO server.
     */
    void stop();

private:
    std::string fifoPath; /**< The path to the FIFO pipe. */
    static const int bufferSize =
        1024; /**< The size of the buffer for receiving messages. */

#ifdef _WIN32
    HANDLE pipeHandle; /**< Handle to the pipe (Windows). */
#else
    int pipeFd; /**< File descriptor for the pipe (Unix/Linux). */
#endif
};

}  // namespace Atom::Connection

#endif  // ATOM_CONNECTION_FIFOSERVER_HPP
