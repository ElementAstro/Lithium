/*
 * WsDeviceServer.hpp
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

Description: WebSocket Device Server

**************************************************/

#ifndef WSDEVICESERVER_HPP
#define WSDEVICESERVER_HPP

#include "WsDeviceHub.hpp"

#include "config.h"

#include <mutex>

#if ENABLE_ASYNC
#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#else
#include "oatpp-websocket/ConnectionHandler.hpp"
#endif

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/utils/switch.hpp"

#if ENABLE_ASYNC
class WsDeviceServer : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener
#else
class WsDeviceServer : public oatpp::websocket::ConnectionHandler::SocketInstanceListener
#endif
{
public:
	WsDeviceServer();

	/**
	 * Generate id for new user
	 * @return
	 */
	v_int32 obtainNewUserId();

	/**
	 * Get device hub by name or create new one if not exists.
	 * @param hubName
	 * @return
	 */
	std::shared_ptr<WsDeviceHub> getOrCreateHub(const oatpp::String &hubName);

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
	std::atomic<v_int32> m_userIdCounter;
	std::unordered_map<oatpp::String, std::shared_ptr<WsDeviceHub>> m_hubs;
	std::mutex m_hubsMutex;
	std::unique_ptr<StringSwitch<const std::shared_ptr<AsyncWebSocket> &, const oatpp::String &, const oatpp::String &>> m_device_switch;
};

#endif // WSDEVICESERVER_HPP
