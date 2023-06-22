/*
 * wsserver.hpp
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

Description: Websockcet Server

**************************************************/

#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <fstream>
#include <chrono>
#include <iostream>

#include "commander.hpp"

using json = nlohmann::json;
using websocketpp::server;

namespace OpenAPT
{
    /**
     * @brief 实现 WebSocket 服务器，提供消息传输和命令派发功能。
     */
    class WebSocketServer
    {
    public:
        /**
         * @brief 构造函数。
         *
         * @param max_connections 服务器最大连接数。
         */
        WebSocketServer(int max_connections);

        /**
         * @brief 启动 WebSocket 服务器。
         *
         * @param port 服务器监听的端口号。
         */
        void run(int port);

        /**
         * @brief 停止 WebSocket 服务器。
         */
        void stop();

        /**
         * @brief 向指定客户端发送消息。
         *
         * @param hdl 客户端连接句柄。
         * @param message 要发送的消息。
         */
        void sendMessage(websocketpp::connection_hdl hdl, const std::string &message);

    private:
        /**
         * @brief 当有新连接建立时的回调函数。
         *
         * @param hdl 新连接的句柄。
         */
        void onOpen(websocketpp::connection_hdl hdl);

        /**
         * @brief 当连接关闭时的回调函数。
         *
         * @param hdl 断开连接的句柄。
         */
        void onClose(websocketpp::connection_hdl hdl);

        /**
         * @brief 当收到客户端消息时的回调函数。
         *
         * @param hdl 发送消息的客户端连接句柄。
         * @param msg 收到的消息。
         */
        void onMessage(websocketpp::connection_hdl hdl, server<websocketpp::config::asio>::message_ptr msg);

        /**
         * @brief 处理客户端发送的命令。
         *
         * @param hdl 发送命令的客户端连接句柄。
         * @param payload 收到的命令负载。
         * @param data 命令携带的数据。
         */
        void processMessage(websocketpp::connection_hdl hdl, const std::string &payload, const json &data);

        /**
         * @brief 将客户端 IP 和端口号记录到文件中。
         *
         * @param ip 客户端 IP 地址。
         * @param port 客户端端口号。
         */
        void saveClientInfo(const std::string &ip, uint16_t port);

    private:
        bool running_;                                          ///< WebSocket 服务器是否正在运行。
        int max_connections_;                                   ///< WebSocket 服务器最大连接数。
        int active_connections_;                                ///< WebSocket 服务器活跃连接数。
        websocketpp::server<websocketpp::config::asio> server_; ///< WebSocket 服务器实例。
        std::mutex lock_;                                       ///< WebSocket 服务器线程锁。
        std::string client_file_path_;                          ///< 客户端信息保存文件路径。

        std::unique_ptr<CommandDispatcher> m_CommandDispatcher; ///< 命令派发器实例。

    public:
        /**
         * @brief 执行设备任务。
         *
         * @param m_params 设备任务参数。
         */
        void RunDeviceTask(const json &m_params);

        /**
         * @brief 获取设备信息。
         *
         * @param m_params 获取设备信息参数。
         */
        void GetDeviceInfo(const json &m_params);
    };

} // namespace OpenAPT
