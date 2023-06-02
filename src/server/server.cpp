/*
 * server.cpp
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

Date: 2023-5-25

Description: Sockcet Server

**************************************************/

#include "server.hpp"

// 构造函数
Server::Server(std::string host, int port, int max_connections)
    : host_(host), port_(port), max_connections_(max_connections)
{
    // 初始化 libev 相关变量
    memset(&accept_watcher_, 0, sizeof(accept_watcher_));
    memset(&read_watcher_, 0, sizeof(read_watcher_));
    memset(&close_watcher_, 0, sizeof(close_watcher_));
    memset(&cleanup_timer_, 0, sizeof(cleanup_timer_));

    // 加载客户端信息
    loadClientInfo();
}

// 析构函数
Server::~Server()
{
    // 关闭所有连接
    for (auto &p : client_info_)
    {
        close(p.first);
    }

    // 保存客户端信息到 JSON 文件
    saveClientInfo();
}

// 启动服务器
void Server::start()
{
    // 创建 socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        spdlog::error("Failed to create listen socket: {}", strerror(errno));
        return;
    }

    // 设置 socket 地址重用选项
    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        spdlog::warn("Failed to set SO_REUSEADDR option: {}", strerror(errno));
    }

    // 绑定地址和端口号
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, host_.c_str(), &addr.sin_addr);
    addr.sin_port = htons(port_);
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        spdlog::error("Failed to bind listen socket: {}", strerror(errno));
        close(listenfd);
        return;
    }

    // 监听连接请求
    if (listen(listenfd, SOMAXCONN) == -1)
    {
        spdlog::error("Failed to listen on socket: {}", strerror(errno));
        close(listenfd);
        return;
    }

    // 初始化 accept 监听器
    ev_io_init(
        &accept_watcher_, [](EV_P_ ev_io *w, int revents)
        {
        auto server = static_cast<Server*>(w->data);
        server->onAccept(w->fd, revents); },
        listenfd, EV_READ);
    accept_watcher_.data = this;

    // 初始化 close 异步器
    ev_async_init(&close_watcher_, [](EV_P_ ev_async *w, int revents)
                  {
        auto server = static_cast<Server*>(w->data);
        server->onClose(); });
    close_watcher_.data = this;

    // 初始化 cleanup 定时器
    ev_timer_init(
        &cleanup_timer_, [](EV_P_ ev_timer *w, int revents)
        {
        auto server = static_cast<Server*>(w->data);
        server->cleanup(0); },
        0, 5);
    cleanup_timer_.data = this;
    ev_timer_start(EV_DEFAULT, &cleanup_timer_);

    // 开始事件循环
    running_ = true;
    spdlog::info("Server started, listening on {}:{}", host_, port_);
    ev_run(EV_DEFAULT, 0);

    // 关闭 socket
    close(listenfd);
}

// 发送消息
void Server::sendMessage(int fd, const std::string &payload)
{
    // 将字符串转换为 JSON 对象
    json data;
    try
    {
        data = json::parse(payload);
    }
    catch (const std::exception &e)
    {
        spdlog::error("Failed to parse JSON data: {}", e.what());
        return;
    }

    // 将 JSON 对象转换为字符串
    std::string message = data.dump();

    // 发送消息
    if (::send(fd, message.c_str(), message.length(), 0) == -1)
    {
        spdlog::error("Failed to send message: {}", strerror(errno));
    }
}

