
#include "WsPluginServer.hpp"

v_int32 WsPluginServer::obtainNewUserId()
{
	return m_userIdCounter++;
}

std::shared_ptr<WsPluginHub> WsPluginServer::getOrCreateHub(const oatpp::String &hubName)
{
	std::lock_guard<std::mutex> lock(m_hubsMutex);
	std::shared_ptr<WsPluginHub> &hub = m_hubs[hubName];
	if (!hub)
	{
		hub = std::make_shared<WsPluginHub>(hubName);
	}
	return hub;
}

void WsPluginServer::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params)
{
	auto pluginName = params->find("pluginName")->second;
	auto pluginHub = params->find("pluginHub")->second;
	auto hub = getOrCreateHub(pluginHub);

	auto plugin = std::make_shared<WsPluginInstance>(socket, hub, pluginName, obtainNewUserId());
	socket->setListener(plugin);

	hub->addplugin(plugin);
	hub->sendMessage(pluginName + " joined " + pluginHub);
}

void WsPluginServer::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket)
{

	auto plugin = std::static_pointer_cast<WsPluginInstance>(socket->getListener());
	auto plugin_name = plugin->getPluginName();
	auto hub = plugin->getHub();

	hub->removepluginByUserId(plugin->getUserId());

	hub->sendMessage(plugin_name + " left the hub");

	/* Remove circle `std::shared_ptr` dependencies */
	socket->setListener(nullptr);
}