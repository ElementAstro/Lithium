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

using json = nlohmann::json;
using websocketpp::server;

namespace OpenAPT
{
    class WebSocketServer
    {
    public:
        explicit WebSocketServer(int max_connections = 100);

        void run(int port);
        void stop();
        bool isRunning() const { return running_; }

        void sendMessage(websocketpp::connection_hdl hdl, const std::string &message);

    private:
        server<websocketpp::config::asio> server_;
        bool running_;
        std::string client_file_path_;
        int max_connections_;
        int active_connections_;
        std::mutex lock_;

        void onOpen(websocketpp::connection_hdl hdl);
        void onClose(websocketpp::connection_hdl hdl);
        void onMessage(websocketpp::connection_hdl hdl, server<websocketpp::config::asio>::message_ptr msg);
        void saveClientInfo(const std::string &ip, uint16_t port);
    };

} // namespace OpenAPT
