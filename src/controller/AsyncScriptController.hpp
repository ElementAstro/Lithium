#ifndef Lithium_SCRIPTCONTROLLER_HPP
#define Lithium_SCRIPTCONTROLLER_HPP

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "LithiumApp.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class ScriptController : public oatpp::web::server::api::ApiController
{
public:
    ScriptController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<ScriptController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<ScriptController>(objectMapper);
    }

public:
    // ----------------------------------------------------------------
    // Script Http Handler
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUIRunScript)
    {
        info->summary = "Run a single line script and get the result";
        info->addConsumes<Object<RunScriptDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/script/run", getUIRunScript)
    {
        ENDPOINT_ASYNC_INIT(getUIRunScript)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RunScriptDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIRunScript::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RunScriptDTO>& body)
        {
            auto res = StatusDto::createShared();
            if(body->device_name.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            }
            else
            {
                auto device_name = body->device_name.getValue("");
                if (!Lithium::MyApp->removeDeviceByName(device_name))
                {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_SCRIPTCONTROLLER_HPP