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
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_MODULECONTROLLER_HPP