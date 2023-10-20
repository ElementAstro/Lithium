
#ifndef WsPluginHUB_HPP
#define WsPluginHUB_HPP

#include "WsPluginInstance.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

class WsPluginHub
{
private:
	oatpp::String m_name;
	std::unordered_map<v_int32, std::shared_ptr<WsPluginInstance>> m_pluginById;
	std::mutex m_pluginByIdLock;

public:
	WsPluginHub(const oatpp::String &name)
		: m_name(name)
	{
	}

	/**
	 * Add plugin to the WsPluginHub.
	 * @param plugin
	 */
	void addplugin(const std::shared_ptr<WsPluginInstance> &plugin);

	/**
	 * Remove plugin from the WsPluginHub.
	 * @param userId
	 */
	void removepluginByUserId(v_int32 userId);

	/**
	 * Send message to all plugins in the WsPluginHub.
	 * @param message
	 */
	void sendMessage(const oatpp::String &message);

	/**
	 * Send binary message to all plugins in the WsPluginHub.
	 * @param binary_message
	 * @param size
	 */
	void sendBinaryMessage(const void *binary_message, int size);
};

#endif // WsPluginHUB_HPP
