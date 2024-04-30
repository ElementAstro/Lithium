/*
 * fifoclient.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO CLient

*************************************************/

#include "fifoclient.hpp"

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace atom::connection {
FifoClient::FifoClient(const std::string& fifoPath) : m_fifoPath(fifoPath) {
#ifdef _WIN32
    m_fifo = CreateFile(m_fifoPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0,
                        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
#elif __APPLE__
    // 在 macOS 上使用命名管道
    m_fifo = open(m_fifoPath.c_str(), O_RDWR);
#else
    m_fifo = open(m_fifoPath.c_str(), O_RDWR);
#endif
}

FifoClient::~FifoClient() {
#ifdef _WIN32
    CloseHandle(m_fifo);
#else
    close(m_fifo);
#endif
}

bool FifoClient::write(const std::string& data) {
    std::vector<char> buffer(data.begin(), data.end());
    buffer.push_back('\0');  // 添加字符串结束符

#ifdef _WIN32
    DWORD bytesWritten;
    return WriteFile(m_fifo, buffer.data(), static_cast<DWORD>(buffer.size()),
                     &bytesWritten, nullptr) != 0;
#else
    return write(m_fifo, buffer.data(), buffer.size()) != -1;
#endif
}

std::string FifoClient::read() {
    std::string data;
    char buffer[1024];

#ifdef _WIN32
    DWORD bytesRead;
    while (ReadFile(m_fifo, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) !=
               0 &&
           bytesRead != 0) {
        buffer[bytesRead] = '\0';
        data += buffer;
    }
#else
    ssize_t bytesRead;
    while ((bytesRead = read(m_fifo, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        data += buffer;
    }
#endif

    return data;
}

}  // namespace atom::connection
