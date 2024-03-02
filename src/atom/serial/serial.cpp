/*
 * serial.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-17

Description: Serial Helper

**************************************************/

#include "serial.hpp"

#ifdef _WIN32
#include <winbase.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace Atom::Serial {
SerialPort::SerialPort(const std::string &portName, int baudRate, int dataBits,
                       Parity parity, StopBits stopBits)
    : portName_(portName),
      baudRate_(baudRate),
      dataBits_(dataBits),
      parity_(parity),
      stopBits_(stopBits),
      handle_(-1) {}

SerialPort::~SerialPort() { close(); }

bool SerialPort::open() {
#ifdef _WIN32
    handle_ = CreateFile(portName_.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                         NULL, OPEN_EXISTING, 0, NULL);
    if (handle_ == INVALID_HANDLE_VALUE) {
        return false;
    }
#else
    handle_ = ::open(portName_.c_str(), O_RDWR | O_NOCTTY);
    if (handle_ == -1) {
        return false;
    }
#endif

    setParameters();

    return true;
}

void SerialPort::close() {
    if (handle_ != -1) {
#ifdef _WIN32
        CloseHandle(handle_);
#else
        ::close(handle_);
#endif
        handle_ = -1;
    }
}

bool SerialPort::read(char *buffer, int bufferSize) {
#ifdef _WIN32
    DWORD bytesRead;
    if (!ReadFile(handle_, buffer, bufferSize, &bytesRead, NULL)) {
        // 处理读取失败的情况
        return false;
    }
#else
    ssize_t bytesRead = ::read(handle_, buffer, bufferSize);
    if (bytesRead == -1) {
        // 处理读取失败的情况
        return false;
    }
#endif

    return true;
}

bool SerialPort::write(const char *data, int dataSize) {
#ifdef _WIN32
    DWORD bytesWritten;
    if (!WriteFile(handle_, data, dataSize, &bytesWritten, NULL)) {
        // 处理写入失败的情况
        return false;
    }
#else
    ssize_t bytesWritten = ::write(handle_, data, dataSize);
    if (bytesWritten == -1) {
        // 处理写入失败的情况
        return false;
    }
#endif

    return true;
}

void SerialPort::setParameters() {
#ifdef _WIN32
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(handle_, &dcbSerialParams)) {
        // 处理获取串口状态失败的情况
        return;
    }

    // 设置波特率
    dcbSerialParams.BaudRate = baudRate_;

    // 设置数据位
    switch (dataBits_) {
        case 5:
            dcbSerialParams.ByteSize = 5;
            break;
        case 6:
            dcbSerialParams.ByteSize = 6;
            break;
        case 7:
            dcbSerialParams.ByteSize = 7;
            break;
        case 8:
            dcbSerialParams.ByteSize = 8;
            break;
        default:
            // 处理无效的数据位设置
            return;
    }

    // 设置奇偶校验
    switch (parity_) {
        case Parity::None:
            dcbSerialParams.Parity = NOPARITY;
            break;
        case Parity::Odd:
            dcbSerialParams.Parity = ODDPARITY;
            break;
        case Parity::Even:
            dcbSerialParams.Parity = EVENPARITY;
            break;
        default:
            // 处理无效的奇偶校验设置
            return;
    }

    // 设置停止位
    switch (stopBits_) {
        case StopBits::One:
            dcbSerialParams.StopBits = ONESTOPBIT;
            break;
        case StopBits::OnePointFive:
            dcbSerialParams.StopBits = ONE5STOPBITS;
            break;
        case StopBits::Two:
            dcbSerialParams.StopBits = TWOSTOPBITS;
            break;
        default:
            // 处理无效的停止位设置
            return;
    }

    if (!SetCommState(handle_, &dcbSerialParams)) {
        // 处理设置串口状态失败的情况
        return;
    }
#else
    struct termios options;
    if (tcgetattr(handle_, &options) == -1) {
        // 处理获取串口属性失败的情况
        return;
    }

    // 设置波特率
    speed_t baudRateConstant;
    switch (baudRate_) {
        case 9600:
            baudRateConstant = B9600;
            break;
        case 19200:
            baudRateConstant = B19200;
            break;
        case 38400:
            baudRateConstant = B38400;
            break;
        // 其他波特率常量...
        default:
            // 处理无效的波特率设置
            return;
    }
    cfsetispeed(&options, baudRateConstant);
    cfsetospeed(&options, baudRateConstant);

    // 设置数据位
    options.c_cflag &= ~CSIZE;
    switch (dataBits_) {
        case 5:
            options.c_cflag |= CS5;
            break;
        case 6:
            options.c_cflag |= CS6;
            break;
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            // 处理无效的数据位设置
            return;
    }

    // 设置奇偶校验
    switch (parity_) {
        case Parity::None:
            options.c_cflag &= ~PARENB;  // 禁用奇偶校验
            options.c_iflag &= ~INPCK;   // 禁用输入奇偶校验
            break;
        case Parity::Odd:
            options.c_cflag |= (PARENB | PARODD);  // 启用奇偶校验，奇校验
            options.c_iflag |= INPCK;              // 启用输入奇偶校验
            break;
        case Parity::Even:
            options.c_cflag |= PARENB;  // 启用奇偶校验，偶校验
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;  // 启用输入奇偶校验
            break;
        default:
            // 处理无效的奇偶校验设置

            return;
    }

    // 设置停止位
    switch (stopBits_) {
        case StopBits::One:
            options.c_cflag &= ~CSTOPB;  // 1位停止位
            break;
        case StopBits::Two:
            options.c_cflag |= CSTOPB;  // 2位停止位
            break;
        default:
            // 处理无效的停止位设置
            return;
    }

    if (tcsetattr(handle_, TCSANOW, &options) == -1) {
        // 处理设置串口属性失败的情况
        return;
    }
#endif
}

std::vector<std::string> SerialPortFactory::getAvailablePorts() {
    std::vector<std::string> ports;

#ifdef _WIN32
    for (int i = 1; i <= 256; i++) {
        std::string portName = "COM" + std::to_string(i);
        HANDLE handle =
            CreateFile(portName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL,
                       OPEN_EXISTING, 0, NULL);
        if (handle != INVALID_HANDLE_VALUE) {
            ports.push_back(portName);
            CloseHandle(handle);
        }
    }
#else
    DIR *dir = opendir("/dev");
    if (dir != NULL) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            std::string portName = entry->d_name;
            if (portName.find("tty") != std::string::npos ||
                portName.find("cu") != std::string::npos) {
                ports.push_back("/dev/" + portName);
            }
        }
        closedir(dir);
    }
#endif

    return ports;
}

SerialPort SerialPortFactory::createSerialPort(const std::string &portName,
                                               int baudRate, int dataBits,
                                               Parity parity,
                                               StopBits stopBits) {
    return SerialPort(portName, baudRate, dataBits, parity, stopBits);
}

}  // namespace Atom::Serial
