#include "async_fifoclient.hpp"

#include <asio.hpp>
#include <iostream>
#include <string>
#include <system_error>

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
    asio::io_context io_context;
#ifdef _WIN32
    HANDLE fifoHandle{nullptr};
#else
    int fifoFd{-1};
#endif
    std::string fifoPath;
    asio::steady_timer timer;

    Impl(std::string_view path) : fifoPath(path), timer(io_context) {
        openFifo();
    }

    ~Impl() { close(); }

    void openFifo() {
#ifdef _WIN32
        fifoHandle =
            CreateFileA(fifoPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (fifoHandle == INVALID_HANDLE_VALUE) {
            throw std::runtime_error("Failed to open FIFO pipe");
        }
#else
        if (mkfifo(fifoPath.c_str(), 0666) == -1 && errno != EEXIST) {
            throw std::system_error(errno, std::generic_category(),
                                    "Failed to create FIFO");
        }
        fifoFd = open(fifoPath.c_str(), O_RDWR | O_NONBLOCK);
        if (fifoFd == -1) {
            throw std::system_error(errno, std::generic_category(),
                                    "Failed to open FIFO pipe");
        }
#endif
    }

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
               const std::optional<std::chrono::milliseconds>& timeout) {
        if (!isOpen())
            return false;

        // Convert data to buffer
        std::vector<char> buffer(data.begin(), data.end());
        buffer.push_back('\0');

#ifdef _WIN32
        // Windows specific writing logic
        DWORD bytesWritten;
        if (timeout) {
            timer.expires_after(*timeout);
            timer.async_wait(
                [this, &buffer, &bytesWritten](const asio::error_code&) {
                    WriteFile(fifoHandle, buffer.data(),
                              static_cast<DWORD>(buffer.size()), &bytesWritten,
                              nullptr);
                });
        } else {
            return WriteFile(fifoHandle, buffer.data(),
                             static_cast<DWORD>(buffer.size()), &bytesWritten,
                             nullptr) != 0;
        }
        io_context.run();
        io_context.reset();
        return true;
#else
        if (timeout) {
            fd_set writeFds;
            FD_ZERO(&writeFds);
            FD_SET(fifoFd, &writeFds);
            timeval tv{};
            tv.tv_sec = timeout->count() / 1000;
            tv.tv_usec = (timeout->count() % 1000) * 1000;
            int result = select(fifoFd + 1, nullptr, &writeFds, nullptr, &tv);
            if (result > 0) {
                return ::write(fifoFd, buffer.data(), buffer.size()) != -1;
            }
            return false;
        } else {
            return ::write(fifoFd, buffer.data(), buffer.size()) != -1;
        }
#endif
    }

    std::optional<std::string> read(
        const std::optional<std::chrono::milliseconds>& timeout) {
        if (!isOpen())
            return std::nullopt;

        std::string data;
        char buffer[1024];

#ifdef _WIN32
        // Windows specific reading logic
        DWORD bytesRead;
        if (timeout) {
            timer.expires_after(*timeout);
            timer.async_wait(
                [this, &data, &buffer, &bytesRead](const asio::error_code&) {
                    if (ReadFile(fifoHandle, buffer, sizeof(buffer) - 1,
                                 &bytesRead, nullptr) &&
                        bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        data += buffer;
                    }
                });
        } else {
            while (ReadFile(fifoHandle, buffer, sizeof(buffer) - 1, &bytesRead,
                            nullptr) &&
                   bytesRead > 0) {
                buffer[bytesRead] = '\0';
                data += buffer;
            }
        }
#else
        if (timeout) {
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
        } else {
            ssize_t bytesRead;
            while ((bytesRead = ::read(fifoFd, buffer, sizeof(buffer) - 1)) >
                   0) {
                buffer[bytesRead] = '\0';
                data += buffer;
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
