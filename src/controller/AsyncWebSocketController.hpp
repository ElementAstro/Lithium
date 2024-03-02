/*
 * AsyncWebSocketController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Websocket Route

**************************************************/

#ifndef LITHIUM_ASYNC_WEBSOCKET_CONTROLLER_HPP
#define LITHIUM_ASYNC_WEBSOCKET_CONTROLLER_HPP

#include "config.h"

#include "oatpp-websocket/Handshaker.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/network/ConnectionHandler.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include <algorithm>
#include <vector>

#include OATPP_CODEGEN_BEGIN(ApiController)  //<-- codegen begin

/**
 * Controller with WebSocket-connect endpoint.
 */
class WebSocketController : public oatpp::web::server::api::ApiController {
private:
    typedef WebSocketController __ControllerType;

    OATPP_COMPONENT(std::shared_ptr<oatpp::network::ConnectionHandler>,
                    websocketConnectionHandler, "websocket");

public:
    WebSocketController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                        objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    // ----------------------------------------------------------------
    // Pointer creator
    // ----------------------------------------------------------------

    static std::shared_ptr<WebSocketController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<WebSocketController>(objectMapper);
    }

    // ----------------------------------------------------------------
    // Websocket Handler
    // ----------------------------------------------------------------

    ENDPOINT_ASYNC("GET", "/ws/{hub-name}", wsConsole){
        ENDPOINT_ASYNC_INIT(wsConsole) Action act()
            override{auto hubType = request->getPathVariable("hub-type");
    auto response = oatpp::websocket::Handshaker::serversideHandshake(
        request->getHeaders(), controller->websocketConnectionHandler);
    auto parameters =
        std::make_shared<oatpp::network::ConnectionHandler::ParameterMap>();
    (*parameters)["type"] = hubType;
    response->setConnectionUpgradeParameters(parameters);

    if (const std::string hub_type = hubType.getValue("");
        hub_type == "device") {
        std::vector<std::string> available_device_types = {
            "camera",      "telescope", "focuser",
            "filterwheel", "solver",    "guider"};
        auto it = std::find(available_device_types.begin(),
                            available_device_types.end(), hubType.getValue(""));
        OATPP_ASSERT_HTTP(it != available_device_types.end(), Status::CODE_500,
                          "Invalid device type");
    } else if (hub_type == "plugin") {
        std::vector<std::string> available_plugins = {"script", "exe",
                                                      "liscript"};
        auto it = std::find(available_plugins.begin(), available_plugins.end(),
                            hubType->c_str());
        OATPP_ASSERT_HTTP(it != available_plugins.end(), Status::CODE_500,
                          "Invalid plugin type");
    }

    return _return(response);
}
}
;
}
;

#include OATPP_CODEGEN_END(ApiController)  //<-- codegen end

#endif /* LITHIUM_ASYNC_WEBSOCKET_CONTROLLER_HPP */
