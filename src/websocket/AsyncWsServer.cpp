/*
 * AsyncWsServer.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-15

Description: WebSocket Server

**************************************************/

#include "AsyncWsServer.hpp"

AsyncWsServer::AsyncWsServer() : m_ConnectionCounter(0) {
    m_SerializationEngine =
        std::make_shared<Atom::Server::SerializationEngine>();
    m_DeserializationEngine =
        std::make_shared<Atom::Server::DeserializationEngine>();

    m_SerializationEngine->addSerializationEngine(
        "json", std::make_shared<JsonSerializationEngine>());
    m_DeserializationEngine->addDeserializeEngine(
        "json", std::make_shared<JsonDeserializer>());
}
v_int32 AsyncWsServer::obtainNewConnectionId() { return m_ConnectionCounter++; }

std::shared_ptr<AsyncWsHub> AsyncWsServer::getOrCreateHub(
    const oatpp::String &hubName) {
    std::lock_guard<std::mutex> lock(m_hubsMutex);
    std::shared_ptr<AsyncWsHub> &hub = m_hubs[hubName];
    if (!hub) {
        hub = std::make_shared<AsyncWsHub>(hubName);
    }
    return hub;
}

void AsyncWsServer::onAfterCreate_NonBlocking(
    const std::shared_ptr<AsyncWebSocket> &socket,
    const std::shared_ptr<const ParameterMap> &params) {
    auto pluginName = params->find("pluginName")->second;
    auto pluginHub = params->find("pluginHub")->second;
    auto hub = getOrCreateHub(pluginHub);

    auto plugin = std::make_shared<AsyncWsInstance>(socket, hub, pluginName,
                                                    obtainNewConnectionId());
    socket->setListener(plugin);

    hub->addConnection(plugin);
    hub->sendMessage(pluginName + " joined " + pluginHub);
}

void AsyncWsServer::onBeforeDestroy_NonBlocking(
    const std::shared_ptr<AsyncWebSocket> &socket) {
    auto plugin =
        std::static_pointer_cast<AsyncWsInstance>(socket->getListener());
    auto hub = plugin->getHub();
    hub->removeConnectionByUserId(plugin->getId());
    /* Remove circle `std::shared_ptr` dependencies */
    socket->setListener(nullptr);
}