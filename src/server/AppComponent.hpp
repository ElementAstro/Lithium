/*
 * AppComponent.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: App Components

**************************************************/

#ifndef LITHIUM_APP_COMPONENT_HPP
#define LITHIUM_APP_COMPONENT_HPP

#include "config/Config.hpp"
#include "config/HubsConfig.hpp"

#include "websocket/Registry.hpp"

// Websocket
#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"

#include "ErrorHandler.hpp"

#include "oatpp/core/macro/component.hpp"
#include "oatpp/core/utils/ConversionUtils.hpp"
#include "oatpp/network/monitor/ConnectionInactivityChecker.hpp"
#include "oatpp/network/monitor/ConnectionMaxAgeChecker.hpp"
#include "oatpp/network/monitor/ConnectionMonitor.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/protocol/http/incoming/SimpleBodyDecoder.hpp"
#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/web/server/interceptor/AllowCorsGlobal.hpp"

#if ENABLE_DEBUG
#include "oatpp/network/virtual_/Interface.hpp"
#include "oatpp/network/virtual_/server/ConnectionProvider.hpp"
#endif

#include "components/SwaggerComponent.hpp"

// SSL
#include "oatpp-openssl/Config.hpp"
#include "oatpp-openssl/configurer/TrustStore.hpp"
#include "oatpp-openssl/server/ConnectionProvider.hpp"

// GZip
#include "oatpp-zlib/EncoderProvider.hpp"

#include <cstdlib>
#include <thread>  // for std::thread::hardware_concurrency

// #include "data/SystemCustom.hpp"

#include <cstdlib>

/**
 *  Class which creates and holds Application components and registers
 * components in oatpp::base::Environment Order of components initialization is
 * from top to bottom
 */
class AppComponent {
public:
    AppComponent() = default;

public:
    /**
     *  Swagger component
     */
    SwaggerComponent swaggerComponent;

    /**
     * Create config component
     */
    OATPP_CREATE_COMPONENT(oatpp::Object<ConfigDto>, appConfig)
    ([this] {
        auto config = ConfigDto::createShared();

        auto hostServer = ServerConfigDto::createShared();
        hostServer->host = "0.0.0.0";
        hostServer->port = 8000;

        auto clientServer = ServerConfigDto::createShared();
        clientServer->host = "0.0.0.0";
        clientServer->port = 8001;

        config->hostAPIServer = hostServer;
        config->clientAPIServer = clientServer;

        return config;
    }());

    /**
     * Hub configs
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<HubsConfig>, hubConfig)
    ([] {
        // We specify the default config here
        auto config = std::make_shared<HubsConfig>(nullptr);
        auto Hub1 = HubConfigDto::createShared();
        auto Hub2 = HubConfigDto::createShared();
        // Script and device are the default hubs
        Hub1->hubId = "device";
        Hub2->hubId = "script";
        config->putHubConfig(Hub1);
        config->putHubConfig(Hub2);
        return config;
    }());

    /**
     * Create Async Executor
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)
    ([] { return std::make_shared<oatpp::async::Executor>(); }());

    /**
     *  Create Router component
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>,
                           httpRouter)
    ([] { return oatpp::web::server::HttpRouter::createShared(); }());

    /**
     *  Create ObjectMapper component to serialize/deserialize DTOs in
     * Contoller's API
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                           apiObjectMapper)
    (Constants::COMPONENT_REST_API, [] {
        /* create serializer and deserializer configurations */
        auto serializeConfig =
            oatpp::parser::json::mapping::Serializer::Config::createShared();
        auto deserializeConfig =
            oatpp::parser::json::mapping::Deserializer::Config::createShared();

        /* enable beautifier */
        serializeConfig->useBeautifier = true;

        auto objectMapper =
            oatpp::parser::json::mapping::ObjectMapper::createShared(
                serializeConfig, deserializeConfig);
        objectMapper->getDeserializer()->getConfig()->allowUnknownFields =
            false;

        // objectMapper->getSerializer()->getConfig()->enabledInterpretations =
        // {
        //     "system::memory"};
        // objectMapper->getDeserializer()->getConfig()->enabledInterpretations
        // = {
        //     "system::memory"};
        return objectMapper;
    }());

    /**
     *  Create ObjectMapper component to serialize/deserialize DTOs in WS
     * communication
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                           wsApiObjectMapper)
    (Constants::COMPONENT_WS_API, [] {
        auto mapper =
            oatpp::parser::json::mapping::ObjectMapper::createShared();
        mapper->getSerializer()->getConfig()->includeNullFields = false;
        return mapper;
    }());

    /**
     *  Create games sessions Registry component.
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<Registry>, gamesSessionsRegistry)
    ([] { return std::make_shared<Registry>(); }());

    

    /**
     *  Create websocket connection handler
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                           websocketConnectionHandler)
    (Constants::COMPONENT_WS_API, [] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        OATPP_COMPONENT(std::shared_ptr<Registry>, registry);
        auto connectionHandler =
            oatpp::websocket::AsyncConnectionHandler::createShared(executor);
        connectionHandler->setSocketInstanceListener(registry);
        return connectionHandler;
    }());

    /**
     * Create Debug virtual interface component
     */
#if ENABLE_DEBUG
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>,
                           virtualInterface)
    ([] {
        return oatpp::network::virtual_::Interface::obtainShared("virtualhost");
    }());
#endif
};

#endif /* LITHIUM_APP_COMPONENT_HPP */