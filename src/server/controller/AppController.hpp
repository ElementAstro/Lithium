#ifndef SCRIPTCONTROLLER_HPP
#define SCRIPTCONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include "data/ScriptDto.hpp"

#include "atom/function/global_ptr.hpp"

#include "atom/macro.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class AppController : public oatpp::web::server::api::ApiController {
    using ControllerType = AppController;

public:
    AppController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                     objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    static auto createShared() -> std::shared_ptr<AppController> {
        return std::make_shared<AppController>();
    }

};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // SCRIPTCONTROLLER_HPP
