/*
 * tcp_proxy.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Tcp proxy server enhanced with additional functionalities,
             detailed logging, and improved exception handling.

*************************************************/

#include <getopt.h>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

constexpr int BUFFER_SIZE = 4096;
std::mutex mutex;
std::atomic<bool> running{true};

/**
 * @brief Function to handle data forwarding from srcSockfd to dstSockfd.
 *
 * @param srcSockfd Source socket file descriptor.
 * @param dstSockfd Destination socket file descriptor.
 */
void forwardData(int srcSockfd, int dstSockfd) {
    char buffer[BUFFER_SIZE];
    ssize_t numBytes;
    size_t totalBytes = 0;

    LOG_F(INFO,
          "Starting data forwarding: source socket {} -> destination socket {}",
          srcSockfd, dstSockfd);

    try {
        while (running.load()) {
            numBytes = recv(srcSockfd, buffer, BUFFER_SIZE, 0);
            if (numBytes > 0) {
                ssize_t sentBytes = send(dstSockfd, buffer, numBytes, 0);
                if (sentBytes == -1) {
                    LOG_F(ERROR,
                          "Failed to send data from socket {} to socket {}",
                          srcSockfd, dstSockfd);
                    break;
                }
                totalBytes += sentBytes;
                LOG_F(INFO, "Forwarded {} bytes from socket {} to socket {}",
                      sentBytes, srcSockfd, dstSockfd);
            } else if (numBytes == 0) {
                LOG_F(INFO, "Source socket {} has closed the connection",
                      srcSockfd);
                break;
            } else {
#ifdef _WIN32
                int error = WSAGetLastError();
                if (error == WSAEWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
#else
                if (errno == EWOULDBLOCK || errno == EAGAIN) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
#endif
                LOG_F(ERROR, "Failed to receive data from source socket {}",
                      srcSockfd);
                break;
            }
        }
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Exception occurred while forwarding data: {}", e.what());
    }

    LOG_F(INFO,
          "Data forwarding ended: source socket {} -> destination socket {}, "
          "total bytes forwarded {}",
          srcSockfd, dstSockfd, totalBytes);

#ifdef _WIN32
    closesocket(srcSockfd);
    closesocket(dstSockfd);
#else
    close(srcSockfd);
    close(dstSockfd);
#endif
}

/**
 * @brief Starts the proxy server, listens on the source IP and port, and
 * forwards traffic to the destination IP and port.
 *
 * @param srcIp Source IP address.
 * @param srcPort Source port.
 * @param dstIp Destination IP address.
 * @param dstPort Destination port.
 */
void startProxyServer(const std::string &srcIp, int srcPort,
                      const std::string &dstIp, int dstPort) {
#ifdef _WIN32
    SOCKET srcSockfd = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET dstSockfd = socket(AF_INET, SOCK_STREAM, 0);
#else
    int srcSockfd = socket(AF_INET, SOCK_STREAM, 0);
    int dstSockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif

    if (srcSockfd == -1 || dstSockfd == -1) {
        LOG_F(ERROR, "Failed to create sockets");
        THROW_RUNTIME_ERROR("Failed to create sockets");
    }
    LOG_F(
        INFO,
        "Successfully created sockets: source socket {}, destination socket {}",
        srcSockfd, dstSockfd);

    sockaddr_in srcAddr{};
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_addr.s_addr = inet_addr(srcIp.c_str());
    srcAddr.sin_port = htons(srcPort);

    if (bind(srcSockfd, reinterpret_cast<sockaddr *>(&srcAddr),
             sizeof(srcAddr)) == -1) {
        LOG_F(ERROR, "Failed to bind source address: {}:{}", srcIp, srcPort);
#ifdef _WIN32
        closesocket(srcSockfd);
        closesocket(dstSockfd);
#else
        close(srcSockfd);
        close(dstSockfd);
#endif
        THROW_RUNTIME_ERROR("Failed to bind source address: {}:{}", srcIp,
                            srcPort);
    }
    LOG_F(INFO, "Successfully bound source address: {}:{}", srcIp, srcPort);

    if (listen(srcSockfd, SOMAXCONN) == -1) {
        LOG_F(ERROR, "Failed to listen on source socket: {}:{}", srcIp,
              srcPort);
#ifdef _WIN32
        closesocket(srcSockfd);
        closesocket(dstSockfd);
#else
        close(srcSockfd);
        close(dstSockfd);
#endif
        THROW_RUNTIME_ERROR("Failed to listen on source socket: {}:{}", srcIp,
                            srcPort);
    }
    LOG_F(INFO, "Listening on source socket: {}:{}", srcIp, srcPort);

    while (running.load()) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int addrLen = sizeof(clientAddr);
#else
        socklen_t addrLen = sizeof(clientAddr);
#endif
        int clientSockfd = accept(
            srcSockfd, reinterpret_cast<sockaddr *>(&clientAddr), &addrLen);
        if (clientSockfd == -1) {
#ifdef _WIN32
            int error = WSAGetLastError();
            if (error == WSAEINTR) {
                LOG_F(INFO,
                      "Received interrupt signal, stopping accepting new "
                      "connections");
                break;
            }
#else
            if (errno == EINTR) {
                LOG_F(INFO,
                      "Received interrupt signal, stopping accepting new "
                      "connections");
                break;
            }
#endif
            LOG_F(ERROR, "Failed to accept new connection");
            continue;
        }

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        int clientPort = ntohs(clientAddr.sin_port);
        LOG_F(INFO, "Accepted new connection: {}:{}, client socket {}",
              clientIP, clientPort, clientSockfd);

        // Connect to the destination server
        sockaddr_in dstAddr{};
        dstAddr.sin_family = AF_INET;
        dstAddr.sin_addr.s_addr = inet_addr(dstIp.c_str());
        dstAddr.sin_port = htons(dstPort);

        if (connect(dstSockfd, reinterpret_cast<sockaddr *>(&dstAddr),
                    sizeof(dstAddr)) == -1) {
            LOG_F(ERROR, "Failed to connect to destination address: {}:{}",
                  dstIp, dstPort);
#ifdef _WIN32
            closesocket(clientSockfd);
            closesocket(dstSockfd);
#else
            close(clientSockfd);
            close(dstSockfd);
#endif
            continue;
        }
        LOG_F(INFO,
              "Successfully connected to destination address: {}:{}, "
              "destination socket {}",
              dstIp, dstPort, dstSockfd);

        // Start data forwarding thread
        std::thread([clientSockfd, dstSockfd]() {
            forwardData(clientSockfd, dstSockfd);
        }).detach();
    }

#ifdef _WIN32
    closesocket(srcSockfd);
    closesocket(dstSockfd);
#else
    close(srcSockfd);
    close(dstSockfd);
#endif

    LOG_F(INFO, "Proxy server closed: {}:{} -> {}:{}", srcIp, srcPort, dstIp,
          dstPort);
}

