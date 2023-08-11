
#ifndef WsPluginINSTANCE_HPP
#define WsPluginINSTANCE_HPP

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

class WsPluginHub; // FWD

class WsPluginInstance : public oatpp::websocket::AsyncWebSocket::Listener
{
private:
	/**
	 * Buffer for messages. Needed for multi-frame messages.
	 */
	oatpp::data::stream::BufferOutputStream m_messageBuffer;

	/**
	 * Lock for synchronization of writes to the web socket.
	 */
	oatpp::async::Lock m_writeLock;

	std::unique_ptr<CommandDispatcher> m_CommandDispatcher;

	template <typename ClassType>
	void LiRegisterFunc(const std::string &name, const nlohmann::json (ClassType::*handler)(const nlohmann::json &))
	{
		m_CommandDispatcher->RegisterHandler(name, handler, this);
	}

	bool LiRunFunc(const std::string &name, const nlohmann::json &params)
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
	std::shared_ptr<WsPluginHub> m_hub;
	oatpp::String m_plugin_name;
	v_int32 m_userId;

private:
	/**
	 * Inject async executor object.
	 */
	OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);

public:
	WsPluginInstance(const std::shared_ptr<AsyncWebSocket> &socket,
					 const std::shared_ptr<WsPluginHub> &hub,
					 const oatpp::String &plugin_name,
					 v_int32 userId)
		: m_socket(socket), m_hub(hub), m_plugin_name(plugin_name), m_userId(userId)
	{
		OATPP_LOGD(m_plugin_name.getValue("").c_str(), "%s created", m_plugin_name.getValue("").c_str());
	}

	/**
	 * Send message to WsPluginInstance (to user).
	 * @param message
	 */
	void sendMessage(const oatpp::String &message);

	void sendBinaryMessage(const void *binary_message, int size);

	/**
	 * Get hub of the WsPluginInstance.
	 * @return
	 */
	std::shared_ptr<WsPluginHub> getHub();

	/**
	 * Get WsPluginInstance plugin_name.
	 * @return
	 */
	oatpp::String getPluginName();

	/**
	 * Get WsPluginInstance userId.
	 * @return
	 */
	v_int32 getUserId();

public: // WebSocket Listener methods
	CoroutineStarter onPing(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;
	CoroutineStarter onPong(const std::shared_ptr<AsyncWebSocket> &socket, const oatpp::String &message) override;
	CoroutineStarter onClose(const std::shared_ptr<AsyncWebSocket> &socket, v_uint16 code, const oatpp::String &message) override;
	CoroutineStarter readMessage(const std::shared_ptr<AsyncWebSocket> &socket, v_uint8 opcode, p_char8 data, oatpp::v_io_size size) override;
};

#endif // WsPluginINSTANCE_HPP
