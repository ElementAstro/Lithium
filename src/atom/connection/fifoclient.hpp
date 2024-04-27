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

namespace atom::connection {
/**
 * @brief The FifoClient class provides functionality to connect to a FIFO
 *        (First In First Out) pipe, write data to the pipe, and read data from
 *        the pipe.
 */
class FifoClient {
public:
    /**
     * @brief Constructs a FifoClient object with the specified FIFO path.
     *
     * @param fifoPath The path to the FIFO pipe.
     */
    FifoClient(const std::string& fifoPath);

    /**
     * @brief Destructor for the FifoClient object.
     */
    ~FifoClient();

    /**
     * @brief Writes data to the FIFO pipe.
     *
     * @param data The data to be written to the pipe.
     * @return True if the data was successfully written, false otherwise.
     */
    bool write(const std::string& data);

    /**
     * @brief Reads data from the FIFO pipe.
     *
     * @return The data read from the pipe as a string.
     */
    std::string read();

private:
#ifdef _WIN32
    HANDLE m_fifo; /**< Handle to the FIFO pipe (Windows). */
#else
    int m_fifo; /**< File descriptor for the FIFO pipe (Unix/Linux). */
#endif
    std::string m_fifoPath; /**< The path to the FIFO pipe. */
};
}  // namespace atom::connection

#endif  // FIFOSERVER_H
