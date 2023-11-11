/*
 * WsDeviceHub.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.	If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-10-20

Description: WebSocket Device Hub (all devices of one type in one hub)

**************************************************/

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
	void addDevice(const std::shared_ptr<WsDeviceInstance> &device);

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
	void sendBinaryMessage(void *binary_message, int size);
};

#endif // WSDEVICEHUB_HPP
