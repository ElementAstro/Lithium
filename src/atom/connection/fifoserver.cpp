/*
 * fifoserver.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Server

*************************************************/

#include "fifoserver.hpp"

#include <stdexcept>

#include "atom/log/loguru.hpp"

namespace Atom::Connection {

#ifdef _WIN32
FifoServer::FifoServer(const std::string &fifoPath)
    : fifoPath(fifoPath),
      pipeHandle(INVALID_HANDLE_VALUE)
#else
FifoServer::FifoServer(const std::string &fifoPath)
    : fifoPath(fifoPath),
      pipeFd()
#endif
{
}

void FifoServer::start() {
    DLOG_F(INFO, "Starting FIFO server...");

#ifdef _WIN32
    if (!WaitNamedPipeA(fifoPath.c_str(), NMPWAIT_WAIT_FOREVER)) {
        throw std::runtime_error("Failed to connect to FIFO");
    }

    pipeHandle =
        CreateNamedPipeA(fifoPath.c_str(), PIPE_ACCESS_INBOUND,
                         PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1,
                         bufferSize, bufferSize, 0, nullptr);

    if (pipeHandle == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create FIFO");
    }

    if (!ConnectNamedPipe(pipeHandle, nullptr)) {
        throw std::runtime_error("Failed to establish connection with client");
    }
#else
    if (mkfifo(fifoPath.c_str(), 0666) == -1) {
        throw std::runtime_error("Failed to create FIFO");
    }

    int fd = open(fifoPath.c_str(), O_RDONLY);
    if (fd == -1) {
        throw std::runtime_error("Failed to open FIFO");
    }

    pipeFd = fd;
#endif

    DLOG_F(INFO, "FIFO server started");
}

std::string FifoServer::receiveMessage() {
    DLOG_F(INFO, "Receiving message...");

    char buffer[bufferSize];

#ifdef _WIN32
    DWORD numBytesRead;
    if (!ReadFile(pipeHandle, buffer, bufferSize - 1, &numBytesRead, nullptr) ||
        numBytesRead == 0) {
        return "";
    }
#else
    ssize_t numBytesRead = read(pipeFd, buffer, bufferSize - 1);
    if (numBytesRead == -1 || numBytesRead == 0) {
        return "";
    }
#endif

    buffer[numBytesRead] = '\0';
    std::string receivedMessage(buffer);

    DLOG_F(INFO, "Received message: %s", receivedMessage.c_str());

    return receivedMessage;
}

void FifoServer::stop() {
    DLOG_F(INFO, "Stopping FIFO server...");

#ifdef _WIN32
    DisconnectNamedPipe(pipeHandle);
    CloseHandle(pipeHandle);
    DeleteFileA(fifoPath.c_str());
#else
    close(pipeFd);
    unlink(fifoPath.c_str());
#endif

    DLOG_F(INFO, "FIFO server stopped");
}
}  // namespace Atom::Connection