/**
 * @brief Signal handler to gracefully shut down the proxy server.
 *
 * @param signal The received signal number.
 */
void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        LOG_F(INFO, "Received interrupt signal, shutting down proxy server...");
        running.store(false);
    }
}

int main(int argc, char *argv[]) {
    // Initialize Loguru
    loguru::init(argc, argv);
    loguru::add_file("tcp_proxy.log", loguru::Append, loguru::Verbosity_MAX);
    LOG_F(INFO, "Starting TCP proxy server");

    // Set up signal handling
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

#ifdef _WIN32
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        LOG_F(ERROR, "Failed to initialize Winsock");
        return -1;
    }
    LOG_F(INFO, "Winsock initialized successfully");
#endif

    // Default configuration
    std::string srcIp = "0.0.0.0";
    int srcPort = 12345;
    std::string dstIp = "127.0.0.1";
    int dstPort = 54321;

    // Parse command-line arguments
    int option;
    while ((option = getopt(argc, argv, "s:p:d:o:h")) != -1) {
        switch (option) {
            case 's':
                srcIp = optarg;
                LOG_F(INFO, "Set source IP to: {}", srcIp);
                break;
            case 'p':
                srcPort = std::stoi(optarg);
                LOG_F(INFO, "Set source port to: {}", srcPort);
                break;
            case 'd':
                dstIp = optarg;
                LOG_F(INFO, "Set destination IP to: {}", dstIp);
                break;
            case 'o':
                dstPort = std::stoi(optarg);
                LOG_F(INFO, "Set destination port to: {}", dstPort);
                break;
            case 'h':
            default:
                LOG_F(INFO,
                      "Usage: {} -s <source_IP> -p <source_port> -d "
                      "<destination_IP> -o <destination_port>",
                      argv[0]);
                return 0;
        }
    }

    // Start the proxy server
    std::thread proxyThread(startProxyServer, srcIp, srcPort, dstIp, dstPort);

    LOG_F(INFO, "Proxy server is running, waiting for interrupt signal...");

    // Wait for the proxy thread to finish
    proxyThread.join();

#ifdef _WIN32
    WSACleanup();
    LOG_F(INFO, "Winsock cleanup completed");
#endif

    LOG_F(INFO, "TCP proxy server has been shut down");
    return 0;
}