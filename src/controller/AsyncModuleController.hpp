#ifndef Lithium_MODULECONTROLLER_HPP
#define Lithium_MODULECONTROLLER_HPP

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "LithiumApp.hpp"
#include "data/ModuleDto.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class ModuleController : public oatpp::web::server::api::ApiController
{
public:
    ModuleController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<ModuleController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<ModuleController>(objectMapper);
    }

public:

    ENDPOINT_INFO(getUILoadModule)
    {
        info->summary = "Load a plugin module from the specified path";
        info->addConsumes<Object<LoadPluginDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/module/load", getUILoadModule)
    {
        ENDPOINT_ASYNC_INIT(getUILoadModule)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<LoadPluginDto>>(controller->getDefaultObjectMapper()).callbackTo(&getUILoadModule::returnResponse);
        }

        Action returnResponse(const oatpp::Object<LoadPluginDto>& body)
        {
            auto res = StatusDto::createShared();
            if(body->plugin_path.getValue("") == "" || body->plugin_name.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "Device plugin path and name is required";
            }
            else
            {
                auto plugin_path = body->plugin_path.getValue("");
                auto plugin_name = body->plugin_name.getValue("");
                if (!Lithium::MyApp->LoadModule(plugin_path, plugin_name))
                {
                    res->error = "DeviceError";
                    res->message = "Failed to add device plugin";
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIUnloadModule)
    {
        info->summary = "Unload module by name";
    }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_MODULECONTROLLER_HPP