/*
 * serial.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-17

Description: Serial Helper

**************************************************/

#ifndef ATOM_SERIAL_SERIAL_HPP
#define ATOM_SERIAL_SERIAL_HPP

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Atom::Serial {

// 校验位
enum class Parity {
    None,  // 无校验
    Odd,   // 奇校验
    Even   // 偶校验
};

// 停止位
enum class StopBits {
    One,  // 1位停止位
    Two   // 2位停止位
};

class SerialPort {
public:
    /**
     * @brief Constructs a SerialPort object.
     * @param portName The name of the serial port (e.g., "COM1").
     * @param baudRate The baud rate of the serial port.
     * @param dataBits The number of data bits (e.g., 8).
     * @param parity The parity bit (e.g., Parity::None).
     * @param stopBits The number of stop bits (e.g., StopBits::One).
     */
    explicit SerialPort(const std::string &portName, int baudRate, int dataBits,
                        Parity parity, StopBits stopBits);

    /**
     * @brief Destroys the SerialPort object.
     */
    ~SerialPort();

    /**
     * @brief Opens the serial port for communication.
     * @return true if the port is successfully opened, false otherwise.
     */
    bool open();

    /**
     * @brief Closes the serial port.
     */
    void close();

    /**
     * @brief Reads data from the serial port.
     * @param buffer The buffer to store the read data.
     * @param bufferSize The size of the buffer.
     * @return true if data is successfully read, false otherwise.
     */
    bool read(char *buffer, int bufferSize);

    /**
     * @brief Writes data to the serial port.
     * @param data The data to be written.
     * @param dataSize The size of the data.
     * @return true if data is successfully written, false otherwise.
     */
    bool write(const char *data, int dataSize);

private:
    /**
     * @brief Sets the parameters of the serial port.
     */
    void setParameters();

private:
    std::string portName_;
    int baudRate_;
    int dataBits_;
    Parity parity_;
    StopBits stopBits_;

#ifdef _WIN32
    HANDLE handle_;
#else
    int handle_;
#endif
};

/**
 * @brief A factory class for creating serial ports.
 */
class SerialPortFactory {
public:
    /**
     * @brief Gets a list of available serial ports.
     * @return A vector of strings containing the names of available serial ports.
     */
    static std::vector<std::string> getAvailablePorts();

    /**
     * @brief Creates a serial port with the specified parameters.
     * @param portName The name of the serial port.
     * @param baudRate The baud rate of the serial port.
     * @param dataBits The number of data bits.
     * @param parity The parity bit.
     * @param stopBits The number of stop bits.
     * @return A serial port object.
     */
    static SerialPort createSerialPort(const std::string &portName,
                                       int baudRate, int dataBits,
                                       Parity parity, StopBits stopBits);
};
}  // namespace Atom::Serial

#endif