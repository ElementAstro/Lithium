/*
 * AsyncServerController.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Upload Route

**************************************************/

// NOTE: 2024-3-9 Max: The 'ServerController' means INDI/Hydrogen and ASCOM
// Remote Server. All of the API is same as LightAPT Server

#ifndef LITHIUM_ASYNC_SERVER_CONTROLLER_HPP
#define LITHIUM_ASYNC_SERVER_CONTROLLER_HPP

#include "config.h"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

namespace multipart = oatpp::web::mime::multipart;

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class ServerController : public oatpp::web::server::api::ApiController {
public:
    ServerController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper) {}

    // ----------------------------------------------------------------
    // Pointer creator
    // ----------------------------------------------------------------

    static std::shared_ptr<ServerController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<ServerController>(objectMapper);
    }

    // ----------------------------------------------------------------
    // Server Http Handler
    // ----------------------------------------------------------------
};

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_SERVER_CONTROLLER_HPP