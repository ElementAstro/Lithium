/*
 * utils.cpp
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

Date: 2023-6-17

Description: Network Utils

**************************************************/

#include "utils.hpp"

#include <cstdlib>
#include <sstream>
#include <string>
#include <cstring>
#include <iterator>
#include <regex>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <iphlpapi.h>
#define close closesocket
#elif __linux__
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#elif __APPLE__
#include <mach/mach_init.h>
#include <mach/task_info.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#endif

#if __cplusplus >= 202002L
#include <format>
#endif
#include "loguru/loguru.hpp"

bool IsConnectedToInternet()
{
    bool connected = false;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock != -1)
    {
        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(80);
#ifdef _WIN32
        server.sin_addr.s_addr = inet_addr("8.8.8.8");
#else
        if (inet_pton(AF_INET, "8.8.8.8", &(server.sin_addr)) != -1)
        {
#endif
        if (connect(sock, (struct sockaddr *)&server, sizeof(server)) != -1)
        {
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

std::vector<std::string> GetNetworkStatus()
{
    std::vector<std::string> net_connections;

#ifdef _WIN32
    DWORD size = 16384;
    MIB_TCPTABLE_OWNER_PID *tcp_table = reinterpret_cast<MIB_TCPTABLE_OWNER_PID *>(new char[size]);

    if (GetExtendedTcpTable(tcp_table, &size, true, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR)
    {
        for (DWORD i = 0; i < tcp_table->dwNumEntries; i++)
        {
            MIB_TCPROW_OWNER_PID row = tcp_table->table[i];
            std::string local_address = inet_ntoa(*reinterpret_cast<IN_ADDR *>(&row.dwLocalAddr));
            std::string remote_address = inet_ntoa(*reinterpret_cast<IN_ADDR *>(&row.dwRemoteAddr));
            USHORT local_port = ntohs(row.dwLocalPort);
            USHORT remote_port = ntohs(row.dwRemotePort);

            std::string connection = "TCP " + local_address + ":" + std::to_string(local_port) +
                                     " -> " + remote_address + ":" + std::to_string(remote_port);

            net_connections.push_back(connection);
        }
    }

    delete[] reinterpret_cast<char *>(tcp_table);
#elif __linux__ || __APPLE__
    FILE *pipe = popen("netstat -an", "r");
    if (pipe)
    {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            std::string line(buffer);

            if (line.find("tcp") != std::string::npos)
            {
                std::istringstream iss(line);
                std::vector<std::string> tokens(std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>());

                std::string local_address;
                std::string remote_address;
                unsigned short local_port = 0;
                unsigned short remote_port = 0;

                if (tokens.size() >= 4)
                {
                    local_address = tokens[3];
                    local_port = std::stoi(tokens[3].substr(tokens[3].find_last_of(':') + 1));
                }

                if (tokens.size() >= 5)
                {
                    remote_address = tokens[4];
                    remote_port = std::stoi(tokens[4].substr(tokens[4].find_last_of(':') + 1));
                }

                std::string connection = "TCP " + local_address + ":" + std::to_string(local_port) +
                                         " -> " + remote_address + ":" + std::to_string(remote_port);

                net_connections.push_back(connection);
            }
        }

        pclose(pipe);
    }
#endif

    return net_connections;
}

bool CheckAndKillProgramOnPort(int port)
{
#ifdef _WIN32
    // 初始化 Windows socket API
    WSADATA wsaData;
    int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (ret != 0)
    {
        LOG_F(ERROR, "Failed to initialize Windows Socket API: %d", ret);
        return false;
    }
#endif

    // 创建一个新的套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_F(ERROR, "Failed to create socket: %s", strerror(errno));
#ifdef _WIN32
        WSACleanup();
#endif
        return false;
    }

    // 绑定到指定端口上
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        if (errno == EADDRINUSE)
        {
            LOG_F(WARNING, "The port(%d) is already in use", port);

            // 获取占用端口的进程 ID
            std::string cmd;
#ifdef _WIN32
            cmd = std::format("netstat -ano | find \"LISTENING\" | find \"{}\"", port);
#else
            cmd = std::format("lsof -i :{} -t", port);
#endif

            FILE *fp = popen(cmd.c_str(), "r");
            if (fp == nullptr)
            {
                LOG_F(ERROR, "Failed to execute command: %s", cmd.c_str());
                close(sockfd);
#ifdef _WIN32
                WSACleanup();
#endif
                return false;
            }

            char buf[1024];
            std::string pid_str;
            while (fgets(buf, 1024, fp) != nullptr)
            {
                pid_str += buf;
            }
            pclose(fp);
            pid_str.erase(pid_str.find_last_not_of("\n") + 1);

            // 如果获取到了 PID，则杀死该进程
            if (!pid_str.empty())
            {
                LOG_F(INFO, "Killing the process on port(%d): PID=%s", port, pid_str.c_str());

#ifdef _WIN32
                ret = std::system(std::format("taskkill /F /PID {}", pid_str).c_str());
#else
                int ret = std::system(std::format("kill {}", pid_str).c_str());
#endif

                if (ret != 0)
                {
                    LOG_F(ERROR, "Failed to kill the process: %s", pid_str.c_str());
                    close(sockfd);
#ifdef _WIN32
                    WSACleanup();
#endif
                    return false;
                }

                LOG_F(INFO, "The process(%s) is killed successfully", pid_str.c_str());
            }
            else
            {
                LOG_F(ERROR, "Failed to get process ID on port(%d)", port);
                close(sockfd);
#ifdef _WIN32
                WSACleanup();
#endif
                return false;
            }
        }
        else
        {
            LOG_F(ERROR, "Failed to bind socket: %s", strerror(errno));
            close(sockfd);
#ifdef _WIN32
            WSACleanup();
#endif
            return false;
        }
    }

    close(sockfd);
#ifdef _WIN32
    WSACleanup();
#endif
    return true;
}

bool isIPv4Format(const std::string &str)
{
    std::regex urlRegex("\\d{2}\\.\\d{2}\\.\\d{2}\\.\\d{2}");
    return std::regex_match(str, urlRegex);
}

bool isIPv6Format(const std::string &str)
{
    std::regex ipv6Regex("^(([0-9A-Fa-f]{1,4}):){7}([0-9A-Fa-f]{1,4})$");
    return std::regex_match(str, ipv6Regex);
}