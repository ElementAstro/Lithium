/*
 * fifoclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Client

*************************************************/

#ifndef ATOM_CONNECTION_FIFOCLIENT_HPP
#define ATOM_CONNECTION_FIFOCLIENT_HPP

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

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
    explicit FifoClient(std::string_view fifoPath);

    /**
     * @brief Destructor for the FifoClient object.
     */
    ~FifoClient();

    /**
     * @brief Writes data to the FIFO pipe.
     *
     * @param data The data to be written to the pipe.
     * @param timeout Optional timeout duration for the write operation.
     * @return True if the data was successfully written, false otherwise.
     */
    bool write(std::string_view data,
               std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    /**
     * @brief Reads data from the FIFO pipe.
     *
     * @param timeout Optional timeout duration for the read operation.
     * @return The data read from the pipe as a string, or an empty optional if
     * the read operation timed out.
     */
    std::optional<std::string> read(
        std::optional<std::chrono::milliseconds> timeout = std::nullopt);

    /**
     * @brief Checks if the FIFO pipe is open and ready for communication.
     *
     * @return True if the FIFO pipe is open, false otherwise.
     */
    [[nodiscard]] bool isOpen() const;

    /**
     * @brief Closes the FIFO pipe.
     */
    void close();

private:
#ifdef _WIN32
    HANDLE m_fifo{nullptr}; /**< Handle to the FIFO pipe (Windows). */
#else
    int m_fifo{-1}; /**< File descriptor for the FIFO pipe (Unix/Linux). */
#endif
    std::string m_fifoPath; /**< The path to the FIFO pipe. */
};
}  // namespace atom::connection

#endif  // ATOM_CONNECTION_FIFOCLIENT_HPP
