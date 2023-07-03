/*
 * phd2client.cpp
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

Description: PHD2 CLient Interface

**************************************************/

#include "phd2client.hpp"
#include <iostream>
#include <boost/asio/bind_executor.hpp>
#include <spdlog/spdlog.h>

PHD2Client::PHD2Client(boost::asio::io_context &io_context, const std::string &host, const unsigned short port)
    : resolver_(io_context), socket_(io_context), host_(host), port_(port), strand_(io_context)
{
}

PHD2Client::~PHD2Client() = default;

void PHD2Client::run()
{
    tcp::resolver::query query(host_, std::to_string(port_));
    resolver_.async_resolve(
        query, boost::asio::bind_executor(strand_, [self = shared_from_this()](const auto &error_code, const auto &endpoint_it)
                                          {
          if (error_code)
          {
              spdlog::error("Error resolving host: {}", error_code.message());
              return;
          }
           const auto endpoint = *endpoint_it;
          self->socket_.async_connect(endpoint, boost::asio::bind_executor(self->strand_, [self](const auto& error_code)
          {
              if (error_code)
              {
                  spdlog::error("Error connecting to host: {}", error_code.message());
                  return;
              }
               spdlog::info("Connected to PHD2");
              self->read();
          })); }));
}

void PHD2Client::send(const json &data)
{
    const auto message = data.dump();
    boost::asio::async_write(socket_, boost::asio::buffer(message),
                             boost::asio::bind_executor(strand_, [self = shared_from_this()](const auto &error_code, const auto /*length*/)
                                                        {
                                 if (error_code)
                                 {
                                     spdlog::error("Error sending message: {}", error_code.message());
                                     return;
                                 } }));
}

void PHD2Client::read()
{
    socket_.async_read_some(boost::asio::buffer(buffer_),
                            boost::asio::bind_executor(strand_, [self = shared_from_this(),this](const auto &error_code, const auto length)
                                                       {
                                if (error_code)
                                {
                                    spdlog::error("Error reading from PHD2: {}", error_code.message());
                                    return;
                                }
                                 const auto received_data = std::string_view(buffer_.data(), length);
                                try
                                {
                                    const auto received_json = json::parse(received_data);
                                     // 按照PHD2协议进行解析和处理返回的消息
                                }
                                catch (const std::exception& e)
                                {
                                    spdlog::error("Error parsing PHD2 response: {}", e.what());
                                }
                                 self->read(); }));
}