
#ifndef WSHUB_HPP
#define WSHUB_HPP

#include "WsInstance.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

class MessageBus;

class WsHub
{
public:
	WsHub(const oatpp::String &name);

	/**
	 * Add Connection to the WsHub.
	 * @param Connection
	 */
	void addConnection(const std::shared_ptr<WsInstance> &Connection);

	/**
	 * Remove Connection from the WsHub.
	 * @param userId
	 */
	void removeConnectionByUserId(v_int32 userId);

	/**
	 * Send message to all Connections in the WsHub.
	 * @param message
	 */
	void sendMessage(const oatpp::String &message);

	/**
	 * Send binary message to all Connections in the WsHub.
	 * @param binary_message
	 * @param size
	 */
	void sendBinaryMessage(const void *binary_message, int size);

private:
	oatpp::String m_name;
	std::unordered_map<v_int32, std::shared_ptr<WsInstance>> m_ConnectionById;
	std::mutex m_ConnectionByIdLock;
	// Serialization and Deserialization Engine
	std::shared_ptr<SerializationEngine> m_SerializationEngine;
	std::shared_ptr<DeserializationEngine> m_DeserializationEngine;
	// Message Bus
	std::shared_ptr<MessageBus> m_MessageBus;
};

#endif // WsHUB_HPP
