#ifndef Lithium_PHD2CONTROLLER_HPP
#define Lithium_PHD2CONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "nlohmann/json.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class PHD2Controller : public oatpp::web::server::api::ApiController
{
public:
    PHD2Controller(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<PHD2Controller> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<PHD2Controller>(objectMapper);
    }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_PHD2CONTROLLER_HPP