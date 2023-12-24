/*
 * WsServer.hpp
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

Date: 2023-12-15

Description: WebSocket Server

**************************************************/

#ifndef WSSERVER_HPP
#define WSSERVER_HPP

#include "WsHub.hpp"

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

#include "atom/server/serialize.hpp"
#include "atom/server/deserialize.hpp"
#include "atom/server/message.hpp"

class MessageBus;

class WsServer : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener
{
public:
	std::atomic<v_int32> m_ConnectionCounter;
#ifdef ENABLE_FASTHASH
	emhash8::HashMap<oatpp::String, std::shared_ptr<WsHub>> m_hubs;
#else
	std::unordered_map<oatpp::String, std::shared_ptr<WsHub>> m_hubs;
#endif
	std::mutex m_hubsMutex;

public:
	WsServer();

public:
	/**
	 *  Called when socket is created
	 */
	void onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params) override;

	/**
	 *  Called before socket instance is destroyed.
	 */
	void onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket) override;

	/**
	 * Generate id for new connection.
	 * @return
	 */
	v_int32 obtainNewConnectionId();

	/**
	 * Get plugin hub by name or create new one if not exists.
	 * @param hubName
	 * @return
	 */
	std::shared_ptr<WsHub> getOrCreateHub(const oatpp::String &hubName);

private:
	// Serialization and Deserialization Engine
	std::shared_ptr<Atom::Server::SerializationEngine> m_SerializationEngine;
	std::shared_ptr<Atom::Server::DeserializationEngine> m_DeserializationEngine;
	// Message Bus
	std::shared_ptr<MessageBus> m_MessageBus;
};

#endif // WSSERVER_HPP
