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

#include <functional>
#include <iostream>
#include <version>
#include <thread>

std::unordered_map<std::string, Lithium::DeviceType> DeviceTypeMap = {
	{"Camera", Lithium::DeviceType::Camera},
	{"Telescope", Lithium::DeviceType::Telescope},
	{"Focuser", Lithium::DeviceType::Focuser},
	{"FilterWheel", Lithium::DeviceType::FilterWheel},
	{"Solver", Lithium::DeviceType::Solver},
	{"Guider", Lithium::DeviceType::Guider}};

WebSocketServer::WebSocketServer(const std::shared_ptr<AsyncWebSocket> &socket)
{
	m_CommandDispatcher = std::make_unique<CommandDispatcher>();

	LiRegisterFunc("RunDeviceTask", &WebSocketServer::RunDeviceTask);
	LiRegisterFunc("GetDeviceInfo", &WebSocketServer::GetDeviceInfo);
	LiRegisterFunc("GetDeviceList", &WebSocketServer::GetDeviceList);
	LiRegisterFunc("AddDevice", &WebSocketServer::AddDevice);
	LiRegisterFunc("AddDeviceLibrary", &WebSocketServer::AddDeviceLibrary);
	LiRegisterFunc("RemoveDevice", &WebSocketServer::RemoveDevice);
	LiRegisterFunc("RemoveDeviceByName", &WebSocketServer::RemoveDevicesByName);
	LiRegisterFunc("RemoveDeviceLibrary", &WebSocketServer::RemoveDeviceLibrary);

	LiRegisterFunc("CreateProcess", &WebSocketServer::CreateProcessLi);
	LiRegisterFunc("RunScript", &WebSocketServer::RunScript);
	LiRegisterFunc("TerminateProcessByName", &WebSocketServer::TerminateProcessByName);
	LiRegisterFunc("GetRunningProcesses", &WebSocketServer::GetRunningProcesses);
	LiRegisterFunc("GetProcessOutput", &WebSocketServer::GetProcessOutput);
	
	LiRegisterFunc("AddTask", &WebSocketServer::AddTask);
	LiRegisterFunc("InsertTask", &WebSocketServer::InsertTask);
	LiRegisterFunc("ExecuteAllTasks", &WebSocketServer::ExecuteAllTasks);
	LiRegisterFunc("StopTask", &WebSocketServer::StopTask);
	LiRegisterFunc("ExecuteTaskByName", &WebSocketServer::ExecuteTaskByName);
	LiRegisterFunc("ModifyTask", &WebSocketServer::ModifyTask);
	LiRegisterFunc("ModifyTaskByName", &WebSocketServer::ModifyTaskByName);
	LiRegisterFunc("DeleteTask", &WebSocketServer::DeleteTask);
	LiRegisterFunc("DeleteTaskByName", &WebSocketServer::DeleteTaskByName);
	LiRegisterFunc("QueryTaskByName", &WebSocketServer::QueryTaskByName);

	LiRegisterFunc("RunChaiCommand", &WebSocketServer::runChaiCommand);
	LiRegisterFunc("RunChaiMultiCommand", &WebSocketServer::runChaiMultiCommand);
	LiRegisterFunc("RunChaiScript", &WebSocketServer::runChaiScript);
	LiRegisterFunc("LoadChaiScript", &WebSocketServer::loadChaiFile);
}

WebSocketServer::~WebSocketServer()
{
}

#if ENABLE_ASYNC
oatpp::async::CoroutineStarter WebSocketServer::onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	DLOG_F(INFO, "onPing");
	return socket->sendPongAsync(message);
}

oatpp::async::CoroutineStarter WebSocketServer::onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message)
{
	DLOG_F(INFO, "onPong");
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WebSocketServer::onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message)
{
	DLOG_F(INFO, "onClose code=%d", code);
	return nullptr; // do nothing
}

