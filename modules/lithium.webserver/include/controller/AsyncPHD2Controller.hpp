/*
 * AsyncPHD2Controller.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: PHD2 Route

**************************************************/

#ifndef ASYNC_PHD2_CONTROLLER_HPP
#define ASYNC_PHD2_CONTROLLER_HPP

#include "lithiumapp.hpp"

#include "data/PHD2Dto.hpp"
#include "data/StatusDto.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "atom/type/json.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class PHD2Controller : public oatpp::web::server::api::ApiController {
public:
    PHD2Controller(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper) {}

public:
    static std::shared_ptr<PHD2Controller> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<PHD2Controller>(objectMapper);
    }

    ENDPOINT_INFO(getUIStartPHD2API) {
        info->summary = "Start PHD2 with some parameters";
        info->addConsumes<Object<StartPHD2DTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_400, "text/json");
    }
    ENDPOINT_ASYNC("GET", "/api/phd2/start", getUIStartPHD2API){

        ENDPOINT_ASYNC_INIT(getUIStartPHD2API)

            Action act()
                override{return request
                             ->readBodyToDtoAsync<oatpp::Object<StartPHD2DTO>>(
                                 controller->getDefaultObjectMapper())
                             .callbackTo(&getUIStartPHD2API::returnResponse);
}

Action
returnResponse(const oatpp::Object<StartPHD2DTO> &body) {
    auto res = StatusDto::createShared();
    res->command = "StartPHD2";
    res->code = 200;
    auto params = body->phd2_params.getValue("");
    if (params != "") {
        try {
            nlohmann::json params_ = nlohmann::json::parse(params);
            if (!lithium::MyApp->createProcess("phd2", "phd2")) {
                res->error = "Process Failed";
                res->message = "Failed to start PHD2";
            }
        } catch (const nlohmann::json::parse_error &e) {
            res->error = "Invalid Parameters";
            res->message = "Failed to parse PHD2 parameters";
        } catch (const std::exception &e) {
            res->error = "PHD2 Failed";
            res->message = "Unknown error happen when starting PHD2";
        }
    }
    return _return(controller->createDtoResponse(Status::CODE_200, res));
}
}
;

ENDPOINT_INFO(getUIStopPHD2ParamAPI) {
    info->summary = "Stop PHD2";
    info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
}
ENDPOINT_ASYNC("GET", "/api/phd2/stop", getUIStopPHD2ParamAPI){

    ENDPOINT_ASYNC_INIT(getUIStopPHD2ParamAPI)

        Action act() override{auto res = StatusDto::createShared();
res->command = "StopPHD2";
if (!lithium::MyApp->terminateProcessByName("phd2")) {
    res->error = "Process Failed";
    res->message = "Failed to stop PHD2";
}
return _return(controller->createDtoResponse(Status::CODE_200, res));
}
}
;

ENDPOINT_INFO(getUIModifyPHD2ParamAPI) {
    info->summary = "Modify PHD2 Parameter with name and value";
    info->addConsumes<Object<ModifyPHD2ParamDTO>>("application/json");
    info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
}
ENDPOINT_ASYNC("GET", "/api/phd2/modify", getUIModifyPHD2ParamAPI){

    ENDPOINT_ASYNC_INIT(getUIModifyPHD2ParamAPI)

        Action act() override{
            return request
                ->readBodyToDtoAsync<oatpp::Object<ModifyPHD2ParamDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIModifyPHD2ParamAPI::returnResponse);
}

Action returnResponse(const oatpp::Object<ModifyPHD2ParamDTO> &body) {
    auto res = StatusDto::createShared();
    res->command = "ModifyPHD2Param";
    res->code = 200;
    auto param_name = body->param_name.getValue("");
    auto param_value = body->param_value.getValue("");
    // OATPP_ASSERT_HTTP(param_name && param_value, Status::CODE_400, "parameter
    // name and id should not be null");
    bool phd2_running = false;
    for (auto process : lithium::MyApp->getRunningProcesses()) {
        if (process.name == "phd2") {
            phd2_running = true;
        }
    }
    if (phd2_running) {
        // PHD2参数热更新
    } else {
        // PHD2参数冷更新，主要是修改配置文件
    }
    return _return(controller->createDtoResponse(Status::CODE_200, res));
}
}
;
}
;

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // ASYNC_PHD2_CONTROLLER_HPP
