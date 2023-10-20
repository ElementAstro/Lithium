/*
 * WsDeviceServer.cpp
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

Description: WebSocket Device Server

**************************************************/

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