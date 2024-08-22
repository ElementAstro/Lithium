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

#include <system_error>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace atom::connection {
struct FifoClient::Impl {
#ifdef _WIN32
    HANDLE fifoHandle{nullptr};
#else
    int fifoFd{-1};
#endif
    std::string fifoPath;

#ifdef _WIN32
    Impl(std::string_view path) : fifoPath(path) {
        fifoHandle =
            CreateFileA(fifoPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fifoHandle == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Failed to open FIFO pipe");
    }
#else
    Impl(std::string_view path) : fifoPath(path) {
        fifoFd = open(fifoPath.c_str(), O_RDWR | O_NONBLOCK);
        if (fifoFd == -1)
            throw std::system_error(errno, std::generic_category(),
                                    "Failed to open FIFO pipe");
    }
#endif

    ~Impl() { close(); }

    bool isOpen() const {
#ifdef _WIN32
        return fifoHandle != INVALID_HANDLE_VALUE;
#else
        return fifoFd != -1;
#endif
    }

    void close() {
#ifdef _WIN32
        if (isOpen()) {
            CloseHandle(fifoHandle);
            fifoHandle = INVALID_HANDLE_VALUE;
        }
#else
        if (isOpen()) {
            ::close(fifoFd);
            fifoFd = -1;
        }
#endif
    }

    bool write(std::string_view data,
               std::optional<std::chrono::milliseconds> timeout) {
        std::vector<char> buffer(data.begin(), data.end());
        buffer.push_back('\0');

#ifdef _WIN32
        DWORD bytesWritten;
        if (timeout) {
            COMMTIMEOUTS timeouts{};
            timeouts.WriteTotalTimeoutConstant =
                static_cast<DWORD>(timeout->count());
            SetCommTimeouts(fifoHandle, &timeouts);
            bool success = WriteFile(fifoHandle, buffer.data(),
                                     static_cast<DWORD>(buffer.size()),
                                     &bytesWritten, nullptr) != 0;
            timeouts.WriteTotalTimeoutConstant = 0;
            SetCommTimeouts(fifoHandle, &timeouts);
            return success;
        }
        return WriteFile(fifoHandle, buffer.data(),
                         static_cast<DWORD>(buffer.size()), &bytesWritten,
                         nullptr) != 0;
#else
        if (!timeout) {
            return ::write(fifoFd, buffer.data(), buffer.size()) != -1;
        } else {
            fd_set writeFds;
            FD_ZERO(&writeFds);
            FD_SET(fifoFd, &writeFds);
            timeval tv{};
            tv.tv_sec = timeout->count() / 1000;
            tv.tv_usec = (timeout->count() % 1000) * 1000;
            int result = select(fifoFd + 1, nullptr, &writeFds, nullptr, &tv);
            if (result <= 0)
                return false;
            return ::write(fifoFd, buffer.data(), buffer.size()) != -1;
        }
#endif
    }

    std::optional<std::string> read(
        std::optional<std::chrono::milliseconds> timeout) {
        std::string data;
        char buffer[1024];

#ifdef _WIN32
        DWORD bytesRead;
        if (timeout) {
            COMMTIMEOUTS timeouts{};
            timeouts.ReadTotalTimeoutConstant =
                static_cast<DWORD>(timeout->count());
            SetCommTimeouts(fifoHandle, &timeouts);
            if (ReadFile(fifoHandle, buffer, sizeof(buffer) - 1, &bytesRead,
                         nullptr) &&
                bytesRead > 0) {
                buffer[bytesRead] = '\0';
                data += buffer;
            }
            timeouts.ReadTotalTimeoutConstant = 0;
            SetCommTimeouts(fifoHandle, &timeouts);
        } else {
            while (ReadFile(fifoHandle, buffer, sizeof(buffer) - 1, &bytesRead,
                            nullptr) &&
                   bytesRead > 0) {
                buffer[bytesRead] = '\0';
                data += buffer;
            }
        }
#else
        if (!timeout) {
            ssize_t bytesRead;
            while ((bytesRead = ::read(fifoFd, buffer, sizeof(buffer) - 1)) >
                   0) {
                buffer[bytesRead] = '\0';
                data += buffer;
            }
        } else {
            fd_set readFds;
            FD_ZERO(&readFds);
            FD_SET(fifoFd, &readFds);
            timeval tv{};
            tv.tv_sec = timeout->count() / 1000;
            tv.tv_usec = (timeout->count() % 1000) * 1000;
            int result = select(fifoFd + 1, &readFds, nullptr, nullptr, &tv);
            if (result > 0) {
                ssize_t bytesRead = ::read(fifoFd, buffer, sizeof(buffer) - 1);
                if (bytesRead > 0) {
                    buffer[bytesRead] = '\0';
                    data += buffer;
                }
            }
        }
#endif

        return data.empty() ? std::nullopt : std::make_optional(data);
    }
};

FifoClient::FifoClient(std::string fifoPath)
    : m_impl(std::make_unique<Impl>(fifoPath)) {}
FifoClient::~FifoClient() = default;

bool FifoClient::write(std::string_view data,
                       std::optional<std::chrono::milliseconds> timeout) {
    return m_impl->write(data, timeout);
}

std::optional<std::string> FifoClient::read(
    std::optional<std::chrono::milliseconds> timeout) {
    return m_impl->read(timeout);
}

bool FifoClient::isOpen() const { return m_impl->isOpen(); }

void FifoClient::close() { m_impl->close(); }

}  // namespace atom::connection