oatpp::async::CoroutineStarter WebSocketServer::readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{
	if (size == 0)
	{
		nlohmann::json reply_data;
		auto wholeMessage = m_messageBuffer.toString();
		m_messageBuffer.setCurrentPosition(0);
		// DLOG_F(INFO, "onMessage message='%s'", wholeMessage->c_str());
		if (!nlohmann::json::accept(wholeMessage->c_str()))
		{
			LOG_F(ERROR, "Message is not in JSON format");
			reply_data["error"] = "Invalid Format";
			reply_data["message"] = "Message is not in JSON format";
			return socket->sendOneFrameTextAsync(reply_data.dump());
		}
		try
		{
			nlohmann::json jdata = nlohmann::json::parse(wholeMessage->c_str());
			try
			{
				if (jdata.empty())
				{
					LOG_F(ERROR, "WebSocketServer::processMessage() data is empty");
					reply_data["error"] = "Invalid Parameters";
					reply_data["message"] = "Data is empty";
					return socket->sendOneFrameTextAsync(reply_data.dump());
				}
				if (jdata.contains("name") && jdata.contains("params"))
				{
					const std::string name = jdata["name"].get<std::string>();
					if (m_CommandDispatcher->HasHandler(name))
					{
						json res = m_CommandDispatcher->Dispatch(name, jdata["params"].get<json>());
						if (res.contains("error"))
						{
							LOG_F(ERROR, "Failed to run command %s , error : %s", name.c_str(), res.dump().c_str());
							reply_data["error"] = res["error"];
						}
						else
						{
							DLOG_F(INFO, "Run command %s successfully", name.c_str());
							reply_data = {{"reply", "OK"}};
						}
					}
				}
				else
				{
					LOG_F(ERROR, "[ASYNC MODE] WebSocketServer::readMessage() missing parameter: name or params");
					reply_data = {{"error", "Invalid Parameters"}, {"message", "Missing parameter: name or params"}};
				}
			}
			catch (const std::exception &e)
			{
				LOG_F(ERROR, "WebSocketServer::readMessage() run command failed: %s", e.what());
				reply_data = {{"error", "Running Error"}, {"message", e.what()}};
			}
		}
		catch (const nlohmann::detail::parse_error &e)
		{
			LOG_F(ERROR, "[ASYNC MODE] WebSocketServer::readMessage() json exception: %s", e.what());
			reply_data = {{"errro", "Invalid Format"}, {"message", e.what()}};
		}
		catch (const std::exception &e)
		{
			LOG_F(ERROR, "[ASYNC MODE] WebSocketServer::readMessage() exception: %s", e.what());
			reply_data = {{"errro", "Unknown Error"}, {"message", e.what()}};
		}
		return socket->sendOneFrameTextAsync(reply_data.dump());
	}
	else if (size > 0)
	{
		m_messageBuffer.writeSimple(data, size);
	}
	return nullptr;
}

#else

void WebSocketServer::onPing(const WebSocket &socket, const oatpp::String &message)
{
	DLOG_F(INFO, "onPing");
	socket.sendPong(message);
}

void WebSocketServer::onPong(const WebSocket &socket, const oatpp::String &message)
{
	DLOG_F(INFO, "onPong");
}

void WebSocketServer::onClose(const WebSocket &socket, v_uint16 code, const oatpp::String &message)
{
	DLOG_F(INFO, "onClose code=%d", code);
}

void WebSocketServer::readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size)
{
	if (size == 0)
	{
		auto wholeMessage = m_messageBuffer.toString();
		m_messageBuffer.setCurrentPosition(0);
		DLOG_F(INFO, "onMessage message='%s'", wholeMessage->c_str());
		if (!nlohmann::json::accept(wholeMessage->c_str()))
		{
			LOG_F(ERROR, "Message is not in JSON format");
			return;
		}
		try
		{
			DLOG_F(INFO, "Start client command in alone thread");
			nlohmann::json jdata = nlohmann::json::parse(wholeMessage->c_str());
#if __cplusplus >= 202002L
			std::jthread myThread(std::bind(&WebSocketServer::ProcessMessage, this, std::ref(socket), std::ref(jdata)));
#else
			std::thread myThread(std::bind(&WebSocketServer::ProcessMessage, this, std::ref(socket), std::ref(jdata)));
#endif
			myThread.detach();
			DLOG_F(INFO, "Started command thread successfully");
		}
		catch (const nlohmann::detail::parse_error &e)
		{
			LOG_F(ERROR, "Failed to parser JSON message : %s", e.what());
		}
		catch (const std::exception &e)
		{
			LOG_F(ERROR, "Unknown error happened in WebsocketServer : %s", e.what());
		}
	}
	else if (size > 0)
	{ // message frame received
		m_messageBuffer.writeSimple(data, size);
	}
}

