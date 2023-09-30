#ifndef Lithium_DeviceCONTROLLER_HPP
#define Lithium_DeviceCONTROLLER_HPP

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "LithiumApp.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class DeviceController : public oatpp::web::server::api::ApiController
{
public:
    DeviceController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<DeviceController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<DeviceController>(objectMapper);
    }

public:

    ENDPOINT_INFO(getUIINDIServerStatus)
    {
        info->summary = "Create a directory with specific path";
        info->addResponse<Oatpp::String>(Status::CODE_200, "application/json");
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "/api/device/indi/server/status", getUIINDIServerStatus)
    {
        ENDPOINT_ASYNC_INIT(getUIINDIServerStatus)
        Action act() override
        {
            
            auto res = StatusDto::createShared();
            if(body->path.getValue("") == "")
            {
                res->error = "Invalid Parameters";
                res->message = "Directory path is required";
            }
            else
            {
                auto path = body->path.getValue("");
                if(!Lithium::File::is_full_path(path))
                {
                    res->error = "Invalid Parameters";
                    res->message = "Directory path must be a absolute path";
                }
                else
                {
                    if(!Lithium::File::create_directory(path))
                    {
                        res->error = "IO Failed";
                        res->message = "Failed to create directory";
                    }
                }
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
#else

#endif
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_DeviceCONTROLLER_HPP