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

namespace OpenAPT
{
    WebSocketServer::WebSocketServer(int max_connections = 0)
        : running_(false), max_connections_(max_connections), active_connections_(0)
    {
        // 设置 WebSocket 服务器的回调函数
        server_.set_open_handler(bind(&WebSocketServer::onOpen, this, std::placeholders::_1));
        server_.set_close_handler(bind(&WebSocketServer::onClose, this, std::placeholders::_1));
        server_.set_message_handler(bind(&WebSocketServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));

        // 获取保存客户端信息的文件路径
        client_file_path_ = "clients.json";

        m_CommandDispatcher = std::make_unique<CommandDispatcher>();

        m_CommandDispatcher->RegisterHandler("RunDeviceTask", &WebSocketServer::RunDeviceTask, this);
        m_CommandDispatcher->RegisterHandler("GetDeviceInfo", &WebSocketServer::GetDeviceInfo, this);
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
                // spdlog::error("WebSocketServer::run() exception: {}", e.what());
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
            // spdlog::error("WebSocketServer::stop() exception: {}", e.what());
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
            // spdlog::error("WebSocketServer::sendMessage() exception: {}", e.what());
        } // Check JSON syntax
    }

    void WebSocketServer::onOpen(websocketpp::connection_hdl hdl)
    {
        std::lock_guard<std::mutex> guard(lock_);

        if (max_connections_ > 0 && active_connections_ >= max_connections_)
        {
            // spdlog::warn("WebSocketServer::onOpen(): exceed max connections, refuse incoming connection");
            return;
        }

        // 获取客户端信息
        auto conn = server_.get_con_from_hdl(hdl);
        auto client_ip = conn->get_socket().remote_endpoint().address().to_string();
        auto client_port = conn->get_socket().remote_endpoint().port();
        // //spdlog::info("New client connected: {} : {}", client_ip, client_port);

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
        // //spdlog::info("Client disconnected: {} : {}", client_ip, client_port);

        active_connections_--;
    }

    void WebSocketServer::onMessage(websocketpp::connection_hdl hdl, server<websocketpp::config::asio>::message_ptr msg)
    {
        // 获取客户端信息
        if (msg == nullptr)
        {
            // spdlog::error("WebSocketServer::onMessage(): null message received");
            return;
        }
        auto conn = server_.get_con_from_hdl(hdl);
        auto client_ip = conn->get_socket().remote_endpoint().address().to_string();
        auto client_port = conn->get_socket().remote_endpoint().port();

        if (msg->get_opcode() == websocketpp::frame::opcode::text)
        {
            // 检查 JSON 语法
            if (!json::accept(msg->get_payload()))
            {
                // //spdlog::error("WebSocketServer::onMessage() invalid JSON syntax: {}", msg->get_payload());
                return;
            }

            // 解析 JSON 数据
            try
            {
                json data = json::parse(msg->get_payload());

                std::thread process_thread(&WebSocketServer::processMessage, this, conn, msg->get_payload(), data);
                process_thread.detach();
            }
            catch (const std::exception &e)
            {
                // spdlog::error("WebSocketServer::onMessage() parse json failed: {}", e.what());
            }
        }
        else
        {
            // spdlog::error("WebSocketServer::onMessage() unexpected message type received");
        }
    }

    void WebSocketServer::processMessage(websocketpp::connection_hdl hdl, const std::string &payload, const json &data)
    {
        if (payload.empty())
        {
            // spdlog::error("WebSocketServer::processMessage() payload is empty");
            return;
        }

        if (data.empty())
        {
            // spdlog::error("WebSocketServer::processMessage() data is empty");
            return;
        }

        try
        {
            // 解析 JSON 数据并获取参数。
            std::string name;
            json params;
            if (data.contains("name") && data.contains("params"))
            {
                name = data["name"].get<std::string>();
                params = data["params"].get<json>();
            }
            else
            {
                // spdlog::error("WebSocketServer::processMessage() missing parameter: name");
                json reply_data = {{"error", "Missing parameter: name or params"}};
                sendMessage(hdl, reply_data.dump());
                return;
            }

            // TODO：在此处添加更多参数检查和处理逻辑。

            // 执行命令。
            if (m_CommandDispatcher->HasHandler(name))
            {
                m_CommandDispatcher->Dispatch(name, {});
            }

            // 发送回复。
            json reply_data = {{"reply", "OK"}};
            sendMessage(hdl, reply_data.dump()); // 将 JSON 对象转换为字符串
        }
        catch (const std::exception &e)
        {
            // spdlog::error("WebSocketServer::processMessage() exception: {}", e.what());
            json reply_data = {{"error", e.what()}};
            sendMessage(hdl, reply_data.dump());
        }
    }

    void WebSocketServer::saveClientInfo(const std::string &ip, uint16_t port)
    {
        // 读取已有的客户端信息列表
        try
        {
            std::ifstream fin(client_file_path_);
            json clients;
            if (fin >> clients)
            {
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
                    if (clients.is_array())
                    {
                        json client_info = {{"ip", ip}, {"port", port}};
                        clients.push_back(client_info);
                    }
                    else if (clients.is_object())
                    {
                        json client_info = {{"ip", ip}, {"port", port}};
                        clients["client" + std::to_string(clients.size() + 1)] = client_info;
                    }
                    else
                    {
                        throw std::runtime_error("Invalid JSON file");
                    }

                    // 将客户端信息列表保存到文件中
                    std::ofstream fout(client_file_path_);
                    if (fout << clients.dump(4))
                    {
                        // spdlog::debug("Client info saved: {}",clients.dump(4));
                    }
                    else
                    {
                        throw std::runtime_error("Failed to write client info to file");
                    }
                }
            }
            else
            {
                throw std::runtime_error("Failed to read client info from file");
            }
        }
        catch (const std::exception &e)
        {
            // spdlog::error("Exception occurred while saving client info: {}", e.what());
        }
    }

    //-------------------------------------

    void WebSocketServer::RunDeviceTask(const json &m_params)
    {
        std::cout << "RunDeviceTask() is called!" << std::endl;
    }

    void WebSocketServer::GetDeviceInfo(const json &m_params)
    {
        std::cout << "GetDeviceInfo() is called!" << std::endl;
    }

}
