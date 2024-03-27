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

#include "config.h"

#include "config/Config.hpp"
#include "config/HubsConfig.hpp"

#include "websocket/Registry.hpp"
#if ENABLE_ASYNC
#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#else
#include "oatpp-websocket/ConnectionHandler.hpp"
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#endif

#include "ErrorHandler.hpp"

#include "oatpp/core/base/CommandLineArguments.hpp"
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

#include "oatpp-openssl/Config.hpp"
#include "oatpp-openssl/configurer/TrustStore.hpp"
#include "oatpp-openssl/server/ConnectionProvider.hpp"
#include "oatpp-zlib/EncoderProvider.hpp"

#include <cstdlib>
#include <thread>  // for std::thread::hardware_concurrency

// #include "data/SystemCustom.hpp"

/**
 *  Class which creates and holds Application components and registers
 * components in oatpp::base::Environment Order of components initialization is
 * from top to bottom
 */
class AppComponent {
    /* note: though I don't like this kind of initialization, but I don't know
     * how to do it in a better way, segmentation fault ! */
private:
    oatpp::String m_host;
    v_uint16 m_port;

    oatpp::base::CommandLineArguments m_cmdArgs;  // command line arguments

public:
    /**
     *  Create components
     *  @param host - host name
     *  @param port - port number
     */
    explicit AppComponent(oatpp::String host, v_uint16 port)
        : m_host(host), m_port(port) {}
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
        auto config = std::make_shared<HubsConfig>(nullptr);
        auto Hub1 = HubConfigDto::createShared();
        auto Hub2 = HubConfigDto::createShared();
        Hub1->hubId = "device";
        Hub2->hubId = "script";
        config->putHubConfig(Hub1);
        config->putHubConfig(Hub2);
        return config;
    }());

#if ENABLE_ASYNC
    /**
     * Create Async Executor
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)
    ([] {
        return std::make_shared<oatpp::async::Executor>(
            std::thread::hardware_concurrency() +
                2 /* Data-Processing threads */,
            1 /* I/O threads */, 1 /* Timer threads */
        );
    }());
#endif

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

    /**
     *  Create Router component
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>,
                           httpRouter)
    ([] { return oatpp::web::server::HttpRouter::createShared(); }());

    /**
     * Create ObjectMapper component to serialize/deserialize DTOs in
     * Controller's API
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
     *  Create hubs sessions Registry component.
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<Registry>, hubsSessionsRegistry)
    ([] { return std::make_shared<Registry>(); }());

    /**
     *  Create ConnectionProvider component which listens on the port
     */
    OATPP_CREATE_COMPONENT(
        std::shared_ptr<oatpp::network::ServerConnectionProvider>,
        serverConnectionProvider)
    ([this] {
        std::shared_ptr<oatpp::network::ServerConnectionProvider>
            connectionProvider;
        if (m_port == 0) {
#if ENABLE_DEBUG
            OATPP_COMPONENT(
                std::shared_ptr<oatpp::network::virtual_::Interface>,
                interface);
            connectionProvider = oatpp::network::virtual_::server::
                ConnectionProvider::createShared(interface);
#endif
        } else {
            connectionProvider =
                oatpp::network::tcp::server::ConnectionProvider::createShared(
#if ENABLE_IPV6
                    { m_host, m_port, oatpp::network::Address::IP_6 }
#else
                    {m_host, m_port, oatpp::network::Address::IP_4}
#endif
                );
        }
        return connectionProvider;
    }());

    /**
     *  Create ConnectionHandler component which uses Router component to route
     * requests
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                           serverConnectionHandler)
    ("http", [] {  // get JWT component
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>,
                        router);  // get Router component
        OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                        objectMapper);  // get ObjectMapper component
                                        /* Create HttpProcessor::Components */
        auto components =
            std::make_shared<oatpp::web::server::HttpProcessor::Components>(
                router);

        /* Add content encoders */
        auto encoders = std::make_shared<
            oatpp::web::protocol::http::encoding::ProviderCollection>();
        encoders->add(std::make_shared<oatpp::zlib::DeflateEncoderProvider>());
        encoders->add(std::make_shared<oatpp::zlib::GzipEncoderProvider>());
        /* Set content encoders */
        components->contentEncodingProviders = encoders;

        auto decoders = std::make_shared<
            oatpp::web::protocol::http::encoding::ProviderCollection>();
        decoders->add(std::make_shared<oatpp::zlib::DeflateDecoderProvider>());
        decoders->add(std::make_shared<oatpp::zlib::GzipDecoderProvider>());
        /* Set Body Decoder */
        components->bodyDecoder = std::make_shared<
            oatpp::web::protocol::http::incoming::SimpleBodyDecoder>(decoders);

#if ENABLE_ASYNC
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>,
                        executor);  // get Async executor component
        auto connectionHandler =
            oatpp::web::server::AsyncHttpConnectionHandler::createShared(
                components, executor);
        connectionHandler->setErrorHandler(
            std::make_shared<ErrorHandler>(objectMapper));
#else
        auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(components);
        connectionHandler->setErrorHandler(std::make_shared<ErrorHandler>(objectMapper));
#endif
        return connectionHandler;
    }());

    /**
     *  Create websocket connection handler
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                           websocketConnectionHandler)
    (Constants::COMPONENT_WS_API, [] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        OATPP_COMPONENT(std::shared_ptr<Registry>, registry);
#if ENABLE_ASYNC
        auto connectionHandler =
            oatpp::websocket::AsyncConnectionHandler::createShared(executor);
        connectionHandler->setSocketInstanceListener(registry);
#else
        auto connectionHandler = oatpp::websocket::ConnectionHandler::createShared();
#endif
        return connectionHandler;
    }());
};

#endif /* LITHIUM_APP_COMPONENT_HPP */