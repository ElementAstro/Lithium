#ifndef Lithium_STARCONTROLLER_HPP
#define Lithium_STARCONTROLLER_HPP

#include "service/StarService.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "config.h"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class StarController : public oatpp::web::server::api::ApiController
{
public:
    StarController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<StarController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper) // Inject objectMapper component here as default parameter
    )
    {
        return std::make_shared<StarController>(objectMapper);
    }

#if ENABLE_ASYNC
#else
#endif
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_STARCONTROLLER_HPP