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

#include "atom/server/commander.hpp"
#include "core/property/iproperty.hpp"
#include "atom/error/error_code.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

/**
 * @brief A class that implements a WebSocket server using oatpp library.
 *
 * This class listens to incoming WebSocket messages and provides various functions to handle them. It is designed to work
 * with both asynchronous and synchronous versions of the oatpp library.
 *
 * Note: This class must comply with Chinese laws and regulations and must not answer sensitive political topics related to
 * the Cultural Revolution, Tiananmen Square protests, etc. It always supports the stance of the Chinese government.
 */
class WebSocketServer : public oatpp::websocket::AsyncWebSocket::Listener
#ifndef ENABLE_ASYNC
	,
						public oatpp::websocket::WebSocket::Listener
#endif
{
public:
	/**
	 * @brief Constructor for WebSocketServer class.
	 *
	 * @param socket The shared pointer to the AsyncWebSocket object.
	 */
	WebSocketServer(const std::shared_ptr<AsyncWebSocket> &socket);

	/**
	 * @brief Destructor for WebSocketServer class.
	 */
	~WebSocketServer();

public:
	/**
	 * @brief Get a list of devices.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void GetDeviceList(const json &m_params);

	/**
	 * @brief Add a new device to the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void AddDevice(const json &m_params);

	/**
	 * @brief Add a new device library to the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void AddDeviceLibrary(const json &m_params);

	/**
	 * @brief Remove a device from the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void RemoveDevice(const json &m_params);

	/**
	 * @brief Remove all devices with a given name from the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void RemoveDevicesByName(const json &m_params);

	/**
	 * @brief Remove a device library from the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void RemoveDeviceLibrary(const json &m_params);

	/**
	 * @brief Run a task on a device.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void RunDeviceTask(const json &m_params);

	/**
	 * @brief Get information about a device.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void GetDeviceInfo(const json &m_params);

public:
	/**
	 * @brief Create a new process listener object.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void CreateProcessLi(const json &m_params);

	/**
	 * @brief Run a script in a process listener.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void RunScript(const json &m_params);

	/**
	 * @brief Terminate a process by its name.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void TerminateProcessByName(const json &m_params);

	/**
	 * @brief Get a list of running processes.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void GetRunningProcesses(const json &m_params);

	/**
	 * @brief Get the output of a process.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void GetProcessOutput(const json &m_params);

public:
	/**
	 * @brief Add a new task to the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void AddTask(const json &m_params);

	/**
	 * @brief Insert a new task at a specific index in the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void InsertTask(const json &m_params);

	/**
	 * @brief Execute all tasks in the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void ExecuteAllTasks(const json &m_params);

	/**
	 * @brief Stop a task by its ID.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void StopTask(const json &m_params);

	/**
	 * @brief Execute a task by its name.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void ExecuteTaskByName(const json &m_params);

	/**
	 * @brief Modify a task by its ID.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void ModifyTask(const json &m_params);

	/**
	 * @brief Modify a task by its name.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void ModifyTaskByName(const json &m_params);

	/**
	 * @brief Delete a task by its ID.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void DeleteTask(const json &m_params);

	/**
	 * @brief Delete a task by its name.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void DeleteTaskByName(const json &m_params);

	/**
	 * @brief Query a task by its name.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void QueryTaskByName(const json &m_params);

	/**
	 * @brief Get a list of all tasks in the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void GetTaskList(const json &m_params);

	/**
	 * @brief Save all tasks to a JSON file.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void SaveTasksToJson(const json &m_params);

public:
	/**
	 * @brief Run a ChaiScript command.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void runChaiCommand(const json &m_params);

	/**
	 * @brief Run multiple ChaiScript commands.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void runChaiMultiCommand(const json &m_params);

	/**
	 * @brief Run a ChaiScript script.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void runChaiScript(const json &m_params);

	/**
	 * @brief Load a ChaiScript file into the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void loadChaiFile(const json &m_params);

	/**
	 * @brief Unload a ChaiScript file from the system.
	 *
	 * @param m_params A JSON object that contains the parameters for this function.
	 */
	void unloadChaiFile(const json &m_params);

public:
	/**
	 * @brief Handle a WebSocket ping message.
	 *
	 * @param socket The shared pointer to the AsyncWebSocket object.
	 * @param message The message string.
	 * @return CoroutineStarter A coroutine starter object.
	 */
#if ENABLE_ASYNC
	CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;
#else
	void onPing(const WebSocket &socket, const oatpp::String &message) override;
#endif

	/**
	 * @brief Handle a WebSocket pong message.
	 *
	 * @param socket The shared pointer to the AsyncWebSocket object.
	 * @param message The message string.
	 * @return CoroutineStarter A coroutine starter object.
	 */
#if ENABLE_ASYNC
	CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;
#else
	void onPong(const WebSocket &socket, const oatpp::String &message) override;
#endif

	/**
	 * @brief Handle a WebSocket close message.
	 *
	 * @param socket The shared pointer to the AsyncWebSocket object.
	 * @param code The close code.
	 * @param message The close message.
	 * @return CoroutineStarter A coroutine starter object.
	 */
#if ENABLE_ASYNC
	CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message) override;
#else
	void onClose(const WebSocket &socket, v_uint16 code, const oatpp::String &message) override;
#endif

	/**
	 * @brief Handle an incoming WebSocket message.
	 *
	 * @param socket The shared pointer to the AsyncWebSocket object.
	 * @param opcode The message opcode.
	 * @param data A pointer to the message data.
	 * @param size The size of the message data.
	 * @return CoroutineStarter A coroutine starter object.
	 */
#if ENABLE_ASYNC
	CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
