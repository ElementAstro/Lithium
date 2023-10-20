
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
