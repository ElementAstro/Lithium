/*
 * WebSocketServer.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: WebSocket Server

**************************************************/

#include "WebSocketServer.hpp"

#include <thread>
#include <functional>
#include <iostream>

WebSocketServer::WebSocketServer()
{
	m_CommandDispatcher = std::make_unique<CommandDispatcher>();

	APTRegisterFunc("RunDeviceTask", &WebSocketServer::RunDeviceTask, this);
	APTRegisterFunc("GetDeviceInfo", &WebSocketServer::GetDeviceInfo, this);
}

void WebSocketServer::onPing(const WebSocket &socket, const oatpp::String &message)
{
	OATPP_LOGD(TAG, "onPing");
	socket.sendPong(message);
}

void WebSocketServer::onPong(const WebSocket &socket, const oatpp::String &message)
{
	OATPP_LOGD(TAG, "onPong");
}

void WebSocketServer::onClose(const WebSocket &socket, v_uint16 code, const oatpp::String &message)
{
	OATPP_LOGD(TAG, "onClose code=%d", code);
}

void WebSocketServer::readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{

	if (size == 0)
	{ // message transfer finished

		auto wholeMessage = m_messageBuffer.toString();
		m_messageBuffer.setCurrentPosition(0);

		OATPP_LOGD(TAG, "onMessage message='%s'", wholeMessage->c_str());

		/* Send message in reply */
		socket.sendOneFrameText("Hello from oatpp!: " + wholeMessage);

		// 检查 JSON 语法
		if (!nlohmann::json::accept(wholeMessage->c_str()))
		{
		}

		// 解析 JSON 数据
		try
		{
			nlohmann::json jdata = nlohmann::json::parse(wholeMessage->c_str());

			if (jdata.empty())
			{
				// spdlog::error("WebSocketServer::processMessage() data is empty");
			}
			nlohmann::json reply_data;
			try
			{
				// 解析 JSON 数据并获取参数。
				std::string name;
				nlohmann::json params;

				if (jdata.contains("name") && jdata.contains("params"))
				{
					name = jdata["name"].get<std::string>();
					params = jdata["params"].get<json>();
				}
				else
				{
					// spdlog::error("WebSocketServer::processMessage() missing parameter: name");
					reply_data = {{"error", "Missing parameter: name or params"}};
					socket.sendOneFrameText(reply_data.dump());
				}

				// TODO：在此处添加更多参数检查和处理逻辑。

				// 执行命令。
				if (m_CommandDispatcher->HasHandler(name))
				{
					m_CommandDispatcher->Dispatch(name, {});
				}
				// 发送回复。
				reply_data = {{"reply", "OK"}};
			}
			catch (const std::exception &e)
			{
				// spdlog::error("WebSocketServer::processMessage() exception: {}", e.what());
				reply_data = {{"error", e.what()}};
			}
			socket.sendOneFrameText(reply_data.dump());
		}
		catch (const std::exception &e)
		{
			// spdlog::error("WebSocketServer::onMessage() parse json failed: {}", e.what());
		}
	}
	else if (size > 0)
	{ // message frame received
		m_messageBuffer.writeSimple(data, size);
	}
}

void WebSocketServer::RunDeviceTask(const nlohmann::json &m_params)
{
	std::cout << "RunDeviceTask() is called!" << std::endl;
}

void WebSocketServer::GetDeviceInfo(const nlohmann::json &m_params)
{
	std::cout << "GetDeviceInfo() is called!" << std::endl;
}

std::atomic<v_int32> WSInstanceListener::SOCKETS(0);

void WSInstanceListener::onAfterCreate(const oatpp::websocket::WebSocket &socket, const std::shared_ptr<const ParameterMap> &params)
{

	SOCKETS++;
	OATPP_LOGD(TAG, "New Incoming Connection. Connection count=%d", SOCKETS.load());

	/* In this particular case we create one WebSocketServer per each connection */
	/* Which may be redundant in many cases */
	socket.setListener(std::make_shared<WebSocketServer>());
}

void WSInstanceListener::onBeforeDestroy(const oatpp::websocket::WebSocket &socket)
{

	SOCKETS--;
	OATPP_LOGD(TAG, "Connection closed. Connection count=%d", SOCKETS.load());
}