
#ifndef WSDEVICEHUB_HPP
#define WSDEVICEHUB_HPP

#include "WsDeviceInstance.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

class WsDeviceHub
{
private:
	oatpp::String m_name;
	std::unordered_map<v_int32, std::shared_ptr<WsDeviceInstance>> m_deviceById;
	std::mutex m_deviceByIdLock;

public:
	WsDeviceHub(const oatpp::String &name)
		: m_name(name)
	{
	}

	/**
	 * Add device to the WsDeviceHub.
	 * @param device
	 */
	void adddevice(const std::shared_ptr<WsDeviceInstance> &device);

	/**
	 * Remove device from the WsDeviceHub.
	 * @param userId
	 */
	void removedeviceByUserId(v_int32 userId);

	/**
	 * Send message to all devices in the WsDeviceHub.
	 * @param message
	 */
	void sendMessage(const oatpp::String &message);

	/**
	 * Send binary message to all devices in the WsDeviceHub.
	 * @param binary_message
	 * @param size
	 */
	void sendBinaryMessage(const void *binary_message, int size);
};

#endif // WSDEVICEHUB_HPP