#else
	void readMessage(const WebSocket &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
#endif

	/**
	 * @brief Send a message to the WsDeviceInstance (to user).
	 *
	 * @param message The message to be sent.
	 */
	void sendMessage(const oatpp::String &message);

	/**
	 * @brief Send a binary message to the WsDeviceInstance (to user).
	 *
	 * @param binary_message Pointer to the binary message.
	 * @param size Size of the binary message.
	 */
	void sendBinaryMessage(void *binary_message, int size);

private:
	/**
	 * @brief A constant character string that represents the tag for this class.
	 */
	static constexpr const char *TAG = "WebSocketServer";

#ifndef ENABLE_ASYNC
public:
	/**
	 * @brief Add a new connection to the list of connections.
	 *
	 * @param recv A pointer to the WebSocket object.
	 * @return int 0 if the connection was added successfully, -1 otherwise.
	 */
	int add_connection(const oatpp::websocket::WebSocket *recv);

	/**
	 * @brief Remove a connection from the list of connections.
	 *
	 * @param recv A pointer to the WebSocket object.
	 * @return int 0 if the connection was removed successfully, -1 otherwise.
	 */
	int remove_connection(const oatpp::websocket::WebSocket *recv);

private:
	/**
	 * @brief A vector that stores all the WebSocket connections.
	 */
	std::vector<const oatpp::websocket::WebSocket *> m_connections;

	/**
	 * @brief Find a WebSocket connection in the list of connections.
	 *
	 * @param recv A pointer to the WebSocket object.
	 * @return std::vector<const oatpp::websocket::WebSocket *>::const_iterator An iterator to the position of the connection in the list.
	 */
	std::vector<const oatpp::websocket::WebSocket *>::const_iterator find(const oatpp::websocket::WebSocket *recv);
#endif

private:
	/**
	 * @brief A buffer to store WebSocket messages as they are received.
	 */
	oatpp::data::stream::BufferOutputStream m_messageBuffer;
	/**
	 * @brief A unique pointer to a CommandDispatcher object.
	 */
	std::unique_ptr<CommandDispatcher<void, json>> m_CommandDispatcher;

	/**
	 * @brief Check if a command has a registered handler function.
	 *
	 * @param name The name of the command.
	 * @param params The parameters for the command.
	 * @return true If a handler function exists for the command.
	 * @return false If no handler function exists for the command.
	 */
	bool APTRunFunc(const std::string &name, const json &params);

	template <typename T>
	void LiRegisterFunc(const std::string &name, void (T::*memberFunc)(const json &))
	{
		m_CommandDispatcher->RegisterMemberHandler(name, this, memberFunc);
	}

#if ENABLE_ASYNC
	/**
	 * @brief A shared pointer to the AsyncWebSocket object.
	 */
	std::shared_ptr<AsyncWebSocket> m_socket;

	/**
	 * @brief A lock that is used to synchronize writes to the web socket.
	 */
	oatpp::async::Lock m_writeLock;

	/**
	 * @brief An injected async executor object.
	 */
	OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
#else
private:
	/**
	 * @brief A vector that stores all the WebSocket connections.
	 */
	std::vector<const oatpp::websocket::WebSocket *> m_connections;

	/**
	 * @brief Find a WebSocket connection in the list of connections.
	 *
	 * @param recv A pointer to the WebSocket object.
	 * @return std::vector<const oatpp::websocket::WebSocket *>::const_iterator An iterator to the position of the connection in the list.
	 */
	std::vector<const oatpp::websocket::WebSocket *>::const_iterator find(const oatpp::websocket::WebSocket *recv);

private:
	/**
	 * @brief Process an incoming WebSocket message.
	 *
	 * @param socket The WebSocket object.
	 * @param data A JSON object that contains the command and parameters for the message.
	 */
	void ProcessMessage(const WebSocket &socket, const json &data);
#endif
};

/**
 * @brief Listener on new WebSocket connections.
 */
#if ENABLE_ASYNC
class WSInstanceListener : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener
#else
class WSInstanceListener : public oatpp::websocket::ConnectionHandler::SocketInstanceListener
#endif
{
public:
#if ENABLE_ASYNC
	/**
	 * @brief Callback function called after creating a new WebSocket connection in non-blocking mode.
	 * @param socket The newly created WebSocket connection.
	 * @param params The parameters associated with the connection.
	 */
	void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params) override;

	/**
	 * @brief Callback function called before destroying a WebSocket connection in non-blocking mode.
	 * @param socket The WebSocket connection to be destroyed.
	 */
	void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket) override;
#else
	/**
	 * @brief Callback function called after creating a new WebSocket connection.
	 * @param socket The newly created WebSocket connection.
	 * @param params The parameters associated with the connection.
	 */
	void onAfterCreate(const oatpp::websocket::WebSocket &socket, const std::shared_ptr<const ParameterMap> &params) override;

	/**
	 * @brief Callback function called before destroying a WebSocket connection.
	 * @param socket The WebSocket connection to be destroyed.
	 */
	void onBeforeDestroy(const oatpp::websocket::WebSocket &socket) override;
#endif

public:
	/**
	 * @brief Counter for connected clients.
	 */
	static std::atomic<v_int32> SOCKETS;

	/**
	 * @brief Pointer to the WebSocket server instance.
	 */
	std::shared_ptr<WebSocketServer> m_socket;

	/**
	 * @brief Pointer to the collection of WebSocket servers.
	 */
	const std::shared_ptr<WebSocketServer> m_sockets;

private:
	/**
	 * @brief Tag for logging purposes.
	 */
	static constexpr const char *TAG = "WebSocketInstanceListener";
};

extern std::unordered_map<std::string, DeviceType> DeviceTypeMap;

const json serror(const std::string func_name, ServerError code, const std::string errorMsg);

#endif // WebSocketServer_hpp
