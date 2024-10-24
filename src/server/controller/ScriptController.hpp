#ifndef SCRIPTCONTROLLER_HPP
#define SCRIPTCONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include "data/ScriptDto.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/system/env.hpp"

#include "atom/macro.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class ScriptController : public oatpp::web::server::api::ApiController {
    using ControllerType = ScriptController;

public:
    ScriptController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                     objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    static auto createShared() -> std::shared_ptr<ScriptController> {
        return std::make_shared<ScriptController>();
    }

    ENDPOINT_INFO(getUIApiScriptEnv) {
        info->summary = "Get Environment Variables";
        info->addConsumes<Object<RequestDto>>("application/json");
        info->addResponse<Object<StatusDto>>(
            Status::CODE_300, "application/json",
            "INDI library version is lower than 2.0.0");
        info->addResponse<Object<StatusDto>>(Status::CODE_500,
                                             "application/json",
                                             "INDI server is not installed");
    }
    ENDPOINT_ASYNC("GET", "/api/script/env", getUIApiScriptEnv) {
        ENDPOINT_ASYNC_INIT(getUIApiScriptEnv);

        static constexpr auto COMMAND = "lithium.script.env";  // Command name
    private:
        auto createErrorResponse(const std::string& message, Status status) {
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "error";
            res->error = message;
            return controller->createDtoResponse(status, res);
        }

        auto createWarningResponse(const std::string& message, Status status) {
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "warning";
            res->warning = message;
            return controller->createDtoResponse(status, res);
        }

    public:
        auto act() -> Action override {
            try {
                auto env = atom::utils::Env::Environ();

                auto res = ReturnScriptEnvDto::createShared();
                res->code = 200;
                res->status = "success";
                res->message = "Get script environment successfully";

                for (const auto& [key, value] : env) {
                    res->env[key] = value;
                }

                return _return(
                    controller->createDtoResponse(Status::CODE_200, res));

            } catch (const std::exception& e) {
                return _return(createErrorResponse(e.what(), Status::CODE_500));
            }
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // SCRIPTCONTROLLER_HPP
