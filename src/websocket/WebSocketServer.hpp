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

#include "LithiumApp.hpp"

#include "modules/server/commander.hpp"
#include "core/property/iproperty.hpp"
#include "modules/error/error_code.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

#if ENABLE_ASYNC
class WebSocketServer : public oatpp::websocket::AsyncWebSocket::Listener
#else
class WebSocketServer : public oatpp::websocket::WebSocket::Listener
#endif
{
public:
	WebSocketServer(const std::shared_ptr<AsyncWebSocket> &socket);
	~WebSocketServer();

public:
	void GetDeviceList(const json &m_params);
	void AddDevice(const json &m_params);
	void AddDeviceLibrary(const json &m_params);
	void RemoveDevice(const json &m_params);
	void RemoveDevicesByName(const json &m_params);
	void RemoveDeviceLibrary(const json &m_params);
	void RunDeviceTask(const json &m_params);
	void GetDeviceInfo(const json &m_params);

public:
	void CreateProcessLi(const json &m_params);
	void RunScript(const json &m_params);
	void TerminateProcessByName(const json &m_params);
	void GetRunningProcesses(const json &m_params);
	void GetProcessOutput(const json &m_params);

public:
	void AddTask(const json &m_params);
	void InsertTask(const json &m_params);
	void ExecuteAllTasks(const json &m_params);
	void StopTask(const json &m_params);
	void ExecuteTaskByName(const json &m_params);
	void ModifyTask(const json &m_params);
	void ModifyTaskByName(const json &m_params);
	void DeleteTask(const json &m_params);
	void DeleteTaskByName(const json &m_params);
	void QueryTaskByName(const json &m_params);
	void GetTaskList(const json &m_params);
	void SaveTasksToJson(const json &m_params);

public:
	void runChaiCommand(const json &m_params);
	void runChaiMultiCommand(const json &m_params);
	void runChaiScript(const json &m_params);
	void loadChaiFile(const json &m_params);

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
	static constexpr const char *TAG = "WebSocketServer";

#if ENABLE_ASYNC == 0
	void ProcessMessage(const WebSocket &socket, const json &data);
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
	void LiRegisterFunc(const std::string &name, void (ClassType::*handler)(const json &))
	{
		m_CommandDispatcher->RegisterHandler(name, handler, this);
	}

	bool APTRunFunc(const std::string &name, const json &params)
	{
		if (m_CommandDispatcher->HasHandler(name))
		{
			m_CommandDispatcher->Dispatch(name, params);
			return true;
		}
		return false;
	}
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

const json serror(const std::string func_name, ServerError code, const std::string errorMsg);

#endif // WebSocketServer_hpp