// 处理连接请求
void Server::onAccept(int fd, short events)
{
    // 接受连接请求
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &client_addrlen);
    if (connfd == -1)
    {
        spdlog::error("Failed to accept connection: {}", strerror(errno));
        return;
    }

    // 检查连接数是否超过限制
    if (connections_ >= max_connections_)
    {
        spdlog::warn("Maximum connections reached, closing connection from {}:{}",
                      inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        close(connfd);
        return;
    }

    // 设置 O_NONBLOCK 选项
    int flags = fcntl(connfd, F_GETFL, 0);
    fcntl(connfd, F_SETFL, flags | O_NONBLOCK);

    // 添加 read 监听器
    ev_io_init(
        &read_watcher_, [](EV_P_ ev_io *w, int revents)
        {
        auto server = static_cast<Server*>(w->data);
        server->onRead(w->fd, revents); },
        connfd, EV_READ);
    read_watcher_.data = this;

    // 启动 read 监听器
    ev_io_start(EV_DEFAULT, &read_watcher_);

    // 记录客户端信息
    ConnectionData data;
    data["host"] = inet_ntoa(client_addr.sin_addr);
    data["port"] = std::to_string(ntohs(client_addr.sin_port));
    client_info_[connfd] = data;
    connections_++;

    // 打印日志
    spdlog::info("New connection from {}:{}", data["host"], data["port"]);
}

// 处理消息
void Server::onRead(int fd, short events)
{
    // 读取数据
    char buf[4096];
    ssize_t n = recv(fd, buf, sizeof(buf), 0);
    if (n == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            spdlog::error("Failed to receive message: {}", strerror(errno));
            ev_io_stop(EV_DEFAULT, &read_watcher_);
            close(fd);
        }
        return;
    }
    else if (n == 0)
    {
        ev_io_stop(EV_DEFAULT, &read_watcher_);
        close(fd);
        return;
    }

    // 解析 JSON 数据
    std::string payload(buf, n);
    try
    {
        json data = json::parse(payload);
        spdlog::info("Received message from {}:{}",
                      client_info_[fd]["host"], client_info_[fd]["port"]);
        spdlog::info("{}\n", data.dump(4));

        // 异步调用处理消息的函数并回复客户端（根据编译器版本选择使用协程或线程）
#if __cplusplus >= 202002L
        asio::co_spawn(
            EV_DEFAULT_UC, [this, fd, data, payload]() -> asio::awaitable<void>
            { co_await sendMessage(fd, data, payload); },
            asio::detached);
#else
        std::thread process_thread([this, fd, data, payload]()
                                   { sendMessage(fd, data, payload); });
        process_thread.detach();
#endif
    }
    catch (const std::exception &e)
    {
        spdlog::error("Failed to parse JSON data: {}", e.what());
    }
}

// 处理断开连接
void Server::onClose()
{
    while (!client_info_.empty())
    {
        auto &p = *(client_info_.begin());
        int fd = p.first;
        close(fd);
        client_info_.erase(fd);
        connections_--;
        spdlog::info("Connection closed from {}:{}", p.second["host"], p.second["port"]);
    }
}

// 清理已断开连接的客户端信息
void Server::cleanup(int signum)
{
    for (auto it = client_info_.begin(); it != client_info_.end();)
    {
        int fd = it->first;
        if (fcntl(fd, F_GETFD) == -1 && errno == EBADF)
        {
            it = client_info_.erase(it);
            connections_--;
            spdlog::info("Connection closed from {}:{}", it->second["host"], it->second["port"]);
        }
        else
        {
            it++;
        }
    }
}

// 保存客户端信息到 JSON 文件
void Server::saveClientInfo()
{
    std::ofstream ofs(client_info_file_);
    if (!ofs)
    {
        spdlog::error("Failed to open JSON file: {}", client_info_file_);
        return;
    }

    json data;
    for (const auto &p : client_info_)
    {
        data[std::to_string(p.first)] = p.second;
    }

    ofs << data.dump(4);
    ofs.close();

    spdlog::info("Saved client info to JSON file: {}", client_info_file_);
}

// 加载客户端信息
void Server::loadClientInfo()
{
    std::ifstream ifs(client_info_file_);
    if (!ifs)
    {
        spdlog::warn("Failed to open JSON file: {}", client_info_file_);
        return;
    }

    json data;
    ifs >> data;
    ifs.close();

    for (const auto &item : data.items())
    {
        int fd = std::stoi(item.key());
        client_info_[fd] = item.value();
        connections_++;
    }

    spdlog::info("Loaded client info from JSON file: {}", client_info_file_);
}