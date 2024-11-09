#ifndef ATOM_CONNECTION_ASYNC_FIFOCLIENT_HPP
#define ATOM_CONNECTION_ASYNC_FIFOCLIENT_HPP

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace atom::async::connection {

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
     */
    explicit FifoClient(std::string fifoPath);

    /**
     * @brief Destroys the FifoClient and closes the FIFO if it is open.
     */
    ~FifoClient();

    /**
     * @brief Writes data to the FIFO.
     *
     * @param data The data to be written to the FIFO, as a string view.
     * @param timeout Optional timeout for the write operation, in milliseconds.
     * @return true if the data was successfully written, false if there was an
     * error.
     */
    auto write(std::string_view data,
               std::optional<std::chrono::milliseconds> timeout = std::nullopt)
        -> bool;

    /**
     * @brief Reads data from the FIFO.
     *
     * @param timeout Optional timeout for the read operation, in milliseconds.
     * @return An optional string containing the data read from the FIFO.
     */
    auto read(std::optional<std::chrono::milliseconds> timeout = std::nullopt)
        -> std::optional<std::string>;

    /**
     * @brief Checks if the FIFO is currently open.
     *
     * @return true if the FIFO is open, false otherwise.
     */
    [[nodiscard]] auto isOpen() const -> bool;

    /**
     * @brief Closes the FIFO.
     */
    void close();

private:
    struct Impl;  ///< Forward declaration of the implementation details
    std::unique_ptr<Impl> m_impl;  ///< Pointer to the implementation
};

}  // namespace atom::connection

#endif  // ATOM_CONNECTION_ASYNC_FIFOCLIENT_HPP
