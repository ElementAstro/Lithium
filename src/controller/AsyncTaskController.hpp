#ifndef Lithium_TASKCONTROLLER_HPP
#define Lithium_TASKCONTROLLER_HPP

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "LithiumApp.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class TaskController : public oatpp::web::server::api::ApiController
{
public:
    TaskController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<TaskController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<TaskController>(objectMapper);
    }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_TASKCONTROLLER_HPP