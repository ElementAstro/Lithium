/*
 * fifoclient.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-6-1

Description: FIFO CLient

*************************************************/

#ifndef FIFOCLIENT_H
#define FIFOCLIENT_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

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

#endif // FIFOSERVER_H