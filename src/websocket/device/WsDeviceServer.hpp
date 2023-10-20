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
#include <mutex>

class WsDeviceServer : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener
{
public:
	std::atomic<v_int32> m_userIdCounter;
	std::unordered_map<oatpp::String, std::shared_ptr<WsDeviceHub>> m_hubs;
	std::mutex m_hubsMutex;

public:
	WsDeviceServer()
		: m_userIdCounter(0)
	{
	}

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
	/**
	 *  Called when socket is created
	 */
	void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params) override;

	/**
	 *  Called before socket instance is destroyed.
	 */
	void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket) override;
};

#endif // WSDEVICESERVER_HPP
