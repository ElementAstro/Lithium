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

#include <loguru/loguru.hpp>

SocketServer::SocketServer(int port)
    : port_(port), listenSocket_(INVALID_SOCKET), isRunning_(false)
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        LOG_F(ERROR, "Failed to initialize Winsock");
    }
#endif
}

SocketServer::~SocketServer()
{
    Stop();
}

void SocketServer::Start()
{
    if (isRunning_)
    {
        return;
    }

#ifdef _WIN32
    listenSocket_ = socket(AF_INET, SOCK_STREAM, 0);
#else
    listenSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
    if (listenSocket_ == INVALID_SOCKET)
    {
#ifdef _WIN32
        WSACleanup();
#endif
        LOG_F(ERROR, "Failed to create socket");
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(port_);

    if (bind(listenSocket_, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
#ifdef _WIN32
        closesocket(listenSocket_);
        WSACleanup();
#else
        close(listenSocket_);
#endif
        LOG_F(ERROR, "Failed to bind socket");
    }

    if (listen(listenSocket_, SOMAXCONN) < 0)
    {
#ifdef _WIN32
        closesocket(listenSocket_);
        WSACleanup();
#else
        close(listenSocket_);
#endif
        LOG_F(ERROR, "Failed to listen");
    }

    isRunning_ = true;
    acceptThread_ = std::thread([&]()
                                { AcceptThread(); });
}

void SocketServer::Stop()
{
    if (!isRunning_)
    {
        return;
    }

    isRunning_ = false;

#ifdef _WIN32
    closesocket(listenSocket_);
    WSACleanup();
#else
    close(listenSocket_);
#endif

    if (acceptThread_.joinable())
    {
        acceptThread_.join();
    }

    for (auto &thread : clientThreads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    clientThreads_.clear();
}

void SocketServer::SetMessageHandler(std::function<void(const std::string &, SOCKET)> handler)
{
    messageHandler_ = std::move(handler);
}

void SocketServer::SendMessage(const std::string &message, SOCKET clientSocket)
{
    send(clientSocket, message.c_str(), message.length(), 0);
}

void SocketServer::AcceptThread()
{
    while (isRunning_)
    {
        sockaddr_in clientAddress;
        int clientAddressLength = sizeof(clientAddress);

        SOCKET clientSocket = accept(listenSocket_, (struct sockaddr *)&clientAddress, (socklen_t *)&clientAddressLength);
        if (clientSocket == INVALID_SOCKET)
        {
            LOG_F(ERROR, "Failed to accept client connection");
            continue;
        }

        try
        {
            clientThreads_.emplace_back([&]()
                                        { ClientThread(clientSocket); });
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Error occurred in client thread: %s", e.what());

#ifdef _WIN32
            closesocket(clientSocket);
#else
            close(clientSocket);
#endif
        }
    }
}

void SocketServer::ClientThread(SOCKET clientSocket)
{
    char buffer[1024];
    std::string clientIP = inet_ntoa(((struct sockaddr_in *)&clientSocket)->sin_addr);
    int bytesRead;

    try
    {
        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0)
        {
            buffer[bytesRead] = '\0';

            if (messageHandler_)
            {
                messageHandler_(std::string(buffer), clientSocket);
            }
        }

#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Error occurred in client thread: %s", e.what());

#ifdef _WIN32
        closesocket(clientSocket);
#else
        close(clientSocket);
#endif
    }
}