/*
 * fifoserver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Server

*************************************************/

#ifndef ATOM_CONNECTION_FIFOSERVER_HPP
#define ATOM_CONNECTION_FIFOSERVER_HPP

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace Atom::Connection {
class FifoServer {
public:
    FifoServer(const std::string &fifoPath);

    void start();
    std::string receiveMessage();
    void stop();

private:
    std::string fifoPath;
    static const int bufferSize = 1024;

#ifdef _WIN32
    HANDLE pipeHandle;
#else
    int pipeFd;
#endif
};
}  // namespace Atom::Connection

#endif  // ATOM_CONNECTION_FIFOSERVER_HPP
