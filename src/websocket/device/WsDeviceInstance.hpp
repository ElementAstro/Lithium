/*
 * WsDeviceInstance.hpp
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

Date: 2023-10-20

Description: WebSocket Device Instance (each device each instance)

**************************************************/

#ifndef WSDEVICEINSTANCE_HPP
#define WSDEVICEINSTANCE_HPP

#include "config.h"

#if ENABLE_ASYNC
#include "oatpp-websocket/AsyncWebSocket.hpp"
#else
#include "oatpp-websocket/WebSocket.hpp"
#endif

#if ENABLE_ASYNC
#include "oatpp/core/async/Lock.hpp"
#include "oatpp/core/async/Executor.hpp"
#endif

#include "oatpp/core/macro/component.hpp"

#include "modules/server/commander.hpp"

#include "LithiumApp.hpp"

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class WsDeviceHub; // FWD

/**
 * @brief Class representing an instance of a WebSocket device.
 *
 */
class WsDeviceInstance : public oatpp::websocket::AsyncWebSocket::Listener
{

public:
	/**
	 * @brief Construct a new WsDeviceInstance object.
	 *
	 * @param socket Shared pointer to the AsyncWebSocket object.
	 * @param hub Shared pointer to the WsDeviceHub object.
	 * @param device_name Name of the device.
	 * @param userId Id of the user.
	 */
	WsDeviceInstance(const std::shared_ptr<AsyncWebSocket> &socket,
					 const std::shared_ptr<WsDeviceHub> &hub,
					 const oatpp::String &device_name,
					 v_int32 userId);

	/**
	 * @brief Destroy the WsDeviceInstance object.
	 *
	 */
	~WsDeviceInstance();

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

	/**
	 * @brief Get the hub of the WsDeviceInstance.
	 *
	 * @return Shared pointer to the WsDeviceHub object.
	 */
	std::shared_ptr<WsDeviceHub> getHub();

	/**
	 * @brief Get the name of the WsDeviceInstance device.
	 *
	 * @return The name of the device.
	 */
	oatpp::String getDeviceName();

	/**
	 * @brief Get the id of the WsDeviceInstance user.
	 *
	 * @return The id of the user.
	 */
	v_int32 getUserId();

public:
	/**
	 * @brief Set a property of the WsDeviceInstance.
	 *
	 * @param m_params JSON object containing the property to be set.
	 */
	void setProperty(const json &m_params);

	/**
	 * @brief Get a property of the WsDeviceInstance.
	 *
	 * @param m_params JSON object containing the property to be retrieved.
	 */
	void getProperty(const json &m_params);

	/**
	 * @brief Get all properties of the WsDeviceInstance.
	 *
	 * @param m_params JSON object containing the request.
	 */
	void getProperties(const json &m_params);

	/**
	 * @brief Run a task on the WsDeviceInstance.
	 *
	 * @param m_params JSON object containing the task to be run.
	 */
	void runTask(const json &m_params);

	/**
	 * @brief Run a function on the WsDeviceInstance.
	 *
	 * @param m_params JSON object containing the function to be run.
	 */
	void runFunc(const json &m_params);

public: // WebSocket Listener methods
	/**
	 * @brief Handle a ping message received by the WebSocket.
	 *
	 * @param socket Shared pointer to the AsyncWebSocket object.
	 * @param message The ping message received.
	 * @return CoroutineStarter object.
	 */
	CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;

	/**
	 * @brief Handle a pong message received by the WebSocket.
	 *
	 * @param socket Shared pointer to the AsyncWebSocket object.
	 * @param message The pong message received.
	 * @return CoroutineStarter object.
	 */
	CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;

	/**
	 * @brief Handle the WebSocket being closed.
	 *
	 * @param socket Shared pointer to the AsyncWebSocket object.
	 * @param code The close code.
	 * @param message The close message.
	 * @return CoroutineStarter object.
	 */
	CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message) override;

	/**
	 * @brief Handle a message received by the WebSocket.
	 *
	 * @param socket Shared pointer to the AsyncWebSocket object.
	 * @param opcode The opcode of the message.
	 * @param data Pointer to the message data.
	 * @param size The size of the message data.
	 * @return CoroutineStarter object.
	 */
	CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

private:
	/**
	 * @brief Buffer for messages. Needed for multi-frame messages.
	 *
	 */
	oatpp::data::stream::BufferOutputStream m_messageBuffer;

	/**
	 * @brief Lock for synchronization of writes to the WebSocket.
	 *
	 */
	oatpp::async::Lock m_writeLock;

	std::unique_ptr<CommandDispatcher<void,json>> m_CommandDispatcher;

	/**
	 * @brief Register a function handler for the VCommandDispatcher.
	 *
	 * @tparam ClassType The class type of the handler.
	 * @param name The name of the function.
	 * @param handler The function handler.
	 */
	template <typename ClassType>
	void LiRegisterFunc(const std::string &name, void (ClassType::*handler)(const json &))
	{
		m_CommandDispatcher->RegisterHandler(name, handler, this);
	}

	/**
	 * @brief Run a function on the VCommandDispatcher.
	 *
	 * @param name The name of the function to be run.
	 * @param params JSON object containing the parameters for the function.
	 * @return True if the function was run successfully, false otherwise.
	 */
	bool LiRunFunc(const std::string &name, const json &params);

private:
	std::shared_ptr<AsyncWebSocket> m_socket;
	std::shared_ptr<WsDeviceHub> m_hub;
	oatpp::String m_device_name;
	v_int32 m_userId;

private:
	/**
	 * @brief Inject async executor object.
	 *
	 */
	OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
};

#endif // WSDEVICEINSTANCE_HPP
