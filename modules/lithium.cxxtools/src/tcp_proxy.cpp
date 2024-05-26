/*
 * tcp_proxy.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Tcp proxy server

*************************************************/

#include <getopt.h>
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

#include "atom/log/loguru.hpp"

const int BUFFER_SIZE = 4096;

std::mutex mutex;

// 处理数据传输
void forwardData(int srcSockfd, int dstSockfd) {
    char buffer[BUFFER_SIZE];
    int numBytes;

    try {
        while ((numBytes = recv(srcSockfd, buffer, BUFFER_SIZE, 0)) > 0) {
            send(dstSockfd, buffer, numBytes, 0);
        }
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to forward data: {}", e.what());
    }
}

// 启动代理服务器
void startProxyServer(const std::string &srcIp, int srcPort,
                      const std::string &dstIp, int dstPort) {
#ifdef _WIN32
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        LOG_F(ERROR, "Failed to initialize Winsock.");
        return;
    }

    SOCKET srcSockfd = socket(AF_INET, SOCK_STREAM, 0);
    SOCKET dstSockfd = socket(AF_INET, SOCK_STREAM, 0);
#else
    int srcSockfd = socket(AF_INET, SOCK_STREAM, 0);
    int dstSockfd = socket(AF_INET, SOCK_STREAM, 0);
#endif

    if (srcSockfd == -1 || dstSockfd == -1) {
        LOG_F(ERROR, "Failed to create socket.");
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    // 绑定源地址和端口
    sockaddr_in srcAddr{};
    srcAddr.sin_family = AF_INET;
    srcAddr.sin_addr.s_addr = inet_addr(srcIp.c_str());
    srcAddr.sin_port = htons(srcPort);

    if (bind(srcSockfd, reinterpret_cast<sockaddr *>(&srcAddr),
             sizeof(srcAddr)) == -1) {
        LOG_F(ERROR, "Failed to bind source address.");
#ifdef _WIN32
        closesocket(srcSockfd);
        WSACleanup();
#else
        close(srcSockfd);
#endif
        return;
    }

    // 连接目标地址和端口
    sockaddr_in dstAddr{};
    dstAddr.sin_family = AF_INET;
    dstAddr.sin_addr.s_addr = inet_addr(dstIp.c_str());
    dstAddr.sin_port = htons(dstPort);

    if (connect(dstSockfd, reinterpret_cast<sockaddr *>(&dstAddr),
                sizeof(dstAddr)) == -1) {
        LOG_F(ERROR, "Failed to connect to destination address.");
#ifdef _WIN32
        closesocket(srcSockfd);
        closesocket(dstSockfd);
        WSACleanup();
#else
        close(srcSockfd);
        close(dstSockfd);
#endif
        return;
    }

    // 开始转发数据
    forwardData(srcSockfd, dstSockfd);

#ifdef _WIN32
    closesocket(srcSockfd);
    closesocket(dstSockfd);
    WSACleanup();
#else
    close(srcSockfd);
    close(dstSockfd);
#endif
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        LOG_F(ERROR, "Failed to initialize Winsock.");
        return -1;
    }
#endif

    std::string srcIp = "127.0.0.1";  // 源IP地址
    int srcPort = 12345;              // 源端口

    std::string dstIp = "127.0.0.1";  // 目标IP地址
    int dstPort = 54321;              // 目标端口

    int option;
    while ((option = getopt(argc, argv, "s:p:d:o:")) != -1) {
        switch (option) {
            case 's':
                srcIp = optarg;
                break;
            case 'p':
                srcPort = std::stoi(optarg);
                break;
            case 'd':
                dstIp = optarg;
                break;
            case 'o':
                dstPort = std::stoi(optarg);
                break;
            default:
                LOG_F(ERROR,
                      "Usage: {} -s <src_ip> -p <src_port> -d <dst_ip> -o "
                      "<dst_port>",
                      argv[0]);
                return 1;
        }
    }
#if __cplusplus >= 202002L
    std::vector<std::jthread> threads;
#else
    std::vector<std::thread> threads;
#endif

    // 启动多个线程处理并发连接
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(startProxyServer, srcIp, srcPort, dstIp, dstPort);
    }

    // 等待所有线程结束
    for (auto &thread : threads) {
        thread.join();
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
