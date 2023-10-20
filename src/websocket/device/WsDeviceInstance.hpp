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

class WsDeviceInstance : public oatpp::websocket::AsyncWebSocket::Listener
{

public:
	WsDeviceInstance(const std::shared_ptr<AsyncWebSocket> &socket,
					 const std::shared_ptr<WsDeviceHub> &hub,
					 const oatpp::String &device_name,
					 v_int32 userId);

	~WsDeviceInstance();

	/**
	 * Send message to WsDeviceInstance (to user).
	 * @param message
	 */
	void sendMessage(const oatpp::String &message);

	void sendBinaryMessage(void *binary_message, int size);

	/**
	 * Get hub of the WsDeviceInstance.
	 * @return
	 */
	std::shared_ptr<WsDeviceHub> getHub();

	/**
	 * Get WsDeviceInstance device_name.
	 * @return
	 */
	oatpp::String getDeviceName();

	/**
	 * Get WsDeviceInstance userId.
	 * @return
	 */
	v_int32 getUserId();

public:
	void setProperty(const json &m_params);
	void getProperty(const json &m_params);
	void runTask(const json &m_params);
	void runFunc(const json &m_params);

public: // WebSocket Listener methods
	CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;
	CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;
	CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message) override;
	CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;

private:
	/**
	 * Buffer for messages. Needed for multi-frame messages.
	 */
	oatpp::data::stream::BufferOutputStream m_messageBuffer;

	/**
	 * Lock for synchronization of writes to the web socket.
	 */
	oatpp::async::Lock m_writeLock;

	std::unique_ptr<VCommandDispatcher> m_CommandDispatcher;

	template <typename ClassType>
	void LiRegisterFunc(const std::string &name, void (ClassType::*handler)(const json &))
	{
		m_CommandDispatcher->RegisterHandler(name, handler, this);
	}

	bool LiRunFunc(const std::string &name, const json &params)
	{
		if (m_CommandDispatcher->HasHandler(name))
		{
			m_CommandDispatcher->Dispatch(name, params);
			return true;
		}
		return false;
	}

private:
	std::shared_ptr<AsyncWebSocket> m_socket;
	std::shared_ptr<WsDeviceHub> m_hub;
	oatpp::String m_device_name;
	v_int32 m_userId;

private:
	/**
	 * Inject async executor object.
	 */
	OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);

};

#endif // WSDEVICEINSTANCE_HPP
