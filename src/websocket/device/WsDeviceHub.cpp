
#include "WsDeviceHub.hpp"

void WsDeviceHub::adddevice(const std::shared_ptr<WsDeviceInstance> &device)
{
	std::lock_guard<std::mutex> guard(m_deviceByIdLock);
	m_deviceById[device->getUserId()] = device;
}

void WsDeviceHub::removedeviceByUserId(v_int32 userId)
{
	std::lock_guard<std::mutex> guard(m_deviceByIdLock);
	m_deviceById.erase(userId);
}

void WsDeviceHub::sendMessage(const oatpp::String &message)
{
	std::lock_guard<std::mutex> guard(m_deviceByIdLock);
	for (auto &pair : m_deviceById)
	{
		pair.second->sendMessage(message);
	}
}

void WsDeviceHub::sendBinaryMessage(const void *binary_message, int size)
{
	std::lock_guard<std::mutex> guard(m_deviceByIdLock);
	for (auto &pair : m_deviceById)
	{
		pair.second->sendBinaryMessage(binary_message, size);
	}
}