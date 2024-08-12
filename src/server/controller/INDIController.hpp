#ifndef INDIController_hpp
#define INDIController_hpp

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class INDIController : public oatpp::web::server::api::ApiController {
private:
    using __ControllerType = INDIController;

public:
    INDIController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                     objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // INDIController_hpp
