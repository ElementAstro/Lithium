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

#include <vector>
#include <string>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Atom::Serial
{

    // 校验位
    enum class Parity
    {
        None, // 无校验
        Odd,  // 奇校验
        Even  // 偶校验
    };

    // 停止位
    enum class StopBits
    {
        One, // 1位停止位
        Two  // 2位停止位
    };

    class SerialPort
    {
    public:
        explicit SerialPort(const std::string &portName, int baudRate, int dataBits, Parity parity, StopBits stopBits);
        
        ~SerialPort();

        bool open();

        void close();

        bool read(char *buffer, int bufferSize);

        bool write(const char *data, int dataSize);

    private:
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

    class SerialPortFactory
    {
    public:
        static std::vector<std::string> getAvailablePorts();

        static SerialPort createSerialPort(const std::string &portName, int baudRate, int dataBits, Parity parity, StopBits stopBits);
    };
}

#endif