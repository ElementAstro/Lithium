/*
 * AsyncWsHub.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-15

Description: WebSocket Connections hub.

**************************************************/

#ifndef ASYNC_WS_HUB_HPP
#define ASYNC_WS_HUB_HPP

#include "AsyncWsInstance.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/server/serialize.hpp"
#include "atom/server/deserialize.hpp"
#include "atom/server/message_bus.hpp"

class AsyncWsHub
{
public:
	AsyncWsHub(const oatpp::String &name);

	/**
	 * Add Connection to the AsyncWsHub.
	 * @param Connection
	 */
	void addConnection(const std::shared_ptr<AsyncWsInstance> &Connection);

	/**
	 * Remove Connection from the AsyncWsHub.
	 * @param userId
	 */
	void removeConnectionByUserId(v_int32 userId);

	/**
	 * Send message to all Connections in the AsyncWsHub.
	 * @param message
	 */
	void sendMessage(const oatpp::String &message);

	/**
	 * Send binary message to all Connections in the AsyncWsHub.
	 * @param binary_message
	 * @param size
	 */
	void sendBinaryMessage(const void *binary_message, int size);

private:
	oatpp::String m_name;
	std::unordered_map<v_int32, std::shared_ptr<AsyncWsInstance>> m_ConnectionById;
	std::mutex m_ConnectionByIdLock;
	// Serialization and Deserialization Engine
	std::shared_ptr<Atom::Server::SerializationEngine> m_SerializationEngine;
	std::shared_ptr<Atom::Server::DeserializationEngine> m_DeserializationEngine;
	// Message Bus
	std::shared_ptr<Atom::Server::MessageBus> m_MessageBus;
};

#endif // WsHUB_HPP
