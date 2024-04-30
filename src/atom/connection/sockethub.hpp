/*
 * sockethub.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: SocketHub类用于管理socket连接的类。

*************************************************/

#ifndef ATOM_CONNECTION_SOCKETHUB_HPP
#define ATOM_CONNECTION_SOCKETHUB_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace atom::connection {
/**
 * @class SocketHub
 * @brief 用于管理socket连接的类。
 *
 * SocketHub类提供了启动和停止socket服务的功能，同时管理多个客户端连接。
 * 它支持接收来自客户端的连接，并为每个客户端启动一个线程处理消息。
 *
 * @class SocketHub
 * @brief A class for managing socket connections.
 *
 * The SocketHub class offers functionalities to start and stop a socket
 * service, while managing multiple client connections. It supports accepting
 * connections from clients and starts a thread for each client to handle
 * messages.
 */
class SocketHub {
public:
    /**
     * @brief 构造函数。
     * @brief Constructor.
     */
    SocketHub();

    /**
     * @brief 析构函数，负责资源的清理。
     * @brief Destructor, responsible for cleaning up resources.
     */
    ~SocketHub();

    /**
     * @brief 启动socket服务并监听指定端口。
     *
     * @param port 要监听的端口号。
     * @brief Starts the socket service and listens on the specified port.
     *
     * @param port The port number to listen on.
     */
    void start(int port);

    /**
     * @brief 停止socket服务并关闭所有连接。
     * @brief Stops the socket service and closes all connections.
     */
    void stop();

    /**
     * @brief 添加消息处理函数。
     *
     * @param handler 消息处理函数。
     * @brief Adds a message handler.
     *
     * @param handler The message handler.
     */
    void addHandler(std::function<void(std::string)> handler);

private:
    static const int maxConnections = 10;  ///< 最大连接数。
                                           ///< Maximum number of connections.
    std::atomic<bool>
        running;  ///< 指示服务是否正在运行的标志。
                  ///< Flag indicating whether the service is running.
#ifdef _WIN32
    SOCKET serverSocket;          ///< 服务器socket。
                                  ///< Server socket.
    std::vector<SOCKET> clients;  ///< 客户端sockets列表。
                                  ///< List of client sockets.
#else
    int serverSocket;
    std::vector<int> clients;
#endif
#if __cplusplus >= 202002L
    std::unique_ptr<std::jthread>
        acceptThread;  ///< 用于接受连接的线程。
                       ///< Thread for accepting connections.
    std::vector<std::unique_ptr<std::jthread>>
        clientThreads;  ///< 客户端处理线程列表。
                        ///< List of threads for handling clients.
#else
    std::unique_ptr<std::thread>
        acceptThread;  ///< 用于接受连接的线程。
                       ///< Thread for accepting connections.
    std::vector<std::unique_ptr<std::thread>>
        clientThreads;  ///< 客户端处理线程列表。
                        ///< List of threads for handling clients.
#endif

    std::function<void(std::string)> handler;  ///< 消息处理函数。

    /**
     * @brief 初始化Winsock。
     *
     * @return 成功返回true，失败返回false。
     * @brief Initializes Winsock.
     *
     * @return true on success, false on failure.
     */
    bool initWinsock();

    /**
     * @brief 清理Winsock资源。
     * @brief Cleans up Winsock resources.
     */
    void cleanupWinsock();

    /**
     * @brief 关闭指定的socket。
     *
     * @param socket 要关闭的socket。
     * @brief Closes the specified socket.
     *
     * @param socket The socket to close.
     */
#ifdef _WIN32
    void closeSocket(SOCKET socket);
#else
    void closeSocket(int socket);
#endif

    /**
     * @brief 接受客户端连接并将它们添加到clients列表。
     * @brief Accepts client connections and adds them to the clients list.
     */
    void acceptConnections();

    /**
     * @brief 处理客户端消息。
     *
     * @param clientSocket 客户端socket。
     * @brief Handles messages from a client.
     *
     * @param clientSocket The client socket.
     */
#ifdef _WIN32
    void handleClientMessages(SOCKET clientSocket);
#else
    void handleClientMessages(int clientSocket);
#endif
    /**
     * @brief 清理sockets资源，关闭所有客户端连接。
     * @brief Cleans up socket resources, closing all client connections.
     */
    void cleanupSocket();
};
}  // namespace atom::connection

#endif