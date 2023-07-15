/*
 * WebSocketServer.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.	If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: WebSocket Server

**************************************************/

#ifndef WebSocketServer_hpp
#define WebSocketServer_hpp

#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp-websocket/WebSocket.hpp"

#include "nlohmann/json.hpp"

#include "modules/server/commander.hpp"

class WebSocketServer : public oatpp::websocket::WebSocket::Listener
{
private:
	static constexpr const char *TAG = "WebSocketServer";

private:
	oatpp::data::stream::BufferOutputStream m_messageBuffer;

	std::unique_ptr<CommandDispatcher> m_CommandDispatcher;

	template <typename ClassType>
	void APTRegisterFunc(const std::string &name, void (ClassType::*handler)(const nlohmann::json &), ClassType *instance)
	{
		m_CommandDispatcher->RegisterHandler(name, handler, instance);
	}

	bool APTRunFunc(const std::string &name, const nlohmann::json &params)
	{
		if (m_CommandDispatcher->HasHandler(name))
		{
			m_CommandDispatcher->Dispatch(name, params);
			return true;
		}
		return false;
	}

public:
	WebSocketServer();

public:
	void RunDeviceTask(const nlohmann::json &m_params);

	void GetDeviceInfo(const nlohmann::json &m_params);

public:
	void onPing(const WebSocket &socket, const oatpp::String &message) override;

	void onPong(const WebSocket &socket, const oatpp::String &message) override;

	void onClose(const WebSocket &socket, v_uint16 code, const oatpp::String &message) override;

	void readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
};

/**
 * Listener on new WebSocket connections.
 */
class WSInstanceListener : public oatpp::websocket::ConnectionHandler::SocketInstanceListener
{
private:
	static constexpr const char *TAG = "WebSocketInstanceListener";

public:
	/**
	 * Counter for connected clients.
	 */
	static std::atomic<v_int32> SOCKETS;

public:
	void onAfterCreate(const oatpp::websocket::WebSocket &socket, const std::shared_ptr<const ParameterMap> &params) override;

	void onBeforeDestroy(const oatpp::websocket::WebSocket &socket) override;
};

#endif // WebSocketServer_hpp
