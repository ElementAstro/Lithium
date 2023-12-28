/*
 * WsServer.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-15

Description: WebSocket Server

**************************************************/

#include "WsServer.hpp"

WsServer::WsServer()
	: m_ConnectionCounter(0)
{
	m_CommandDispatcher = std::make_shared<CommandDispatcher<ReturnMessage, std::shared_ptr<IParams>>();
	m_SerializationEngine = std::make_shared<SerializationEngine>();
	m_DeserializationEngine = std::make_shared<DeserializationEngine>();

	m_SerializationEngine->addRenderEngine("json", std::make_shared<JsonRenderEngine>());
	m_DeserializationEngine->addDeserializeEngine("json", std::make_shared<JsonDeserializer>());
}
v_int32 WsServer::obtainNewConnectionId()
{
	return m_ConnectionCounter++;
}

std::shared_ptr<WsHub> WsServer::getOrCreateHub(const oatpp::String &hubName)
{
	std::lock_guard<std::mutex> lock(m_hubsMutex);
	std::shared_ptr<WsHub> &hub = m_hubs[hubName];
	if (!hub)
	{
		hub = std::make_shared<WsHub>(hubName);
		
	}
	return hub;
}

void WsServer::onAfterCreate_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket, const std::shared_ptr<const ParameterMap> &params)
{
	auto pluginName = params->find("pluginName")->second;
	auto pluginHub = params->find("pluginHub")->second;
	auto hub = getOrCreateHub(pluginHub);

	auto plugin = std::make_shared<WsInstance>(socket, hub, pluginName, obtainNewConnectionId());
	socket->setListener(plugin);

	hub->addConnection(plugin);
	hub->sendMessage(pluginName + " joined " + pluginHub);
}

void WsServer::onBeforeDestroy_NonBlocking(const std::shared_ptr<AsyncWebSocket> &socket)
{
	auto plugin = std::static_pointer_cast<WsInstance>(socket->getListener());
	auto hub = plugin->getHub();
	hub->removeConnectionByUserId(plugin->getId());
	/* Remove circle `std::shared_ptr` dependencies */
	socket->setListener(nullptr);
}