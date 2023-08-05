
#include "WsDeviceServer.hpp"

v_int32 WsDeviceServer::obtainNewUserId()
{
	return m_userIdCounter++;
}

std::shared_ptr<WsDeviceHub> WsDeviceServer::getOrCreateHub(const oatpp::String &hubName)
{
	std::lock_guard<std::mutex> lock(m_hubsMutex);
	std::shared_ptr<WsDeviceHub> &hub = m_hubs[hubName];
	if (!hub)
	{
		hub = std::make_shared<WsDeviceHub>(hubName);
	}
	return hub;
}

void WsDeviceServer::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params)
{
	auto deviceName = params->find("deviceName")->second;
	auto deviceHub = params->find("deviceHub")->second;
	auto hub = getOrCreateHub(deviceHub);

	auto device = std::make_shared<WsDeviceInstance>(socket, hub, deviceName, obtainNewUserId());
	socket->setListener(device);

	hub->adddevice(device);
	hub->sendMessage(deviceName + " joined " + deviceHub);
}

void WsDeviceServer::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket)
{

	auto device = std::static_pointer_cast<WsDeviceInstance>(socket->getListener());
	auto device_name = device->getDeviceName();
	auto hub = device->getHub();

	hub->removedeviceByUserId(device->getUserId());

	hub->sendMessage(device_name + " left the hub");

	/* Remove circle `std::shared_ptr` dependencies */
	socket->setListener(nullptr);
}