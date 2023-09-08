/*
 * deviceio.cpp
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

#include "deviceio.hpp"
#include "event/eventloop.hpp"

#include <loguru/loguru.hpp>

#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>

SocketServer::SocketServer(EventLoop &eventLoop, int port)
    : eventLoop(eventLoop), port(port), serverSocket(INVALID_SOCKET), running(false) {}

void SocketServer::start()
{
    LOG_F(INFO, "Starting server on port %d", port);

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG_F(ERROR, "Failed to initialize Winsock");
        return;
    }
#endif

    serverSocket = socket(AF_INET, SOCK_STREAM, 0); // 修改 IPPROTO_TCP 为 0
    if (serverSocket == INVALID_SOCKET)
    {
        LOG_F(ERROR, "Failed to create socket");
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr *)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
    {
        LOG_F(ERROR, "Failed to bind socket");
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        LOG_F(ERROR, "Failed to listen on socket");
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    eventLoop.registerEventTrigger([this]()
                                   { acceptClientConnection(); });

    LOG_F(INFO, "Server started on port %d", port);
    running = true;
}

void SocketServer::stop()
{
    if (serverSocket != INVALID_SOCKET)
    {
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        serverSocket = INVALID_SOCKET;
    }
    running = false;
}

bool SocketServer::is_running()
{
    return running;
}

void SocketServer::setMessageHandler(MessageHandler handler)
{
    messageHandler = std::move(handler);
}

void SocketServer::sendMessage(SOCKET clientSocket, const std::string &message)
{
    send(clientSocket, message.c_str(), message.length(), 0);
}

void SocketServer::acceptClientConnection()
{
    sockaddr_in clientAddress{};
#ifdef _WIN32
    int clientAddressLength = sizeof(clientAddress);
#else
    socklen_t clientAddressLength = sizeof(clientAddress);
#endif

    SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddress, &clientAddressLength);
    if (clientSocket == INVALID_SOCKET)
    {
        LOG_F(ERROR, "Failed to accept client connection");
        return;
    }

    eventLoop.addTask([this, clientSocket]()
                      { handleClientMessage(clientSocket); });
}

void SocketServer::handleClientMessage(SOCKET clientSocket)
{
    char buffer[1024];

    int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead == SOCKET_ERROR)
    {
        LOG_F(ERROR, "Failed to read from client socket");
    }
    else if (bytesRead == 0)
    {
        closesocket(clientSocket);
    }
    else
    {
        std::string message(buffer, bytesRead);
        LOG_F(INFO, "Received message from client: %s", message.c_str());

        if (messageHandler)
        {
            messageHandler(message);
        }

        std::string response = "Server response: " + message;
        sendMessage(clientSocket, response);

        handleClientMessage(clientSocket);
    }
}