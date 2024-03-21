/*
 * AsyncClientController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Async Client Controller

**************************************************/

#ifndef LITHIUM_ASYNC_CLIENT_CONTROLLER_HPP
#define LITHIUM_ASYNC_CLIENT_CONTROLLER_HPP

#include "Constants.hpp"

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/web/server/api/ApiController.hpp"


#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class ClientController : public oatpp::web::server::api::ApiController {
private:
    typedef ClientController __ControllerType;

private:
    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                    websocketConnectionHandler, Constants::COMPONENT_WS_API);

public:
    ClientController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                     objectMapper,
                                     Constants::COMPONENT_REST_API))
        : oatpp::web::server::api::ApiController(objectMapper) {}

public:
    /**
     * Join existing game
     */
    ENDPOINT_ASYNC("GET", "api/join-game/*", WS_CLIENT) {
        ENDPOINT_ASYNC_INIT(WS_CLIENT);

        Action act() override {
            /* Websocket handshake */
            auto response = oatpp::websocket::Handshaker::serversideHandshake(
                request->getHeaders(), controller->websocketConnectionHandler);
            auto parameters = std::make_shared<
                oatpp::network::ConnectionHandler::ParameterMap>();

            (*parameters)[Constants::PARAM_GAME_ID] =
                request->getQueryParameter(Constants::PARAM_GAME_ID);
            (*parameters)[Constants::PARAM_GAME_SESSION_ID] =
                request->getQueryParameter(Constants::PARAM_GAME_SESSION_ID);
            (*parameters)[Constants::PARAM_PEER_TYPE] =
                Constants::PARAM_PEER_TYPE_CLIENT;

            /* Set connection upgrade params */
            response->setConnectionUpgradeParameters(parameters);

            return _return(response);
        }
    };

    /**
     * Create new host
     */
    ENDPOINT_ASYNC("GET", "api/create-game/*", WS_HOST) {
        ENDPOINT_ASYNC_INIT(WS_HOST);

        Action act() override {
            /* Websocket handshake */
            auto response = oatpp::websocket::Handshaker::serversideHandshake(
                request->getHeaders(), controller->websocketConnectionHandler);
            auto parameters = std::make_shared<
                oatpp::network::ConnectionHandler::ParameterMap>();

            (*parameters)[Constants::PARAM_GAME_ID] =
                request->getQueryParameter(Constants::PARAM_GAME_ID);
            (*parameters)[Constants::PARAM_GAME_SESSION_ID] =
                request->getQueryParameter(Constants::PARAM_GAME_SESSION_ID);
            (*parameters)[Constants::PARAM_PEER_TYPE] =
                Constants::PARAM_PEER_TYPE_HOST;

            /* Set connection upgrade params */
            response->setConnectionUpgradeParameters(parameters);

            return _return(response);
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif /* LITHIUM_ASYNC_CLIENT_CONTROLLER_HPP */
