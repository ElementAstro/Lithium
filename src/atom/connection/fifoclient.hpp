/*
 * fifoclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO CLient

*************************************************/

#ifndef ATOM_CONNECTION_FIFOCLIENT_HPP
#define ATOM_CONNECTION_FIFOCLIENT_HPP

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
 * @brief The FifoClient class provides functionality to connect to a FIFO
 * (First In First Out) pipe, send messages through the pipe, and disconnect
 * from the pipe.
 */
class FifoClient {
public:
    /**
     * @brief Constructor for FifoClient.
     * @param fifoPath The path to the FIFO pipe.
     */
    explicit FifoClient(const std::string &fifoPath);

    /**
     * @brief Connects to the FIFO pipe.
     */
    void connect();

    /**
     * @brief Sends a message through the FIFO pipe.
     * @param message The message to send.
     */
    void sendMessage(const std::string &message);

    /**
     * @brief Disconnects from the FIFO pipe.
     */
    void disconnect();

private:
    std::string fifoPath; /**< The path to the FIFO pipe. */

#ifdef _WIN32
    HANDLE pipeHandle; /**< Handle to the pipe (Windows). */
#else
    int pipeFd; /**< File descriptor for the pipe (Unix/Linux). */
#endif
};

}  // namespace Atom::Connection

#endif  // FIFOSERVER_H
