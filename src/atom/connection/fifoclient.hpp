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
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace atom::connection {
/**
 * @brief A class for interacting with a FIFO (First In, First Out) pipe.
 *
 * This class provides methods to read from and write to a FIFO pipe,
 * handling timeouts and ensuring proper resource management.
 */
class FifoClient {
public:
    /**
     * @brief Constructs a FifoClient with the specified FIFO path.
     *
     * @param fifoPath The path to the FIFO file to be used for communication.
     *
     * This constructor opens the FIFO and prepares the client for
     * reading and writing operations.
     */
    explicit FifoClient(std::string fifoPath);

    /**
     * @brief Destroys the FifoClient and closes the FIFO if it is open.
     *
     * This destructor ensures that all resources are released and the FIFO
     * is properly closed to avoid resource leaks.
     */
    ~FifoClient();

    /**
     * @brief Writes data to the FIFO.
     *
     * @param data The data to be written to the FIFO, as a string view.
     * @param timeout Optional timeout for the write operation, in milliseconds.
     *                If not provided, the default is no timeout.
     * @return true if the data was successfully written, false if there was an
     * error.
     *
     * This method will attempt to write the specified data to the FIFO.
     * If a timeout is specified, the operation will fail if it cannot complete
     * within the given duration.
     */
    auto write(std::string_view data,
               std::optional<std::chrono::milliseconds> timeout = std::nullopt)
        -> bool;

    /**
     * @brief Reads data from the FIFO.
     *
     * @param timeout Optional timeout for the read operation, in milliseconds.
     *                If not provided, the default is no timeout.
     * @return An optional string containing the data read from the FIFO.
     *         If there is an error or no data is available, returns
     * std::nullopt.
     *
     * This method will read data from the FIFO. If a timeout is specified,
     * it will return std::nullopt if the operation cannot complete within the
     * specified time.
     */
    auto read(std::optional<std::chrono::milliseconds> timeout = std::nullopt)
        -> std::optional<std::string>;

    /**
     * @brief Checks if the FIFO is currently open.
     *
     * @return true if the FIFO is open, false otherwise.
     *
     * This method can be used to determine if the FIFO client is ready for
     * operations.
     */
    [[nodiscard]] auto isOpen() const -> bool;

    /**
     * @brief Closes the FIFO.
     *
     * This method closes the FIFO and releases any associated resources.
     * It is good practice to call this when you are done using the FIFO
     * to ensure proper cleanup.
     */
    void close();

private:
    struct Impl;  ///< Forward declaration of the implementation details.
    std::unique_ptr<Impl> m_impl;  ///< Pointer to the implementation, using
                                   ///< PImpl idiom for encapsulation.
};

}  // namespace atom::connection

#endif  // ATOM_CONNECTION_FIFOCLIENT_HPP
