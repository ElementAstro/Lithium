/*
 * discovery.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 20234-3-1

Description: A simple way to discover HTTP server

**************************************************/

#include "discovery.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

#include "atom/log/loguru.hpp"

#ifdef _WIN32
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#ifdef _MSVC
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#endif
bool init_winsock() {
    WSADATA wsaData;
    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (err != 0) {
        std::cerr << "WSAStartup failed with error: " << err << std::endl;
        return false;
    }

    return true;
}
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#endif

#include "atom/type/json.hpp"

using json = nlohmann::json;

const int PORT = 32227;
const std::string ALPACA_DISCOVERY = "alpacadiscovery1";
const std::string ALPACA_RESPONSE = "AlpacaPort";

std::vector<std::string> search_ipv4(int numquery = 2, int timeout = 2) {
    std::vector<std::string> addrs;
#ifdef _WIN32
    // Initialize Winsock
    if (!init_winsock()) {
        return addrs;
    }
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "failed to create socket" << std::endl;
        return addrs;
    }

    // Enable broadcasting
    int broadcast = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&broadcast,
                   sizeof(broadcast)) < 0) {
        std::cerr << "failed to enable broadcasting" << std::endl;
        close(sock);
        return addrs;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);

    char buf[1024];
    for (int i = 0; i < numquery; ++i) {
        struct ifaddrs *ifaddr, *ifa;
        if (getifaddrs(&ifaddr) == -1) {
            std::cerr << "failed to get interface addresses" << std::endl;
            break;
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL)
                continue;

            if (ifa->ifa_addr->sa_family == AF_INET) {
                auto sin = (struct sockaddr_in *)ifa->ifa_addr;
                std::string ip = inet_ntoa(sin->sin_addr);
                if (ip == "127.0.0.1") {
                    addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
                } else {
                    auto netmask = (struct sockaddr_in *)ifa->ifa_netmask;
                    std::string netmask_str = inet_ntoa(netmask->sin_addr);
                    std::string broadcast_str = "";
                    for (int i = 0; i < 4; ++i) {
                        unsigned char b1 = sin->sin_addr.S_un.S_un_b.s_b1;
                        unsigned char b2 = sin->sin_addr.S_un.S_un_b.s_b2;
                        unsigned char b3 = sin->sin_addr.S_un.S_un_b.s_b3;
                        unsigned char b4 = sin->sin_addr.S_un.S_un_b.s_b4;
                        unsigned char m1 = netmask->sin_addr.S_un.S_un_b.s_b1;
                        unsigned char m2 = netmask->sin_addr.S_un.S_un_b.s_b2;
                        unsigned char m3 = netmask->sin_addr.S_un.S_un_b.s_b3;
                        unsigned char m4 = netmask->sin_addr.S_un.S_un_b.s_b4;

                        unsigned char b = ((b1 & m1) | (~m1 & 0xff)) << 24 |
                                          ((b2 & m2) | (~m2 & 0xff)) << 16 |
                                          ((b3 & m3) | (~m3 & 0xff)) << 8 |
                                          ((b4 & m4) | (~m4 & 0xff));

                        std::stringstream ss;
                        ss << (int)(b >> (i * 8) & 0xff);
                        broadcast_str += ss.str();
                        if (i < 3)
                            broadcast_str += ".";
                    }
                    addr.sin_addr.s_addr = inet_addr(broadcast_str.c_str());
                }

                // Send discovery message
                if (sendto(sock, ALPACA_DISCOVERY.c_str(),
                           ALPACA_DISCOVERY.length(), 0,
                           (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                    std::cerr << "failed to send discovery message"
                              << std::endl;
                    freeifaddrs(ifaddr);
                    close(sock);
                    return addrs;
                }

                // Receive response
                struct sockaddr_in remote_addr = {0};
                socklen_t addrlen = sizeof(remote_addr);
                int len = recvfrom(sock, buf, sizeof(buf), 0,
                                   (struct sockaddr *)&remote_addr, &addrlen);
                if (len < 0) {
                    // Timeout or error occurred
                    continue;
                }

                // Parse response as JSON
                std::string data(buf, len);
                try {
                    json j = json::parse(data);
                    int port = j[ALPACA_RESPONSE];
                    std::string ip = inet_ntoa(remote_addr.sin_addr);
                    std::string addr_str = ip + ":" + std::to_string(port);
                    if (std::find(addrs.begin(), addrs.end(), addr_str) ==
                        addrs.end()) {
                        addrs.push_back(addr_str);
                    }
                } catch (std::exception &e) {
                    std::cerr << "failed to parse response: " << e.what()
                              << std::endl;
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    close(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return addrs;
}

std::vector<std::string> search_ipv6(int numquery = 2, int timeout = 2) {
    std::vector<std::string> addrs;

    int sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        std::cerr << "failed to create socket" << std::endl;
        return addrs;
    }

    struct sockaddr_in6 addr = {0};
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(PORT);

    char buf[1024];
    for (int i = 0; i < numquery; ++i) {
        struct ifaddrs *ifaddr, *ifa;
        if (getifaddrs(&ifaddr) == -1) {
            std::cerr << "failed to get interface addresses" << std::endl;
            break;
        }

        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL)
                continue;

            if (ifa->ifa_addr->sa_family == AF_INET6) {
                auto sin6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                if (IN6_IS_ADDR_LOOPBACK(&sin6->sin6_addr) ||
                    IN6_IS_ADDR_LINKLOCAL(&sin6->sin6_addr)) {
                    // Skip loopback and link-local addresses
                    continue;
                }

                addr.sin6_addr = sin6->sin6_addr;

                // Send discovery message
                if (sendto(sock, ALPACA_DISCOVERY.c_str(),
                           ALPACA_DISCOVERY.length(), 0,
                           (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                    std::cerr << "failed to send discovery message"
                              << std::endl;
                    freeifaddrs(ifaddr);
                    close(sock);
                    return addrs;
                }

                // Receive response
                struct sockaddr_in6 remote_addr = {0};
                socklen_t addrlen = sizeof(remote_addr);
                int len = recvfrom(sock, buf, sizeof(buf), 0,
                                   (struct sockaddr *)&remote_addr, &addrlen);
                if (len < 0) {
                    // Timeout or error occurred
                    continue;
                }

                // Parse response as JSON
                std::string data(buf, len);
                try {
                    json j = json::parse(data);
                    int port = j[ALPACA_RESPONSE];
                    char ip_str[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, &remote_addr.sin6_addr, ip_str,
                              INET6_ADDRSTRLEN);
                    std::string addr_str = std::string("[") + ip_str + "]" +
                                           ":" + std::to_string(port);
                    if (std::find(addrs.begin(), addrs.end(), addr_str) ==
                        addrs.end()) {
                        addrs.push_back(addr_str);
                    }
                } catch (std::exception &e) {
                    std::cerr << "failed to parse response: " << e.what()
                              << std::endl;
                }
            }
        }

        freeifaddrs(ifaddr);
    }

    close(sock);
    return addrs;
}