int WebSocketServer::add_connection(const oatpp::websocket::WebSocket *recv)
{
	auto it = find(recv);
	if (it == m_connections.end())
	{
		DLOG_F("WebSocketServer", "Registering %p", recv);
		m_connections.push_back(recv);
	}
	else
	{
		DLOG_F("WebSocketServer", "%p already registered", recv);
	}
	return 0;
}

int WebSocketServer::remove_connection(const oatpp::websocket::WebSocket *recv)
{
	auto it = find(recv);
	if (it != m_connections.end())
	{
		DLOG_F("WebSocketServer", "Unregistering %p", recv);
		m_connections.erase(it);
	}
	else
	{
		DLOG_F("WebSocketServer", "%p not registered", recv);
	}
	return 0;
}

std::vector<const oatpp::websocket::WebSocket *>::const_iterator WebSocketServer::find(const oatpp::websocket::WebSocket *recv)
{
	for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
	{
		if ((*it) == recv)
		{
			return it;
		}
	}
	return m_connections.end();
}

#endif

#if ENABLE_ASYNC == 0
void WebSocketServer::ProcessMessage(const WebSocket &socket, const nlohmann::json &data)
{
	try
	{
		if (data.empty())
		{
			LOG_F(ERROR, "WebSocketServer::processMessage() data is empty");
			return;
		}
		nlohmann::json reply_data;
		try
		{
			if (data.contains("name") && data.contains("params"))
			{
				const std::string name = data["name"].get<std::string>();
				if (m_CommandDispatcher->HasHandler(name))
				{
					json res = m_CommandDispatcher->Dispatch(name, data["params"].get<json>());
					if (res.contains("error"))
					{
						LOG_F(ERROR, "Failed to run command %s , error : %s", name.c_str(), res.dump().c_str());
						reply_data["error"] = res["error"];
					}
					else
					{
						DLOG_F(INFO, "Run command %s successfully", name.c_str());
						reply_data = {{"reply", "OK"}};
					}
				}
			}
			else
			{
				LOG_F(ERROR, "WebSocketServer::processMessage() missing parameter: name or params");
				reply_data = {{"error", "Missing parameter: name or params"}};
			}
		}
		catch (const nlohmann::json::exception &e)
		{
			LOG_F(ERROR, "WebSocketServer::processMessage() json exception: %s", e.what());
			reply_data = {{"error", e.what()}};
		}
		catch (const std::exception &e)
		{
			LOG_F(ERROR, "WebSocketServer::processMessage() exception: %s", e.what());
			reply_data = {{"error", e.what()}};
		}
		socket.sendOneFrameText(reply_data.dump());
	}
	catch (const std::exception &e)
	{
		LOG_F(ERROR, "WebSocketServer::onMessage() parse json failed: %s", e.what());
	}
}
#endif

std::atomic<v_int32> WSInstanceListener::SOCKETS(0);

#if ENABLE_ASYNC
void WSInstanceListener::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params)
{
	SOCKETS++;
	DLOG_F(INFO, "New Incoming Connection. Connection count=%d", SOCKETS.load());
	if (!m_socket)
	{
		m_socket = std::make_shared<WebSocketServer>(socket);
	}
	socket->setListener(m_socket);
}

void WSInstanceListener::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket)
{
	SOCKETS--;
	DLOG_F(INFO, "Connection closed. Connection count=%d", SOCKETS.load());
}
#else
void WSInstanceListener::onAfterCreate(const oatpp::websocket::WebSocket &socket, const std::shared_ptr<const ParameterMap> &params)
{
	SOCKETS++;
	DLOG_F(INFO, "New Incoming Connection. Connection count=%d", SOCKETS.load());
	if (!m_socket)
	{
		m_socket = std::make_shared<WebSocketServer>();
	}
	socket->setListener(m_socket);
	m_sockets->add_connection(&socket);
}

void WSInstanceListener::onBeforeDestroy(const oatpp::websocket::WebSocket &socket)
{
	SOCKETS--;
	DLOG_F(INFO, "Connection closed. Connection count=%d", SOCKETS.load());
	m_sockets->remove_connection(&socket);
}
#endif