#ifndef ATOM_CONNECTION_TTYBASE_HPP
#define ATOM_CONNECTION_TTYBASE_HPP

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>
#include <string_view>

// Windows specific includes
#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

/**
 * @class TTYBase
 * @brief Provides a base class for handling TTY (Teletypewriter) connections.
 *
 * This class serves as an interface for reading from and writing to TTY
 * devices, handling various responses and errors associated with the
 * communication.
 */
class TTYBase {
public:
    /**
     * @enum TTYResponse
     * @brief Enumerates possible responses from TTY operations.
     */
    enum class TTYResponse {
        OK = 0,            ///< Operation completed successfully.
        ReadError = -1,    ///< Error occurred while reading from the TTY.
        WriteError = -2,   ///< Error occurred while writing to the TTY.
        SelectError = -3,  ///< Error occurred while selecting the TTY device.
        Timeout = -4,      ///< Operation timed out.
        PortFailure = -5,  ///< Failed to connect to the TTY port.
        ParamError = -6,   ///< Invalid parameters provided to a function.
        Errno = -7,        ///< An error occurred as indicated by errno.
        Overflow = -8      ///< Buffer overflow occurred during an operation.
    };

    /**
     * @brief Constructs a TTYBase instance with the specified driver name.
     *
     * @param driverName The name of the TTY driver to be used.
     */
    explicit TTYBase(std::string_view driverName) : m_DriverName(driverName) {}

    /**
     * @brief Destructor for TTYBase.
     *
     * Cleans up resources associated with the TTY connection.
     */
    virtual ~TTYBase();

    /**
     * @brief Reads data from the TTY device.
     *
     * @param buffer Pointer to the buffer where read data will be stored.
     * @param nbytes The number of bytes to read from the TTY.
     * @param timeout Timeout duration for the read operation in seconds.
     * @param nbytesRead Reference to store the actual number of bytes read.
     * @return TTYResponse indicating the result of the read operation.
     */
    TTYResponse read(uint8_t* buffer, uint32_t nbytes, uint8_t timeout,
                     uint32_t& nbytesRead);

    /**
     * @brief Reads a section of data from the TTY until a stop byte is
     * encountered.
     *
     * @param buffer Pointer to the buffer where read data will be stored.
     * @param nsize The maximum number of bytes to read.
     * @param stopByte The byte value that will stop the reading.
     * @param timeout Timeout duration for the read operation in seconds.
     * @param nbytesRead Reference to store the actual number of bytes read.
     * @return TTYResponse indicating the result of the read operation.
     */
    TTYResponse readSection(uint8_t* buffer, uint32_t nsize, uint8_t stopByte,
                            uint8_t timeout, uint32_t& nbytesRead);

    /**
     * @brief Writes data to the TTY device.
     *
     * @param buffer Pointer to the data to be written.
     * @param nbytes The number of bytes to write to the TTY.
     * @param nbytesWritten Reference to store the actual number of bytes
     * written.
     * @return TTYResponse indicating the result of the write operation.
     */
    TTYResponse write(const uint8_t* buffer, uint32_t nbytes,
                      uint32_t& nbytesWritten);

    /**
     * @brief Writes a string to the TTY device.
     *
     * @param string The string to be written to the TTY.
     * @param nbytesWritten Reference to store the actual number of bytes
     * written.
     * @return TTYResponse indicating the result of the write operation.
     */
    TTYResponse writeString(std::string_view string, uint32_t& nbytesWritten);

    /**
     * @brief Connects to the specified TTY device.
     *
     * @param device The device name or path to connect to.
     * @param bitRate The baud rate for the connection.
     * @param wordSize The data size (in bits) of each character.
     * @param parity The parity checking mode (e.g. none, odd, even).
     * @param stopBits The number of stop bits to use in communication.
     * @return TTYResponse indicating the result of the connection attempt.
     */
    TTYResponse connect(std::string_view device, uint32_t bitRate,
                        uint8_t wordSize, uint8_t parity, uint8_t stopBits);

    /**
     * @brief Disconnects from the TTY device.
     *
     * @return TTYResponse indicating the result of the disconnection.
     */
    TTYResponse disconnect();

    /**
     * @brief Enables or disables debugging information.
     *
     * @param enabled true to enable debugging, false to disable it.
     */
    void setDebug(bool enabled);

    /**
     * @brief Retrieves an error message corresponding to a given TTYResponse
     * code.
     *
     * @param code The TTYResponse code for which to get the error message.
     * @return A string containing the error message.
     */
    std::string getErrorMessage(TTYResponse code) const;

    /**
     * @brief Gets the file descriptor for the TTY port.
     *
     * @return The integer file descriptor for the TTY port.
     */
    int getPortFD() const { return m_PortFD; }

private:
    /**
     * @brief Checks for timeouts.
     *
     * @param timeout The timeout duration to check.
     * @return TTYResponse indicating the result of the timeout check.
     */
    TTYResponse checkTimeout(uint8_t timeout);

    int m_PortFD{-1};     ///< File descriptor for the TTY port.
    bool m_Debug{false};  ///< Flag indicating whether debugging is enabled.
    std::string_view m_DriverName;  ///< The name of the driver for this TTY.
};

#endif
