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

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>

#include "atom/log/loguru.hpp"

#ifndef _WIN32
typedef int SOCKET;
#endif

namespace atom::connection {
SocketHub::SocketHub() : running(false), serverSocket(-1), epoll_fd(-1) {}

SocketHub::~SocketHub() { stop(); }

void SocketHub::start(int port) {
    if (running.load()) {
        LOG_F(WARNING, "SocketHub is already running.");
        return;
    }

    if (!initWinsock()) {
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
    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress),
             sizeof(serverAddress)) == SOCKET_ERROR)
#else
    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress),
             sizeof(serverAddress)) < 0)
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

#ifdef __linux__
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        LOG_F(ERROR, "Failed to create epoll file descriptor.");
        cleanupSocket();
        return;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
        LOG_F(ERROR, "Failed to add server socket to epoll.");
        cleanupSocket();
        return;
    }
#endif

    running.store(true);
    DLOG_F(INFO, "SocketHub started on port {}", port);

#if __cplusplus >= 202002L
    acceptThread = std::jthread(&SocketHub::acceptConnections, this);
#else
    acceptThread =
        std::make_unique<std::thread>(&SocketHub::acceptConnections, this);
#endif
}

void SocketHub::stop() {
    if (!running.load()) {
        LOG_F(WARNING, "SocketHub is not running.");
        return;
    }

    running.store(false);

    if (acceptThread.joinable()) {
        acceptThread.join();
    }

    cleanupSocket();
    cleanupWinsock();
    DLOG_F(INFO, "SocketHub stopped.");
}

void SocketHub::addHandler(std::function<void(std::string)> handler) {
    this->handler = std::move(handler);
}

bool SocketHub::initWinsock() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        LOG_F(ERROR, "Failed to initialize Winsock.");
        return false;
    }
#endif
    return true;
}

void SocketHub::cleanupWinsock() {
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

void SocketHub::acceptConnections() {
#ifdef __linux__
    struct epoll_event events[maxConnections];
    while (running.load()) {
        int n = epoll_wait(epoll_fd, events, maxConnections, -1);
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == serverSocket) {
                sockaddr_in clientAddress{};
                socklen_t clientAddressLength = sizeof(clientAddress);
                int clientSocket = accept(
                    serverSocket, reinterpret_cast<sockaddr *>(&clientAddress),
                    &clientAddressLength);

                if (clientSocket < 0) {
                    if (running.load()) {
                        LOG_F(ERROR, "Failed to accept client connection.");
                    }
                    continue;
                }

                struct epoll_event event;
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = clientSocket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clientSocket, &event) ==
                    -1) {
                    LOG_F(ERROR, "Failed to add client socket to epoll.");
                    closeSocket(clientSocket);
                    continue;
                }

                std::scoped_lock lock(clientMutex);
                clients.push_back(clientSocket);

                clientThreads[clientSocket] = std::jthread(
                    &SocketHub::handleClientMessages, this, clientSocket);
            } else {
                handleClientMessages(events[i].data.fd);
            }
        }
    }
#else
    while (running.load()) {
        sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);

        SOCKET clientSocket =
            accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress),
                   &clientAddressLength);
        if (clientSocket == INVALID_SOCKET) {
            if (running.load()) {
                LOG_F(ERROR, "Failed to accept client connection.");
            }
            continue;
        }

        std::scoped_lock lock(clientMutex);
        clients.push_back(clientSocket);

        clientThreads.push_back(
            std::jthread(&SocketHub::handleClientMessages, this, clientSocket));
    }
#endif
}

void SocketHub::handleClientMessages(SOCKET clientSocket) {
    char buffer[1024];
    while (running.load()) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            {
                std::scoped_lock lock(clientMutex);
                closeSocket(clientSocket);
                clients.erase(
                    std::remove(clients.begin(), clients.end(), clientSocket),
                    clients.end());
            }
#ifdef __linux__
            clientThreads.erase(clientSocket);
#endif
            break;
        }

        std::string message(buffer, bytesRead);
        if (handler) {
            handler(message);
        }
    }
}

void SocketHub::cleanupSocket() {
    {
        std::scoped_lock lock(clientMutex);
        for (const auto &client : clients) {
            closeSocket(client);
        }
        clients.clear();
    }

    closeSocket(serverSocket);

#ifdef __linux__
    if (epoll_fd != -1) {
        close(epoll_fd);
        epoll_fd = -1;
    }
#endif

    for (auto &thread : clientThreads) {
        if (thread.second.joinable()) {
            thread.second.join();
        }
    }
    clientThreads.clear();
}

}  // namespace atom::connection
