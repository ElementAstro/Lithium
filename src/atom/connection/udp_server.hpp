/*
 * udp_server.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: A simple UDP server.

*************************************************/

#ifndef ATOM_CONNECTION_UDP_HPP
#define ATOM_CONNECTION_UDP_HPP

#include <functional>
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <string>

// 区分平台
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
using socklen_t = int;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
using SOCKET = int;
#endif

/**
 * @class SocketHub
 * @brief A simple UDP socket server class that handles incoming messages and allows sending messages to specified addresses.
 *
 * 一个简单的UDP套接字服务器类，用于处理接收到的消息，并允许向指定地址发送消息。
 */
class SocketHub
{
public:
    /**
     * @brief Constructor for SocketHub. Initializes the server state.
     *
     * SocketHub的构造函数。初始化服务器状态。
     */
    SocketHub();

    /**
     * @brief Destructor for SocketHub. Ensures proper resource cleanup.
     *
     * SocketHub的析构函数。确保适当的资源清理。
     */
    ~SocketHub();

    /**
     * @brief Starts the UDP server on the specified port.
     *
     * 在指定端口上启动UDP服务器。
     *
     * @param port The port number on which the server will listen for incoming messages.
     *
     * @param port 服务器监听传入消息的端口号。
     */
    void start(int port);

    /**
     * @brief Stops the server and cleans up resources.
     *
     * 停止服务器并清理资源。
     */
    void stop();

    /**
     * @brief Adds a message handler function that will be called whenever a new message is received.
     *
     * 添加一个消息处理函数，每当接收到新消息时都会调用此函数。
     *
     * @param handler A function to handle incoming messages. It takes a string as input.
     *
     * @param handler 一个处理传入消息的函数。它接受一个字符串作为输入。
     */
    void addHandler(std::function<void(std::string)> handler);

    /**
     * @brief Sends a message to the specified IP address and port.
     *
     * 向指定的IP地址和端口发送消息。
     *
     * @param message The message to be sent.
     * @param ip The target IP address.
     * @param port The target port number.
     *
     * @param message 要发送的消息。
     * @param ip 目标IP地址。
     * @param port 目标端口号。
     */
    void sendTo(const std::string &message, const std::string &ip, int port);

private:
    std::atomic<bool> m_running; ///< Indicates whether the server is running or not.
                                 ///< 指示服务器是否正在运行。
    SOCKET m_serverSocket;       ///< The socket descriptor for the server.
                                 ///< 服务器的套接字描述符。
#if __cplusplus >= 202002L
    std::unique_ptr<std::jthread> m_acceptThread; ///< The thread for handling incoming messages.
                                                  ///< 用于处理传入消息的线程。
#else
    std::unique_ptr<std::thread> m_acceptThread; ///< The thread for handling incoming messages (for C++ standards before C++20).
                                                 ///< 用于处理传入消息的线程（针对C++20之前的C++标准）。
#endif

    std::function<void(std::string)> m_handler; ///< The function to handle incoming messages.
                                                ///< 处理传入消息的函数。

    /**
     * @brief Initializes networking. Required for Windows.
     *
     * 初始化网络。Windows系统需要。
     *
     * @return true if successful, false otherwise.
     *
     * @return 如果成功，则为true；否则为false。
     */
    bool initNetworking();

    /**
     * @brief Cleans up networking resources. Required for Windows.
     *
     * 清理网络资源。Windows系统需要。
     */
    void cleanupNetworking();

    /**
     * @brief The main loop for receiving messages. Runs in a separate thread.
     *
     * 接收消息的主循环。在单独的线程中运行。
     */
    void handleMessages();
};

#endif