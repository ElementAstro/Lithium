/*
 * wsserver.cpp
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

#include "wsserver.hpp"

#include <spdlog/spdlog.h>

namespace OpenAPT
{
    WebSocketServer::WebSocketServer(int max_connections)
        : running_(false), max_connections_(max_connections), active_connections_(0)
    {
        // 设置 WebSocket 服务器的回调函数
        server_.set_open_handler(bind(&WebSocketServer::onOpen, this, ::_1));
        server_.set_close_handler(bind(&WebSocketServer::onClose, this, ::_1));
        server_.set_message_handler(bind(&WebSocketServer::onMessage, this, ::_1, ::_2));

        // 获取保存客户端信息的文件路径
        client_file_path_ = "clients.json";
    }

    void WebSocketServer::run(int port)
    {
        if (running_)
        {
            return;
        }

        // 初始化 Websocket 服务器配置
        server_.init_asio();
        server_.set_reuse_addr(true);
        server_.set_max_http_body_size(1024 * 1024); // 1MB
        server_.listen(port);
        server_.start_accept();
        running_ = true;

        // 运行 WebSocket 服务器
        while (running_)
        {
            try
            {
                server_.run();
            }
            catch (const std::exception &e)
            {
                spdlog::error("WebSocketServer::run() exception: {}", e.what());
            }
        }
    }

    void WebSocketServer::stop()
    {
        if (!running_)
        {
            return;
        }
        try
        {
            server_.stop();
            running_ = false;
        }
        catch (const std::exception &e)
        {
            spdlog::error("WebSocketServer::stop() exception: {}", e.what());
        }
    }

    void WebSocketServer::sendMessage(websocketpp::connection_hdl hdl, const std::string &message)
    {
        try
        {
            server_.send(hdl, message, websocketpp::frame::opcode::text);
        }
        catch (const std::exception &e)
        {
            spdlog::error("WebSocketServer::sendMessage() exception: {}", e.what());
        }
    }

    void WebSocketServer::onOpen(websocketpp::connection_hdl hdl)
    {
        std::lock_guard<std::mutex> guard(lock_);

        if (max_connections_ > 0 && active_connections_ >= max_connections_)
        {
            spdlog::warn("WebSocketServer::onOpen(): exceed max connections, refuse incoming connection");
            return;
        }

        // 获取客户端信息
        auto conn = server_.get_con_from_hdl(hdl);
        auto client_ip = conn->get_socket().remote_endpoint().address().to_string();
        auto client_port = conn->get_socket().remote_endpoint().port();
        spdlog::info("New client connected: {} : {}", client_ip, client_port);

        // 记录客户端信息到 JSON 文件中
        saveClientInfo(client_ip, client_port);

        active_connections_++;
    }

    void WebSocketServer::onClose(websocketpp::connection_hdl hdl)
    {
        std::lock_guard<std::mutex> guard(lock_);

        // 获取客户端信息
        auto conn = server_.get_con_from_hdl(hdl);
        auto client_ip = conn->get_socket().remote_endpoint().address().to_string();
        auto client_port = conn->get_socket().remote_endpoint().port();
        spdlog::info("Client disconnected: {} : {}", client_ip, client_port);

        active_connections_--;
    }

    void WebSocketServer::onMessage(websocketpp::connection_hdl hdl, server<websocketpp::config::asio>::message_ptr msg)
    {
        // 获取客户端信息
        auto conn = server_.get_con_from_hdl(hdl);
        auto client_ip = conn->get_socket().remote_endpoint().address().to_string();
        auto client_port = conn->get_socket().remote_endpoint().port();

        if (msg->get_opcode() == websocketpp::frame::opcode::text)
        {
            // 解析 JSON 数据
            try
            {
                json data = json::parse(msg->get_payload());
                spdlog::info("Received message from {} : {}", client_ip, client_port);
                spdlog::info("{}\n", data.dump(4));

                // 异步调用处理消息的函数并回复客户端（根据编译器版本选择使用协程或线程）
#if __cplusplus >= 202002L
                asio::co_spawn(
                    server_.get_io_service(), [this, conn, data, payload = msg->get_payload()]() -> asio::awaitable<void>
                    { co_await processMessage(conn, data, payload); },
                    asio::detached);
#else
                std::thread process_thread(&WebSocketServer::processMessage, this, conn, data, msg->get_payload());
                process_thread.detach();
#endif
            }
            catch (const std::exception &e)
            {
                spdlog::error("WebSocketServer::onMessage() parse json failed: {}", e.what());
            }
        }
        else
        {
            spdlog::error("WebSocketServer::onMessage() unexpected message type received");
        }
    }

    void WebSocketServer::processMessage(websocketpp::connection_hdl hdl, const json &data, const std::string &payload)
    {
        // 处理消息并回复客户端
        json reply_data = {{"reply", payload + " - OK"}};
        try
        {
            // 模拟耗时操作
#if __cplusplus >= 202002L
            co_await asio::this_coro::executor->context().get_scheduler()->schedule_after(std::chrono::seconds(2));
            co_await sendAsync(hdl, reply_data.dump());
#else
            std::this_thread::sleep_for(std::chrono::seconds(2));
            sendMessage(hdl, reply_data.dump());
#endif
        }
        catch (const std::exception &e)
        {
            spdlog::error("WebSocketServer::processMessage() exception: {}", e.what());
        }
    }

    void WebSocketServer::saveClientInfo(const std::string &ip, uint16_t port)
    {
        // 读取已有的客户端信息列表
        try
        {
            std::ifstream fin(client_file_path_);
            if (fin)
            {
                json clients;
                fin >> clients;
                // 检查如果客户端信息已经存在于列表中，则不再添加
                bool exists = false;
                for (auto &client : clients)
                {
                    if (client["ip"] == ip && client["port"] == port)
                    {
                        exists = true;
                        break;
                    }
                }
                if (!exists)
                {
                    // 将新的客户端信息添加到列表中
                    json client_info = {{"ip", ip}, {"port", port}};
                    clients.push_back(client_info);
                    // 将客户端信息列表保存到文件中
                    std::ofstream fout(client_file_path_);
                    if (fout)
                    {
                        fout << clients.dump(4);
                        fout.close();
                    }
                    else
                    {
                        throw std::runtime_error("Failed to open client info file for writing");
                    }
                }
            }
            else
            {
                throw std::runtime_error("Failed to open client info file for reading");
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Exception occurred while saving client info: " << e.what() << std::endl;
        }
    }
}
