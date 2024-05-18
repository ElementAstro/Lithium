/*
 * Registry.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: Websocket Hub Registry

**************************************************/

#ifndef LITHIUM_WEBSOCKET_REGISTRY_HPP
#define LITHIUM_WEBSOCKET_REGISTRY_HPP

#include "Hub.hpp"

#include "oatpp-websocket/AsyncConnectionHandler.hpp"

#include <mutex>
#include <unordered_map>

class Registry
    : public oatpp::websocket::AsyncConnectionHandler::SocketInstanceListener {
private:
    struct SessionInfo {
        std::shared_ptr<Session> session;
        oatpp::Object<ErrorDto> error;
        bool isHost;
    };

private:
    std::unordered_map<oatpp::String, std::shared_ptr<Hub>> m_hubs;
    std::mutex m_mutex;

private:
    /* Inject application components */
    OATPP_COMPONENT(oatpp::Object<ConfigDto>, m_config);
    OATPP_COMPONENT(std::shared_ptr<HubsConfig>, m_hubConfig);
    OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, m_asyncExecutor);
    OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                    m_objectMapper, Constants::COMPONENT_REST_API);

private:
    oatpp::String getRequiredParameter(
        const oatpp::String& name,
        const std::shared_ptr<const ParameterMap>& params,
        SessionInfo& sessionInfo);

private:
    void sendSocketErrorAsync(const std::shared_ptr<AsyncWebSocket>& socket,
                              const oatpp::Object<ErrorDto>& error,
                              bool fatal = false);
    SessionInfo getSessionForConnection(
        const std::shared_ptr<AsyncWebSocket>& socket,
        const std::shared_ptr<const ParameterMap>& params);

public:
    Registry();

    /**
     * Get all sessions of the hub.
     * @param hubId
     * @return
     */
    std::shared_ptr<Hub> getHubById(const oatpp::String& hubId);

public:
    /**
     *  Called when socket is created
     */
    void onAfterCreate_NonBlocking(
        const std::shared_ptr<AsyncWebSocket>& socket,
        const std::shared_ptr<const ParameterMap>& params) override;

    /**
     *  Called before socket instance is destroyed.
     */
    void onBeforeDestroy_NonBlocking(
        const std::shared_ptr<AsyncWebSocket>& socket) override;
};

#endif  // LITHIUM_WEBSOCKET_REGISTRY_HPP
