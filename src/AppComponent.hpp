/*
 * AppComponent.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-13

Description: App Components

**************************************************/

#ifndef AppComponent_hpp
#define AppComponent_hpp

#include "config.h"

#include "websocket/WebSocketServer.hpp"

#include "ErrorHandler.hpp"

#include "components/SwaggerComponent.hpp"
#include "components/DatabaseComponent.hpp"

#include "oatpp-openssl/server/ConnectionProvider.hpp"
#include "oatpp-openssl/configurer/TrustStore.hpp"
#include "oatpp-openssl/Config.hpp"

#if ENABLE_ASYNC
#include "oatpp/web/server/AsyncHttpConnectionHandler.hpp"
#include "oatpp-websocket/AsyncConnectionHandler.hpp"
#else
#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp-websocket/ConnectionHandler.hpp"
#endif

#include "oatpp/web/server/HttpRouter.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"

// #include "oatpp/network/virtual_/server/ConnectionProvider.hpp"
// #include "oatpp/network/virtual_/Interface.hpp"

#include "oatpp-zlib/EncoderProvider.hpp"
#include "oatpp/web/protocol/http/incoming/SimpleBodyDecoder.hpp"

// #include "interceptor/AuthInterceptor.hpp"
#include "oatpp/web/server/interceptor/AllowCorsGlobal.hpp"

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "oatpp/core/macro/component.hpp"

#include <thread>

/**
 *  Class which creates and holds Application components and registers components in oatpp::base::Environment
 *  Order of components initialization is from top to bottom
 */
class AppComponent
{
private:
    v_uint16 m_port;

public:
    AppComponent(v_uint16 port)
        : m_port(port)
    {
    }
    /**
     *  Swagger component
     */
    SwaggerComponent swaggerComponent;

    /**
     * Database component
     */
    DatabaseComponent databaseComponent;

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

    // OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, virtualInterface)
    //([]
    //  { return oatpp::network::virtual_::Interface::obtainShared("virtualhost"); }());
#endif

    /**
     * Create ObjectMapper component to serialize/deserialize DTOs in Controller's API
     */
    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, apiObjectMapper)
    ([]
     {
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
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
		{ // Use oatpp virtual interface
			//OATPP_COMPONENT(std::shared_ptr<oatpp::network::virtual_::Interface>, interface);
			//connectionProvider = oatpp::network::virtual_::server::ConnectionProvider::createShared(interface);
    	} 
		else 
		{
      		connectionProvider = oatpp::network::tcp::server::ConnectionProvider::createShared({"0.0.0.0", m_port, oatpp::network::Address::IP_4});
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
    ("http", []
     {
        // OATPP_COMPONENT(std::shared_ptr<JWT>, jwt);                                         // get JWT component
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
        // oatpp-jwt for login system
        // connectionHandler->addRequestInterceptor(std::make_shared<oatpp::web::server::interceptor::AllowOptionsGlobal>());
        // connectionHandler->addRequestInterceptor(std::make_shared<AuthInterceptor>(jwt));
        // connectionHandler->addResponseInterceptor(std::make_shared<oatpp::web::server::interceptor::AllowCorsGlobal>());
        return connectionHandler; }());

    OATPP_CREATE_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>, websocketConnectionHandler)
    ("websocket", []
     {
#if ENABLE_ASYNC
        OATPP_COMPONENT(std::shared_ptr<oatpp::async::Executor>, executor);
        auto connectionHandler = oatpp::websocket::AsyncConnectionHandler::createShared(executor);
#else
        auto connectionHandler = oatpp::websocket::ConnectionHandler::createShared();
#endif
        connectionHandler->setSocketInstanceListener(std::make_shared<WSInstanceListener>());
        return connectionHandler; }());
};

#endif /* AppComponent_hpp */