/*
 * deviceio.hpp
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

Date: 2023-6-1

Description: Device IO Module

*************************************************/

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <functional>

#include "lidriver/event/eventloop.hpp"

class EventLoop;

class SocketServer
{
public:
    using MessageHandler = std::function<void(const std::string &)>;

    SocketServer(EventLoop &eventLoop, int port);

    void start();
    void stop();

    bool is_running();

    void setMessageHandler(MessageHandler handler);
    void sendMessage(int clientSocket, const std::string &message);

private:
    EventLoop &eventLoop;
    int port;
    int serverSocket;
    bool running;
    MessageHandler messageHandler;

    void acceptClientConnection();
    void handleClientMessage(int clientSocket);
};

#endif // SOCKET_SERVER_H
