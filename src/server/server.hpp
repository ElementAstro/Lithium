/*
 * server.hpp
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

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <ev.h>

// 类型别名
using json = nlohmann::json;
using ConnectionData = std::unordered_map<std::string, std::string>;

class Server {
public:
    // 构造函数和析构函数
    Server(std::string host, int port, int max_connections);
    ~Server();

    // 启动服务器
    void start();

    // 发送消息
    void sendMessage(int fd, const std::string& payload);

private:
    // 服务器状态
    bool running_ = false;

    // libev 相关变量
    ev_io accept_watcher_;
    ev_io read_watcher_;
    ev_async close_watcher_;
    ev_timer cleanup_timer_;

    // 最大连接数
    int max_connections_;

    // 监听地址和端口号
    std::string host_;
    int port_;

    // 客户端信息
    std::unordered_map<int, ConnectionData> client_info_;

    // 客户端连接数
    int connections_ = 0;

    // 保存客户端信息的 JSON 文件路径
    std::string client_info_file_ = "client_info.json";

    // 日志器
    std::shared_ptr<spdlog::logger> logger_;

    // 处理连接请求
    void onAccept(int fd, short events);

    // 处理消息
    void onRead(int fd, short events);

    // 处理断开连接
    void onClose();

    // 清理已断开连接的客户端信息
    void cleanup(int signum);

    // 保存客户端信息到 JSON 文件
    void saveClientInfo();

    // 加载客户端信息
    void loadClientInfo();
};
