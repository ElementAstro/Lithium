/*
 * utils.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Network Utils

**************************************************/

#include "utils.hpp"

#include <cstdlib>
#include <cstring>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <iphlpapi.h>
#include <tlhelp32.h>
#include <winsock2.h>
#include <windows.h>
#include <Psapi.h>
#define close closesocket
#define WIN_FLAG true
#elif __linux__
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#define WIN_FLAG false
#elif __APPLE__
#include <mach/mach_init.h>
#include <mach/task_info.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/mount.h>
#include <sys/param.h>
#endif

#if __cplusplus >= 202002L && defined(__cpp_lib_format)
#include <format>
#else
#include <fmt/format.h>
#endif

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace Atom::Web {
bool isConnectedToInternet() {
    bool connected = false;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock != -1) {
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(80);
#ifdef _WIN32
        server.sin_addr.s_addr = inet_addr("8.8.8.8");
#else
        if (inet_pton(AF_INET, "8.8.8.8", &(server.sin_addr)) != -1) {
#endif
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) != -1) {
            connected = true;
        }
#ifdef _WIN32
        closesocket(sock);
#else
            close(sock);
        }
#endif
    }
    return connected;
}

std::vector<std::string> getNetworkStatus() {
    std::vector<std::string> net_connections;

#ifdef _WIN32
    DWORD size = 16384;
    MIB_TCPTABLE_OWNER_PID *tcp_table =
        reinterpret_cast<MIB_TCPTABLE_OWNER_PID *>(new char[size]);

    if (GetExtendedTcpTable(tcp_table, &size, true, AF_INET,
                            TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < tcp_table->dwNumEntries; i++) {
            MIB_TCPROW_OWNER_PID row = tcp_table->table[i];
            std::string local_address =
                inet_ntoa(*reinterpret_cast<IN_ADDR *>(&row.dwLocalAddr));
            std::string remote_address =
                inet_ntoa(*reinterpret_cast<IN_ADDR *>(&row.dwRemoteAddr));
            USHORT local_port = ntohs(row.dwLocalPort);
            USHORT remote_port = ntohs(row.dwRemotePort);

            std::string connection =
                "TCP " + local_address + ":" + std::to_string(local_port) +
                " -> " + remote_address + ":" + std::to_string(remote_port);

            net_connections.push_back(connection);
        }
    }

    delete[] reinterpret_cast<char *>(tcp_table);
#elif __linux__ || __APPLE__
    FILE *pipe = popen("netstat -an", "r");
    if (pipe) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            std::string line(buffer);

            if (line.find("tcp") != std::string::npos) {
                std::istringstream iss(line);
                std::vector<std::string> tokens(
                    std::istream_iterator<std::string>{iss},
                    std::istream_iterator<std::string>());

                std::string local_address;
                std::string remote_address;
                unsigned short local_port = 0;
                unsigned short remote_port = 0;

                if (tokens.size() >= 4) {
                    local_address = tokens[3];
                    local_port = std::stoi(
                        tokens[3].substr(tokens[3].find_last_of(':') + 1));
                }

                if (tokens.size() >= 5) {
                    remote_address = tokens[4];
                    remote_port = std::stoi(
                        tokens[4].substr(tokens[4].find_last_of(':') + 1));
                }

                std::string connection =
                    "TCP " + local_address + ":" + std::to_string(local_port) +
                    " -> " + remote_address + ":" + std::to_string(remote_port);

                net_connections.push_back(connection);
            }
        }

        pclose(pipe);
    }
#endif

    return net_connections;
}

bool checkAndKillProgramOnPort(int port) {
    bool success = false;

#ifdef _WIN32
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0) {
        LOG_F(ERROR, "Failed to initialize Windows Socket API: %d", ret);
        return false;
    }
#endif

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        LOG_F(ERROR, "Failed to create socket: {}", strerror(errno));
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        if (errno == EADDRINUSE) {
            DLOG_F(WARNING, "The port(%d) is already in use", port);

            std::string cmd;
#ifdef __cpp_lib_format
            cmd = std::format(
                "{}{}",
                (WIN_FLAG ? "netstat -ano | find \"LISTENING\" | find \""
                        : "lsof -i :{} -t"),
                port);
#else
            cmd = fmt::format(
                "{}{}",
                (WIN_FLAG ? "netstat -ano | find \"LISTENING\" | find \""
                        : "lsof -i :{} -t"),
                port);
#endif

            std::string pid_str =
                System::executeCommand(cmd, false, [](const std::string &line) {
                    return line.find("LISTENING") != std::string::npos;
                });
            if (pid_str.empty()) {
                LOG_F(ERROR,
                      "Failed to get the PID of the process on port({}): {}",
                      port, pid_str);
                success = false;
            } else {
                pid_str.erase(pid_str.find_last_not_of("\n") + 1);

                if (!pid_str.empty()) {
                    DLOG_F(INFO, "Killing the process on port(%d): PID={}",
                           port, pid_str);

                    std::string kill_cmd;
#ifdef __cpp_lib_format
                    kill_cmd = std::format(
                        "{}{}", (WIN_FLAG ? "taskkill /F /PID " : "kill "),
                        pid_str);
#else
                    kill_cmd = fmt::format(
                        "{}{}", (WIN_FLAG ? "taskkill /F /PID " : "kill "),
                        pid_str);
#endif

                    if (!System::executeCommand(
                             kill_cmd, false,
                             [pid_str](const std::string &line) {
                                 return line.find(pid_str) != std::string::npos;
                             })
                             .empty()) {
                        LOG_F(ERROR, "Failed to kill the process: {}", pid_str);
                        success = false;
                    } else {
                        DLOG_F(INFO, "The process({}) is killed successfully",
                               pid_str);
                        success = true;
                    }
                } else {
                    LOG_F(ERROR, "Failed to get process ID on port(%d)", port);
                    success = false;
                }
            }
        } else {
            LOG_F(ERROR, "Failed to bind socket: {}", strerror(errno));
            success = false;
        }
    } else {
        success = true;
    }

    close(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif
    return success;
}

bool isIPv4Format(const std::string &str) {
    std::regex urlRegex("\\d{2}\\.\\d{2}\\.\\d{2}\\.\\d{2}");
    return std::regex_match(str, urlRegex);
}

bool isIPv6Format(const std::string &str) {
    std::regex ipv6Regex("^(([0-9A-Fa-f]{1,4}):){7}([0-9A-Fa-f]{1,4})$");
    return std::regex_match(str, ipv6Regex);
}
}  // namespace Atom::Web
