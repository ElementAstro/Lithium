/*
 * AsyncWsServer.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-15

Description: WebSocket Server

**************************************************/

#ifndef ASYNC_WS_SERVER_HPP
#define ASYNC_WS_SERVER_HPP

#include "AsyncWsHub.hpp"

#include "oatpp-websocket/AsyncConnectionHandler.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <mutex>

#include "atom/server/serialize.hpp"
#include "atom/server/deserialize.hpp"
#include "atom/type/message.hpp"

class MessageBus;

class AsyncWsServer : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener
{
public:
	std::atomic<v_int32> m_ConnectionCounter;
#if ENABLE_FASTHASH
	emhash8::HashMap<oatpp::String, std::shared_ptr<WsHub>> m_hubs;
#else
	std::unordered_map<oatpp::String, std::shared_ptr<WsHub>> m_hubs;
#endif
	std::mutex m_hubsMutex;

public:
	AsyncWsServer();

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

#endif // ASYNC_WS_SERVER_HPP
