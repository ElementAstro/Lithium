/*
 * sockethub.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: SocketHub类用于管理socket连接的类。

*************************************************/

#include "sockethub.hpp"

#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>

#include "atom/log/loguru.hpp"

#ifndef _WIN32
typedef int SOCKET;
#endif

namespace Atom::Connection
{
    SocketHub::SocketHub() : running(false) {}

    SocketHub::~SocketHub()
    {
        stop();
    }

    void SocketHub::start(int port)
    {
        if (running.load())
        {
            LOG_F(WARNING, "SocketHub is already running.");
            return;
        }

        if (!initWinsock())
        {
            return;
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
        if (serverSocket == INVALID_SOCKET)
#else
        if (serverSocket < 0)
#endif
        {
            LOG_F(ERROR, "Failed to create server socket.");
            cleanupWinsock();
            return;
        }

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

#ifdef _WIN32
        if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
#else
        if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0)
#endif
        {
            LOG_F(ERROR, "Failed to bind server socket.");
            cleanupSocket();
            return;
        }

#ifdef _WIN32
        if (listen(serverSocket, maxConnections) == SOCKET_ERROR)
#else
        if (listen(serverSocket, maxConnections) < 0)
#endif
        {
            LOG_F(ERROR, "Failed to listen on server socket.");
            cleanupSocket();
            return;
        }

        running.store(true);
        DLOG_F(INFO, "SocketHub started on port {}", port);

#if __cplusplus >= 202002L
        acceptThread = std::make_unique<std::jthread>(&SocketHub::acceptConnections, this);
#else
        acceptThread = std::make_unique<std::thread>(&SocketHub::acceptConnections, this);
#endif
    }

    void SocketHub::stop()
    {
        if (!running.load())
        {
            LOG_F(WARNING, "SocketHub is not running.");
            return;
        }

        running.store(false);

        if (acceptThread && acceptThread->joinable())
        {
            acceptThread->join();
        }

        cleanupSocket();
        cleanupWinsock();
        DLOG_F(INFO, "SocketHub stopped.");
    }

    bool SocketHub::initWinsock()
    {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            LOG_F(ERROR, "Failed to initialize Winsock.");
            return false;
        }
#endif
        return true;
    }

    void SocketHub::cleanupWinsock()
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }

#ifdef _WIN32
    void SocketHub::closeSocket(SOCKET socket)
#else
    void SocketHub::closeSocket(int socket)
#endif
    {
#ifdef _WIN32
        closesocket(socket);
#else
        close(socket);
#endif
    }

    void SocketHub::acceptConnections()
    {
        while (running.load())
        {
            sockaddr_in clientAddress{};
            socklen_t clientAddressLength = sizeof(clientAddress);

            SOCKET clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLength);
#ifdef _WIN32
            if (clientSocket == INVALID_SOCKET)
#else
            if (clientSocket < 0)
#endif
            {
                if (running.load())
                {
                    LOG_F(ERROR, "Failed to accept client connection.");
                }
                continue;
            }

            clients.push_back(clientSocket);

#if __cplusplus >= 202002L
            clientThreads.push_back(std::make_unique<std::jthread>([this, clientSocket]()
                                                                   { handleClientMessages(clientSocket); }));
#else
            clientThreads.push_back(std::make_unique<std::thread>([this, clientSocket]()
                                                                  { handleClientMessages(clientSocket); }));
#endif
        }
    }

    void SocketHub::handleClientMessages(SOCKET clientSocket)
    {
        char buffer[1024];
        while (running.load())
        {
            memset(buffer, 0, sizeof(buffer));
            int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead <= 0)
            {
                closeSocket(clientSocket);
                clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
                break;
            }

            std::string message(buffer, bytesRead);

            if (handler)
            {
                handler(message);
            }
        }
    }

    void SocketHub::cleanupSocket()
    {
        for (const auto &client : clients)
        {
            closeSocket(client);
        }
        clients.clear();

        closeSocket(serverSocket);

        for (auto &thread : clientThreads)
        {
            if (thread->joinable())
            {
                thread->join();
            }
        }
        clientThreads.clear();
    }

}
