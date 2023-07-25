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

#include "config.h"

#if ENABLE_ASYNC
#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include "oatpp-websocket/AsyncWebSocket.hpp"
#include "oatpp/core/async/Lock.hpp"
#include "oatpp/core/async/Executor.hpp"
#else
#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp-websocket/WebSocket.hpp"
#endif
#include "oatpp/core/macro/component.hpp"

#include "nlohmann/json.hpp"

#include "modules/server/commander.hpp"
#include "modules/property/imessage.hpp"

#if ENABLE_ASYNC
class WebSocketServer : public oatpp::websocket::AsyncWebSocket::Listener
#else
class WebSocketServer : public oatpp::websocket::WebSocket::Listener
#endif
{
private:
	static constexpr const char *TAG = "WebSocketServer";

#if ENABLE_ASYNC == 0
	void ProcessMessage(const WebSocket &socket, const nlohmann::json &data);
#endif

private:
	oatpp::data::stream::BufferOutputStream m_messageBuffer;

#if ENABLE_ASYNC
	std::shared_ptr<AsyncWebSocket> m_socket;
	/**
	 * Lock for synchronization of writes to the web socket.
	 */
	oatpp::async::Lock m_writeLock;

#endif

private:
	/**
	 * Inject async executor object.
	 */
	OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);

	std::unique_ptr<CommandDispatcher> m_CommandDispatcher;

	template <typename ClassType>
	void LiRegisterFunc(const std::string &name, nlohmann::json (ClassType::*handler)(const nlohmann::json &))
	{
		m_CommandDispatcher->RegisterHandler(name, handler, this);
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

	void SendMessageNonBlocking(const oatpp::String &message);

public:
	nlohmann::json GetDeviceList(const nlohmann::json &m_params);
	nlohmann::json AddDevice(const nlohmann::json &m_params);
	nlohmann::json AddDeviceLibrary(const nlohmann::json &m_params);
	nlohmann::json RemoveDevice(const nlohmann::json &m_params);
	nlohmann::json RemoveDevicesByName(const nlohmann::json &m_params);
	nlohmann::json RemoveDeviceLibrary(const nlohmann::json &m_params);
	nlohmann::json RunDeviceTask(const nlohmann::json &m_params);
	nlohmann::json GetDeviceInfo(const nlohmann::json &m_params);

public:
	nlohmann::json CreateProcessLi(const nlohmann::json &m_params);
	nlohmann::json RunScript(const nlohmann::json &m_params);
	nlohmann::json TerminateProcessByName(const nlohmann::json &m_params);
	nlohmann::json GetRunningProcesses(const nlohmann::json &m_params);
	nlohmann::json GetProcessOutput(const nlohmann::json &m_params);

public:
	nlohmann::json AddTask(const nlohmann::json &m_params);
	nlohmann::json InsertTask(const nlohmann::json &m_params);
	nlohmann::json ExecuteAllTasks(const nlohmann::json &m_params);
	nlohmann::json StopTask(const nlohmann::json &m_params);
	nlohmann::json ExecuteTaskByName(const nlohmann::json &m_params);
	nlohmann::json ModifyTask(const nlohmann::json &m_params);
	nlohmann::json ModifyTaskByName(const nlohmann::json &m_params);
	nlohmann::json DeleteTask(const nlohmann::json &m_params);
	nlohmann::json DeleteTaskByName(const nlohmann::json &m_params);
	nlohmann::json QueryTaskByName(const nlohmann::json &m_params);
	nlohmann::json GetTaskList(const nlohmann::json &m_params);
	nlohmann::json SaveTasksToJson(const nlohmann::json &m_params);

public:
#if ENABLE_ASYNC
	CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;

	CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;

	CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message) override;

	CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
#else
	void onPing(const WebSocket &socket, const oatpp::String &message) override;

	void onPong(const WebSocket &socket, const oatpp::String &message) override;

	void onClose(const WebSocket &socket, v_uint16 code, const oatpp::String &message) override;

	void readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
#endif
private:
	void OnMessageReceived(const Lithium::IMessage &message);
};

/**
 * Listener on new WebSocket connections.
 */
#if ENABLE_ASYNC
class WSInstanceListener : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener
#else
class WSInstanceListener : public oatpp::websocket::ConnectionHandler::SocketInstanceListener
#endif
{
private:
	static constexpr const char *TAG = "WebSocketInstanceListener";

public:
	/**
	 * Counter for connected clients.
	 */
	static std::atomic<v_int32> SOCKETS;

public:
#if ENABLE_ASYNC
	void onAfterCreate_NonBlocking(const std::shared_ptr<WebSocketServer::AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params) override;

	void onBeforeDestroy_NonBlocking(const std::shared_ptr<WebSocketServer::AsyncWebSocket> &socket) override;
#else
	void onAfterCreate(const oatpp::websocket::WebSocket &socket, const std::shared_ptr<const ParameterMap> &params) override;

	void onBeforeDestroy(const oatpp::websocket::WebSocket &socket) override;
#endif
};

#endif // WebSocketServer_hpp