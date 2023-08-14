/*
 * deviceio.hpp
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

Description: Device IO Module

*************************************************/

#include <string>
#include <thread>
#include <vector>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#endif

using json = nlohmann::json;

class SocketServer
{
public:
    SocketServer(int port);
    ~SocketServer();

    void Start();
    void Stop();
    void SetMessageHandler(std::function<void(const std::string &, SOCKET)> handler);
    void SendMessage(const std::string &message, SOCKET clientSocket);

private:
    int port_;
    SOCKET listenSocket_;
    std::thread acceptThread_;
    std::function<void(const std::string &, SOCKET)> messageHandler_;
    std::vector<std::thread> clientThreads_;
    bool isRunning_;

    void AcceptThread();
    void ClientThread(SOCKET clientSocket);
};
