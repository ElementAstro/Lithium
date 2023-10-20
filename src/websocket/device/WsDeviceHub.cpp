/*
 * WsDeviceHub.cpp
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

void WsDeviceHub::sendBinaryMessage(void *binary_message, int size)
{
	std::lock_guard<std::mutex> guard(m_deviceByIdLock);
	for (auto &pair : m_deviceById)
	{
		pair.second->sendBinaryMessage(binary_message, size);
	}
}