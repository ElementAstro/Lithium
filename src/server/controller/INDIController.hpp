#ifndef INDICONTROLLER_HPP
#define INDICONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class INDIController : public oatpp::web::server::api::ApiController {
    using ControllerType = INDIController;

public:
    INDIController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    ENDPOINT_ASYNC("POST", "/api/", WS) {
        ENDPOINT_ASYNC_INIT(WS);

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<SaveConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUISaveConfig::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<SaveConfigDTO>& body) -> Action {
            OATPP_ASSERT_HTTP(!body->path->empty(), Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "saveConfig";
            if (!m_configManager) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else if (m_configManager->saveToFile(
                           body->path.getValue("config/config.json"))) {
                res->status = "success";
                res->code = 200;
            } else {
                res->status = "error";
                res->code = 404;
                res->error = "Failed to save the config";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // INDICONTROLLER_HPP
