/*
 * ttybase.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-17

Description: TTY Base class

**************************************************/

#ifndef ATOM_SERIAL_TTYBASE_HPP
#define ATOM_SERIAL_TTYBASE_HPP

#include <hydrogenlogger.h>
#include <string>

/** \class TTYBase
    \brief Base class for serial communications

    This class is the C++ implementation of indicom serial functionality. Due to
   the idiosyncrasies of different serial implementation (including TCP/UDP),
    the base class methods can be overridden to provide specific implementations
   for a particular serial behavior.

    It provides methods to connect to and disconnect from serial devices,
   including TCP/UDP connections.

    \author Jasem Mutlaq
    \author Wildi Markus
*/

class TTYBase {
public:
    typedef enum {
        TTY_OK = 0,
        TTY_READ_ERROR = -1,
        TTY_WRITE_ERROR = -2,
        TTY_SELECT_ERROR = -3,
        TTY_TIME_OUT = -4,
        TTY_PORT_FAILURE = -5,
        TTY_PARAM_ERROR = -6,
        TTY_ERRNO = -7,
        TTY_OVERFLOW = -8
    } TTY_RESPONSE;

    TTYBase(const char *driverName);
    virtual ~TTYBase();

    /** \brief read buffer from terminal
        \param fd file descriptor
        \param buf pointer to store data. Must be initialized and big enough to
       hold data. \param nbytes number of bytes to read. \param timeout number
       of seconds to wait for terminal before a timeout error is issued. \param
       nbytes_read the number of bytes read. \return On success, it returns
       TTY_OK, otherwise, a TTY_ERROR code.
    */
    TTY_RESPONSE read(uint8_t *buffer, uint32_t nbytes, uint8_t timeout,
                      uint32_t *nbytes_read);

    /** \brief read buffer from terminal with a delimiter
        \param fd file descriptor
        \param buf pointer to store data. Must be initilized and big enough to
       hold data. \param stop_char if the function encounters \e stop_char then
       it stops reading and returns the buffer. \param nsize size of buf. If
       stop character is not encountered before nsize, the function aborts.
        \param timeout number of seconds to wait for terminal before a timeout
       error is issued. \param nbytes_read the number of bytes read. \return On
       success, it returns TTY_OK, otherwise, a TTY_ERROR code.
    */
    TTY_RESPONSE readSection(uint8_t *buffer, uint32_t nsize, uint8_t stop_byte,
                             uint8_t timeout, uint32_t *nbytes_read);

    /** \brief Writes a buffer to fd.
        \param fd file descriptor
        \param buffer a null-terminated buffer to write to fd.
        \param nbytes number of bytes to write from \e buffer
        \param nbytes_written the number of bytes written
        \return On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
    */
    TTY_RESPONSE write(const uint8_t *buffer, uint32_t nbytes,
                       uint32_t *nbytes_written);

    /** \brief Writes a null terminated string to fd.
        \param fd file descriptor
        \param buffer the buffer to write to fd.
        \param nbytes_written the number of bytes written
        \return On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
    */
    TTY_RESPONSE writeString(const char *string, uint32_t *nbytes_written);

    /** \brief Establishes a tty connection to a terminal device.
        \param device the device node. e.g. /dev/ttyS0
        \param bit_rate bit rate
        \param word_size number of data bits, 7 or 8, USE 8 DATA BITS with
       modbus \param parity 0=no parity, 1=parity EVEN, 2=parity ODD \param
       stop_bits number of stop bits : 1 or 2 \return On success, it returns
       TTY_OK, otherwise, a TTY_ERROR code.
    */

    TTY_RESPONSE connect(const char *device, uint32_t bit_rate,
                         uint8_t word_size, uint8_t parity, uint8_t stop_bits);

    /** \brief Closes a tty connection and flushes the bus.
        \return On success, it returns TTY_OK, otherwise, a TTY_ERROR code.
    */
    TTY_RESPONSE disconnect();

    /**
     * @brief setDebug Enable or Disable debug logging
     * @param enabled If true, TTY traffic will be logged.
     * @param channel Channel from HYDROGEN logger to log to.
     * @warning Only enable TTY debugging when diagnosting issues with serial
     * communications. Due to the verbose traffic generated from serial data,
     * this can have significant adverse effects on the function of the driver.
     * Use with caution!
     */
    void setDebug(HYDROGEN::Logger::VerbosityLevel channel);

    /** \brief Retrieve the tty error message
        \param err_code the error code return by any TTY function.
        \return Error message string
    */
    const std::string error(TTY_RESPONSE code) const;

    int getPortFD() const { return m_PortFD; }

private:
    TTY_RESPONSE checkTimeout(uint8_t timeout);

    int m_PortFD{-1};
    bool m_Debug{false};
    HYDROGEN::Logger::VerbosityLevel m_DebugChannel{
        HYDROGEN::Logger::DBG_IGNORE};
    const char *m_DriverName;
};

#endif
