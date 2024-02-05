/*
 * udp_server.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A simple UDP server.

*************************************************/

#include "udp_server.hpp"

#include "atom/log/loguru.hpp"

namespace Atom::Connection
{
    UdpSocketHub::UdpSocketHub() : m_running(false), m_serverSocket(INVALID_SOCKET) {}

    UdpSocketHub::~UdpSocketHub()
    {
        stop();
    }

    bool UdpSocketHub::initNetworking()
    {
#ifdef _WIN32
        WSADATA wsaData;
        return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
        return true; // 在Linux上不需要初始化
#endif
    }

    void UdpSocketHub::cleanupNetworking()
    {
#ifdef _WIN32
        WSACleanup();
#endif
    }

    void UdpSocketHub::start(int port)
    {
        if (m_running.load())
            return; // 防止重复启动
        m_running.store(true);

        if (!initNetworking())
        {
            LOG_F(ERROR, "Networking initialization failed.");
            return;
        }

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        m_serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_serverSocket == INVALID_SOCKET)
        {
            LOG_F(ERROR, "Failed to create socket.");
            cleanupNetworking();
            return;
        }

        if (bind(m_serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            LOG_F(ERROR, "Bind failed with error.");
            closesocket(m_serverSocket);
            cleanupNetworking();
            return;
        }

#if __cplusplus >= 202002L
        m_acceptThread = std::make_unique<std::jthread>([this]()
                                                        { this->handleMessages(); });
#else
        m_acceptThread = std::make_unique<std::thread>([this]()
                                                       { this->handleMessages(); });
#endif
    }

    void UdpSocketHub::stop()
    {
        if (!m_running.load())
            return;
        m_running.store(false);

        closesocket(m_serverSocket);
        cleanupNetworking();

        if (m_acceptThread && m_acceptThread->joinable())
        {
            m_acceptThread->join();
        }
    }

    void UdpSocketHub::addHandler(std::function<void(std::string)> handler)
    {
        m_handler = handler;
    }

    void UdpSocketHub::sendTo(const std::string &message, const std::string &ip, int port)
    {
        if (!m_running.load())
        {
            LOG_F(ERROR, "Server is not running.");
            return;
        }

        sockaddr_in targetAddr;
        targetAddr.sin_family = AF_INET;
        targetAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &targetAddr.sin_addr);

        int sentBytes = sendto(m_serverSocket, message.c_str(), message.length(), 0,
                               (struct sockaddr *)&targetAddr, sizeof(targetAddr));
        if (sentBytes == SOCKET_ERROR)
        {
            LOG_F(ERROR, "Failed to send message.");
#ifdef _WIN32
            LOG_F(ERROR, "Error: {}", WSAGetLastError());
#else
            LOG_F(ERROR, "Error: {}", errno);
#endif
        }
    }

    void UdpSocketHub::handleMessages()
    {
        char buffer[1024];
        sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);

        while (m_running.load())
        {
            int bytesReceived = recvfrom(m_serverSocket, buffer, sizeof(buffer), 0,
                                         (struct sockaddr *)&clientAddr, &clientAddrSize);
            if (bytesReceived == SOCKET_ERROR)
            {
                LOG_F(ERROR, "recvfrom failed with error.");
                continue;
            }

            if (m_handler)
            {
                std::string message(buffer, bytesReceived);
                m_handler(message);
            }
        }
    }

}
