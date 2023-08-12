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

#include "LithiumApp.hpp"

#include "modules/server/commander.hpp"
#include "liproperty/iproperty.hpp"

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
	/**
	 * Inject async executor object.
	 */
	OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
#else
public:
	int add_connection(const oatpp::websocket::WebSocket *recv);
	int remove_connection(const oatpp::websocket::WebSocket *recv);
private:
	std::vector<const oatpp::websocket::WebSocket *> m_connections;
	std::vector<const oatpp::websocket::WebSocket *>::const_iterator find(const oatpp::websocket::WebSocket *recv);
#endif

private:
	std::unique_ptr<CommandDispatcher> m_CommandDispatcher;

	template <typename ClassType>
	void LiRegisterFunc(const std::string &name, const nlohmann::json (ClassType::*handler)(const nlohmann::json &))
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
	WebSocketServer(const std::shared_ptr<AsyncWebSocket>& socket);
	~WebSocketServer();

public:
	const nlohmann::json GetDeviceList(const nlohmann::json &m_params);
	const nlohmann::json AddDevice(const nlohmann::json &m_params);
	const nlohmann::json AddDeviceLibrary(const nlohmann::json &m_params);
	const nlohmann::json RemoveDevice(const nlohmann::json &m_params);
	const nlohmann::json RemoveDevicesByName(const nlohmann::json &m_params);
	const nlohmann::json RemoveDeviceLibrary(const nlohmann::json &m_params);
	const nlohmann::json RunDeviceTask(const nlohmann::json &m_params);
	const nlohmann::json GetDeviceInfo(const nlohmann::json &m_params);

public:
	const nlohmann::json CreateProcessLi(const nlohmann::json &m_params);
	const nlohmann::json RunScript(const nlohmann::json &m_params);
	const nlohmann::json TerminateProcessByName(const nlohmann::json &m_params);
	const nlohmann::json GetRunningProcesses(const nlohmann::json &m_params);
	const nlohmann::json GetProcessOutput(const nlohmann::json &m_params);

public:
	const nlohmann::json AddTask(const nlohmann::json &m_params);
	const nlohmann::json InsertTask(const nlohmann::json &m_params);
	const nlohmann::json ExecuteAllTasks(const nlohmann::json &m_params);
	const nlohmann::json StopTask(const nlohmann::json &m_params);
	const nlohmann::json ExecuteTaskByName(const nlohmann::json &m_params);
	const nlohmann::json ModifyTask(const nlohmann::json &m_params);
	const nlohmann::json ModifyTaskByName(const nlohmann::json &m_params);
	const nlohmann::json DeleteTask(const nlohmann::json &m_params);
	const nlohmann::json DeleteTaskByName(const nlohmann::json &m_params);
	const nlohmann::json QueryTaskByName(const nlohmann::json &m_params);
	const nlohmann::json GetTaskList(const nlohmann::json &m_params);
	const nlohmann::json SaveTasksToJson(const nlohmann::json &m_params);

public:

	const nlohmann::json runChaiCommand(const nlohmann::json &m_params);
	const nlohmann::json runChaiMultiCommand(const nlohmann::json &m_params);
	const nlohmann::json runChaiScript(const nlohmann::json &m_params);
	const nlohmann::json loadChaiFile(const nlohmann::json &m_params);

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

	std::shared_ptr<WebSocketServer> m_socket;

	const std::shared_ptr<WebSocketServer> m_sockets;

public:
#if ENABLE_ASYNC
	void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params) override;

	void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket) override;
#else
	void onAfterCreate(const oatpp::websocket::WebSocket &socket, const std::shared_ptr<const ParameterMap> &params) override;

	void onBeforeDestroy(const oatpp::websocket::WebSocket &socket) override;
#endif
};

extern std::unordered_map<std::string, Lithium::DeviceType> DeviceTypeMap;

#endif // WebSocketServer_hpp
