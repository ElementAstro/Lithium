/*
 * fifoclient.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Client

*************************************************/

#include "fifoclient.hpp"

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace atom::connection {
FifoClient::FifoClient(std::string_view fifoPath) : m_fifoPath(fifoPath) {
#ifdef _WIN32
    m_fifo =
        CreateFileA(m_fifoPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                    nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (m_fifo == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to open FIFO pipe");
    }
#else
    m_fifo = open(m_fifoPath.c_str(), O_RDWR | O_NONBLOCK);
    if (m_fifo == -1) {
        throw std::runtime_error("Failed to open FIFO pipe");
    }
#endif
}

FifoClient::~FifoClient() { close(); }

bool FifoClient::write(std::string_view data,
                       std::optional<std::chrono::milliseconds> timeout) {
    std::vector<char> buffer(data.begin(), data.end());
    buffer.push_back('\0');  // Add null terminator

#ifdef _WIN32
    DWORD bytesWritten;
    if (!timeout.has_value()) {
        return WriteFile(m_fifo, buffer.data(),
                         static_cast<DWORD>(buffer.size()), &bytesWritten,
                         nullptr) != 0;
    } else {
        COMMTIMEOUTS timeouts{};
        timeouts.WriteTotalTimeoutConstant =
            static_cast<DWORD>(timeout->count());
        SetCommTimeouts(m_fifo, &timeouts);
        bool success =
            WriteFile(m_fifo, buffer.data(), static_cast<DWORD>(buffer.size()),
                      &bytesWritten, nullptr) != 0;
        timeouts.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(m_fifo, &timeouts);
        return success;
    }
#else
    if (!timeout.has_value()) {
        return ::write(m_fifo, buffer.data(), buffer.size()) != -1;
    } else {
        fd_set writeFds;
        FD_ZERO(&writeFds);
        FD_SET(m_fifo, &writeFds);
        timeval tv{};
        tv.tv_sec = timeout->count() / 1000;
        tv.tv_usec = (timeout->count() % 1000) * 1000;
        int selectResult = select(m_fifo + 1, nullptr, &writeFds, nullptr, &tv);
        if (selectResult == -1) {
            return false;
        } else if (selectResult == 0) {
            return false;  // Timeout occurred
        } else {
            return ::write(m_fifo, buffer.data(), buffer.size()) != -1;
        }
    }
#endif
}

std::optional<std::string> FifoClient::read(
    std::optional<std::chrono::milliseconds> timeout) {
    std::string data;
    char buffer[1024];

#ifdef _WIN32
    DWORD bytesRead;
    if (!timeout.has_value()) {
        while (ReadFile(m_fifo, buffer, sizeof(buffer) - 1, &bytesRead,
                        nullptr) != 0 &&
               bytesRead != 0) {
            buffer[bytesRead] = '\0';
            data += buffer;
        }
    } else {
        COMMTIMEOUTS timeouts{};
        timeouts.ReadTotalTimeoutConstant =
            static_cast<DWORD>(timeout->count());
        SetCommTimeouts(m_fifo, &timeouts);
        bool success = ReadFile(m_fifo, buffer, sizeof(buffer) - 1, &bytesRead,
                                nullptr) != 0;
        if (success && bytesRead != 0) {
            buffer[bytesRead] = '\0';
            data += buffer;
        }
        timeouts.ReadTotalTimeoutConstant = 0;
        SetCommTimeouts(m_fifo, &timeouts);
    }
#else
    if (!timeout.has_value()) {
        ssize_t bytesRead;
        while ((bytesRead = ::read(m_fifo, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            data += buffer;
        }
    } else {
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(m_fifo, &readFds);
        timeval tv{};
        tv.tv_sec = timeout->count() / 1000;
        tv.tv_usec = (timeout->count() % 1000) * 1000;
        int selectResult = select(m_fifo + 1, &readFds, nullptr, nullptr, &tv);
        if (selectResult == -1) {
            // Error occurred
        } else if (selectResult == 0) {
            // Timeout occurred
        } else {
            ssize_t bytesRead = ::read(m_fifo, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                data += buffer;
            }
        }
    }
#endif

    return data.empty() ? std::nullopt : std::make_optional(data);
}

bool FifoClient::isOpen() const {
#ifdef _WIN32
    return m_fifo != INVALID_HANDLE_VALUE;
#else
    return m_fifo != -1;
#endif
}

void FifoClient::close() {
    if (isOpen()) {
#ifdef _WIN32
        CloseHandle(m_fifo);
        m_fifo = INVALID_HANDLE_VALUE;
#else
        ::close(m_fifo);
        m_fifo = -1;
#endif
    }
}

}  // namespace atom::connection
