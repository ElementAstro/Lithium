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

#include <stdexcept>

#include "atom/log/loguru.hpp"

namespace Atom::Connection
{
#ifdef _WIN32
    FifoClient::FifoClient(const std::string &fifoPath) : fifoPath(fifoPath), pipeHandle(INVALID_HANDLE_VALUE)
#else
    FifoClient::FifoClient(const std::string &fifoPath) : fifoPath(fifoPath), pipeFd()
#endif
    {
    }

    void FifoClient::connect()
    {
        DLOG_F(INFO, "Connecting to FIFO...");

#ifdef _WIN32
        if (!WaitNamedPipeA(fifoPath.c_str(), NMPWAIT_WAIT_FOREVER))
        {
            throw std::runtime_error("Failed to connect to FIFO");
        }

        pipeHandle = CreateFileA(
            fifoPath.c_str(),
            GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (pipeHandle == INVALID_HANDLE_VALUE)
        {
            throw std::runtime_error("Failed to open FIFO");
        }
#else
        int fd = open(fifoPath.c_str(), O_WRONLY);
        if (fd == -1)
        {
            throw std::runtime_error("Failed to open FIFO");
        }

        pipeFd = fd;
#endif

        DLOG_F(INFO, "Connected to FIFO");
    }

    void FifoClient::sendMessage(const std::string &message)
    {
        DLOG_F(INFO, "Sending message...");

#ifdef _WIN32
        DWORD numBytesWritten;
        if (!WriteFile(
                pipeHandle,
                message.c_str(),
                message.length(),
                &numBytesWritten,
                nullptr) ||
            numBytesWritten != message.length())
        {
            throw std::runtime_error("Failed to write message to FIFO");
        }
#else
        ssize_t numBytesWritten = write(pipeFd, message.c_str(), message.length());
        if (numBytesWritten == -1 || static_cast<size_t>(numBytesWritten) != message.length())
        {
            throw std::runtime_error("Failed to write message to FIFO");
        }
#endif

        DLOG_F(INFO, "Message sent");
    }

    void FifoClient::disconnect()
    {
        DLOG_F(INFO, "Disconnecting from FIFO...");

#ifdef _WIN32
        CloseHandle(pipeHandle);
#else
        close(pipeFd);
#endif

        DLOG_F(INFO, "Disconnected from FIFO");
    }
} // namespace Atom::Connection
