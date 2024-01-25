/*
 * AppComponent.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: App Components

**************************************************/

#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "config.h"

#ifdef ENABLE_ASYNC
#include "websocket/WsServer.hpp"
#else
#include "websocket/WsServer.hpp"
#endif

#include "ErrorHandler.hpp"

#if ENABLE_ASYNC
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#else
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp-websocket/ConnectionHandler.hpp"
#endif

#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/monitor/ConnectionMaxAgeChecker.hpp"
#include "oatpp/network/monitor/ConnectionInactivityChecker.hpp"
#include "oatpp/network/monitor/ConnectionMonitor.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/web/protocol/http/incoming/SimpleBodyDecoder.hpp"
#include "oatpp/web/server/interceptor/AllowCorsGlobal.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/component.hpp"
#if ENABLE_DEBUG
#include "oatpp/network/virtual_/server/ConnectionProvider.hpp"
#include "oatpp/network/virtual_/Interface.hpp"
#endif

#include "controller/SwaggerComponent.hpp"

#include "oatpp-openssl/server/ConnectionProvider.hpp"
#include "oatpp-openssl/configurer/TrustStore.hpp"
#include "oatpp-openssl/Config.hpp"
#include "oatpp-zlib/EncoderProvider.hpp"

#include <thread> // for std::thread::hardware_concurrency

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent
{
    /* note: though I don't like this kind of initialization, but I don't know how to do it in a better way, segmentation fault ! */
private:
    oatpp::String m_host;
    v_uint16 m_port;

public:
    /**
     *  Create components
     *  @param host - host name
     *  @param port - port number
     */
    AppComponent(oatpp::String host, v_uint16 port)
        : m_host(host), m_port(port)
    {
    }
    /**
     *  Swagger component
     */
    SwaggerComponent swaggerComponent;

#if ENABLE_ASYNC
    /**
     * Create Async Executor
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor)
    ([]
     { return std::make_shared<oatpp::async::Executor>(
           std::thread::hardware_concurrency() + 2 /* Data-Processing threads */,
           1 /* I/O threads */,
           1 /* Timer threads */
       ); }());
#endif

#if ENABLE_DEBUG
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, virtualInterface)
    ([]
     { return oatpp::network::virtual_::Interface::obtainShared("virtualhost"); }());
#endif

    /**
     * Create ObjectMapper component to serialize/deserialize DTOs in Controller's API
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)
    ([]
     {
        /* create serializer and deserializer configurations */
        auto serializeConfig = oatpp::parser::json::mapping::Serializer::Config::createShared();
        auto deserializeConfig = oatpp::parser::json::mapping::Deserializer::Config::createShared();

        /* enable beautifier */
        serializeConfig->useBeautifier = true;

        auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared(serializeConfig, deserializeConfig);
        objectMapper->getDeserializer()->getConfig()->allowUnknownFields = false;
        return objectMapper; }());

    /**
     *  Create ConnectionProvider component which listens on the port
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, serverConnectionProvider)
    ([this]
     { 
        std::shared_ptr<oatpp::network::ServerConnectionProvider> connectionProvider;
		if(m_port == 0) 
		{
#if ENABLE_DEBUG
            OATPP_LOGD("Debug", "Debug server is starting ...");
            OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, interface);
			connectionProvider = oatpp::network::virtual_::server::ConnectionProvider::createShared(interface);
#else
            OATPP_LOGE("Debug", "Debug mode is not enabled,please enable when compile");
#endif
    	} 
		else 
		{
#if ENABLE_IPV6
            connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared({m_host, m_port, oatpp::network::Address::IP_6});
#else
            connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared({m_host, m_port, oatpp::network::Address::IP_4});
#endif	
    	}
        return connectionProvider; }());

    /**
     *  Create Router component
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)
    ([]
     { return oatpp::web::server::HttpRouter::createShared(); }());

    /**
     *  Create ConnectionHandler component which uses Router component to route requests
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, serverConnectionHandler)
    ("http", [] {                                                                           // get JWT component
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);           // get Router component
        OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper); // get ObjectMapper component
                                                                                            /* Create HttpProcessor::Components */
        auto components = std::make_shared<oatpp::web::server::HttpProcessor::Components>(router);

        /* Add content encoders */
        auto encoders = std::make_shared<oatpp::web::protocol::http::encoding::ProviderCollection>();
        encoders->add(std::make_shared<oatpp::zlib::DeflateEncoderProvider>());
        encoders->add(std::make_shared<oatpp::zlib::GzipEncoderProvider>());
        /* Set content encoders */
        components->contentEncodingProviders = encoders;

        auto decoders = std::make_shared<oatpp::web::protocol::http::encoding::ProviderCollection>();
        decoders->add(std::make_shared<oatpp::zlib::DeflateDecoderProvider>());
        decoders->add(std::make_shared<oatpp::zlib::GzipDecoderProvider>());
        /* Set Body Decoder */
        components->bodyDecoder = std::make_shared<oatpp::web::protocol::http::incoming::SimpleBodyDecoder>(decoders);

#if ENABLE_ASYNC
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor); // get Async executor component
        auto connectionHandler = oatpp::web::server::AsyncHttpConnectionHandler::createShared(components, executor);
        connectionHandler->setErrorHandler(std::make_shared<ErrorHandler>(objectMapper));
#else
        auto connectionHandler = oatpp::web::server::HttpConnectionHandler::createShared(components);
        connectionHandler->setErrorHandler(std::make_shared<ErrorHandler>(objectMapper));
#endif
        return connectionHandler;
    }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler)
    ("websocket", []
     {
#if ENABLE_ASYNC
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared(executor);
#else
        auto connectionHandler = oatpp::websocket::ConnectionHandler::createShared();
#endif
        connectionHandler->setSocketInstanceListener(std::make_shared<WsServer>());
        return connectionHandler; }());
};

#endif /* AppComponent_hpp */