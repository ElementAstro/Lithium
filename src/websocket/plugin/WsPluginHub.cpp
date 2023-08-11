
#include "WsPluginHub.hpp"

void WsPluginHub::addplugin(const std::shared_ptr<WsPluginInstance> &plugin)
{
	std::lock_guard<std::mutex> guard(m_pluginByIdLock);
	m_pluginById[plugin->getUserId()] = plugin;
}

void WsPluginHub::removepluginByUserId(v_int32 userId)
{
	std::lock_guard<std::mutex> guard(m_pluginByIdLock);
	m_pluginById.erase(userId);
}

void WsPluginHub::sendMessage(const oatpp::String &message)
{
	std::lock_guard<std::mutex> guard(m_pluginByIdLock);
	for (auto &pair : m_pluginById)
	{
		pair.second->sendMessage(message);
	}
}

void WsPluginHub::sendBinaryMessage(const void *binary_message, int size)
{
	std::lock_guard<std::mutex> guard(m_pluginByIdLock);
	for (auto &pair : m_pluginById)
	{
		pair.second->sendBinaryMessage(binary_message, size);
	}
}