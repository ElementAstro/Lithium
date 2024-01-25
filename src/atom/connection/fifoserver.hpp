/*
 * fifoserver.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: FIFO Server

*************************************************/

#ifndef FIFOSERVER_H
#define FIFOSERVER_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

class FifoServer
{
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

#endif // FIFOSERVER_H
