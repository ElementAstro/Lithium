/*
 * fifoclient.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO CLient

*************************************************/

#ifndef ATOM_CONNECTION_FIFOCLIENT_HPP
#define ATOM_CONNECTION_FIFOCLIENT_HPP

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace Atom::Connection
{
    class FifoClient
    {
    public:
        FifoClient(const std::string &fifoPath);

        void connect();
        void sendMessage(const std::string &message);
        void disconnect();

    private:
        std::string fifoPath;

#ifdef _WIN32
        HANDLE pipeHandle;
#else
        int pipeFd;
#endif
    };
}

#endif // FIFOSERVER_H
