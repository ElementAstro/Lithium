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

Date: 2023-6-16

Description: Socket Server

**************************************************/

#include "server.hpp"

#include <spdlog/spdlog.h>

namespace OpenAPT
{
    SocketServer::SocketServer(int max_connections)
        : running_(false), max_connections_(max_connections), active_connections_(0), acceptor_(io_service_)
    {
        m_CommandDispatcher = std::make_unique<CommandDispatcher>();
    }

    void SocketServer::run(int port)
    {
        // 初始化和配置 IO 服务对象，创建 TCP acceptor 。
        tcp::endpoint endpoint(tcp::v4(), port);
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();

        running_ = true;
        do_accept();
        io_service_.run();
    }

    void SocketServer::stop()
    {
        running_ = false;
        io_service_.stop();
    }

    void SocketServer::sendMsg(const std::shared_ptr<tcp::socket> &socket, const json &message)
    {
        do_write(socket, message);
    }

    void SocketServer::do_accept()
    {
        if (!running_)
        {
            return;
        }

        auto socket = std::make_shared<tcp::socket>(io_service_);
        acceptor_.async_accept(*socket, [this, socket](const boost::system::error_code &error)
                               {
        if (!error) {
            // 处理新连接。
            spdlog::info(logger, "New connection from: {}:{}", socket->remote_endpoint().address().to_string(), socket->remote_endpoint().port());
            ++active_connections_;

            // 异步读取数据。
            do_read(socket);
        } else {
            spdlog::error(logger, "Error accepting connection: {}", error.message());
        }

        // 继续接受下一个连接请求。
        do_accept(); });
    }

    void SocketServer::do_read(const std::shared_ptr<tcp::socket> &socket)
    {
        auto self = shared_from_this();
        auto buffer = std::make_shared<boost::asio::streambuf>();
        boost::asio::async_read_until(*socket, *buffer, '\n',
                                      [this, socket, buffer](const boost::system::error_code &error, std::size_t bytes_transferred)
                                      {
                                          if (!error)
                                          {
                                              // 处理接收到的数据。
                                              std::istream input(buffer.get());
                                              std::string message_str;
                                              std::getline(input, message_str);

                                              spdlog::info(logger, "Received {} bytes of data: {}", bytes_transferred, message_str);

                                              // 解析 JSON 数据。
                                              json message;
                                              try
                                              {
                                                  message = json::parse(message_str);
                                              }
                                              catch (const std::exception &e)
                                              {
                                                  spdlog::error(logger, "Error parsing JSON data: {}", e.what());
                                              }

                                              // 处理消息。
                                              if (!message.empty())
                                              {
                                                  // TODO: 在这里添加对 message 的处理逻辑。

                                                  // 异步发送回复。
                                                  json reply = {{"status", "ok"}};
                                                  do_write(socket, reply);
                                              }

                                              // 继续异步读取数据。
                                              do_read(socket);
                                          }
                                          else
                                          {
                                              spdlog::error(logger, "Error receiving data: {}", error.message());

                                              // 关闭连接。
                                              socket->close();
                                              --active_connections_;
                                          }
                                      });
    }

    void SocketServer::do_write(const std::shared_ptr<tcp::socket> &socket, const json &message)
    {
        auto self = shared_from_this();
        boost::asio::async_write(*socket, boost::asio::buffer(message.dump() + "\n"),
                                 [this, socket, message](const boost::system::error_code &error, std::size_t bytes_transferred)
                                 {
                                     if (!error)
                                     {
                                         spdlog::info(logger, "Sent {} bytes of reply: {}", bytes_transferred, message.dump());
                                     }
                                     else
                                     {
                                         spdlog::error(logger, "Error sending data: {}", error.message());

                                         // 关闭连接。
                                         socket->close();
                                         --active_connections_;
                                     }
                                 });
    }

}
