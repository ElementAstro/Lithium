
#ifndef WsPluginSERVER_HPP
#define WsPluginSERVER_HPP

#include "WsPluginHub.hpp"

#include "config.h"

#if ENABLE_ASYNC
#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#else
#include "oatpp-websocket/ConnectionHandler.hpp"
#endif

#include <unordered_map>
#include <mutex>

class WsPluginServer : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener
{
public:
	std::atomic<v_int32> m_userIdCounter;
	std::unordered_map<oatpp::String, std::shared_ptr<WsPluginHub>> m_hubs;
	std::mutex m_hubsMutex;

public:
	WsPluginServer()
		: m_userIdCounter(0)
	{
	}

	/**
	 * Generate id for new user
	 * @return
	 */
	v_int32 obtainNewUserId();

	/**
	 * Get plugin hub by name or create new one if not exists.
	 * @param hubName
	 * @return
	 */
	std::shared_ptr<WsPluginHub> getOrCreateHub(const oatpp::String &hubName);

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

#endif // WsPluginSERVER_HPP