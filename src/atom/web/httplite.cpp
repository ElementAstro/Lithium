/*
 * httplite.hpp
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

Date: 2023-11-3

Description: Simple Http Client

**************************************************/

#include "httplite.hpp"

#include <cstring>

#include "loguru/loguru.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

std::string httpRequest(const std::string &url, const std::string &method, std::function<void(const std::string &)> errorHandler)
{
    std::string response;

    // 解析URL
    std::string host;
    std::string path;
    size_t pos = url.find("://");
    if (pos != std::string::npos)
    {
        pos += 3;
        size_t slashPos = url.find('/', pos);
        if (slashPos != std::string::npos)
        {
            host = url.substr(pos, slashPos - pos);
            path = url.substr(slashPos);
        }
        else
        {
            errorHandler("Invalid URL");
            return response;
        }
    }
    else
    {
        errorHandler("Invalid URL");
        return response;
    }

    // 创建socket
    int socketfd;
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        errorHandler("Failed to initialize Winsock");
        return response;
    }
    socketfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
#endif

    // 连接服务器
    struct sockaddr_in serverAddr
    {
    };
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(80); // HTTP默认端口为80
#ifdef _WIN32
    if (InetPton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0)
#else
    if (inet_pton(AF_INET, host.c_str(), &(serverAddr.sin_addr)) <= 0)
#endif
    {
        errorHandler("Failed to parse server address");
#ifdef _WIN32
        closesocket(socketfd);
        WSACleanup();
#else
        close(socketfd);
#endif
        return response;
    }
    if (connect(socketfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        errorHandler("Failed to connect to server");
#ifdef _WIN32
        closesocket(socketfd);
        WSACleanup();
#else
        close(socketfd);
#endif
        return response;
    }

    // 构建请求
    std::string request = method + " " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Connection: close\r\n\r\n";

    // 发送请求
    if (send(socketfd, request.c_str(), request.length(), 0) < 0)
    {
        errorHandler("Failed to send request");
#ifdef _WIN32
        closesocket(socketfd);
        WSACleanup();
#else
        close(socketfd);
#endif
        return response;
    }

    // 接收响应
    char buffer[4096];
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(socketfd, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead <= 0)
        {
            break;
        }
        response += buffer;
    }

// 关闭socket
#ifdef _WIN32
    closesocket(socketfd);
    WSACleanup();
#else
    close(socketfd);
#endif

    return response;
}

/*
void handleError(const std::string &errorMessage)
{
    LOG_F(ERROR, "Error: %s", errorMessage.c_str());
    // 可以在这里进行其他的错误处理操作
}

int main()
{
    loguru::init();

    std::string url = "http://example.com";

    std::string getResponse = httpRequest(url, "GET", handleError);
    if (!getResponse.empty())
    {
        LOG_F(INFO, "GET Response: %s", getResponse.c_str());
    }

    // 发送POST请求
    std::string postData = "key1=value1&key2=value2";
    std::string postResponse = httpRequest(url, "POST", handleError);
    if (!postResponse.empty())
    {
        LOG_F(INFO, "POST Response: %s", postResponse.c_str());
    }

    // 发送PUT请求
    std::string putData = "new content";
    std::string putResponse = httpRequest(url, "PUT", handleError);
    if (!putResponse.empty())
    {
        LOG_F(INFO, "PUT Response: %s", putResponse.c_str());
    }

    // 发送DELETE请求
    std::string deleteResponse = httpRequest(url, "DELETE", handleError);
    if (!deleteResponse.empty())
    {
        LOG_F(INFO, "DELETE Response: %s", deleteResponse.c_str());
    }

    loguru::shutdown();

    return 0;
}
*/