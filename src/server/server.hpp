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

Date: 2023-6-16

Description: Socket Server

**************************************************/

#pragma once

#include <iostream>
#include <mutex>
#include <string>
#include <memory>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

#include <nlohmann/json.hpp>

#include "commander.hpp"

using boost::asio::ip::tcp;
using json = nlohmann::json;

namespace OpenAPT
{
    /**
     * @brief 实现 Socket 服务器，提供消息传输功能。
     */
    class SocketServer : public std::enable_shared_from_this<SocketServer>
    {
    public:
        /**
         * @brief 构造函数。
         *
         * @param max_connections 服务器最大连接数。
         */
        SocketServer(int max_connections);

        /**
         * @brief 启动 Socket 服务器。
         *
         * @param port 服务器监听的端口号。
         */
        void run(int port);

        /**
         * @brief 停止 Socket 服务器。
         */
        void stop();

        /**
         * @brief 向指定客户端发送消息。
         *
         * @param socket 客户端连接的套接字。
         * @param message 要发送的消息。
         */
        void sendMsg(const std::shared_ptr<tcp::socket> &socket, const json &message);

    private:
        /**
         * @brief 接收新的客户端连接请求。
         *
         * @note 此函数为异步操作。
         */
        void do_accept();

        /**
         * @brief 读取客户端发送的消息。
         *
         * @param socket 客户端连接的套接字。
         *
         * @note 此函数为异步操作。
         */
        void do_read(const std::shared_ptr<tcp::socket> &socket);

        /**
         * @brief 向客户端发送消息。
         *
         * @param socket 客户端连接的套接字。
         * @param message 要发送的消息。
         *
         * @note 此函数为异步操作。
         */
        void do_write(const std::shared_ptr<tcp::socket> &socket, const json &message);

    private:
        bool running_;                       ///< Socket 服务器是否正在运行。
        int max_connections_;                ///< Socket 服务器最大连接数。
        int active_connections_;             ///< Socket 服务器活跃连接数。
        boost::asio::io_service io_service_; ///< asio IO 服务实例。
        tcp::acceptor acceptor_;             ///< 用于接收新连接的套接字。
        std::mutex lock_;                    ///< Socket 服务器线程锁。

        std::unique_ptr<CommandDispatcher> m_CommandDispatcher; ///< 命令派发器实例。
    };

}
