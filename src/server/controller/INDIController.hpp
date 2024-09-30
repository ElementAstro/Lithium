#ifndef INDICONTROLLER_HPP
#define INDICONTROLLER_HPP

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include "data/INDIDto.hpp"

#include "atom/sysinfo/os.hpp"
#include "atom/system/process.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class INDIController : public oatpp::web::server::api::ApiController {
    using ControllerType = INDIController;

public:
    INDIController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    ENDPOINT_INFO(getUIApiServreINDIScan) {
        info->summary = "Scan INDI server";
        info->addConsumes<Object<RequestDto>>("application/json");
        info->addResponse<Object<ReturnServerINDIScanDto>>(Status::CODE_200,
                                                           "application/json");
        info->addResponse<Object<ReturnServerINDIScanMultiInstancesDto>>(
            Status::CODE_402, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/server/indi/scan", getUIApiServreINDIScan) {
        ENDPOINT_ASYNC_INIT(getUIApiServreINDIScan);

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiServreINDIScan::returnResponse);
        }

        auto returnResponse(const oatpp::Object<RequestDto>& body) -> Action {
            auto res = StatusDto::createShared();
            res->command = "lithium.server.starter.indi.scan";

            // Check if the system is WSL, if so, return a warning
            if (atom::system::isWsl()) {
                res->status = "warning";
                res->warning =
                    "WSL detected, INDI server can not perform all "
                    "operations, please use a real Linux system";
                return _return(
                    controller->createDtoResponse(Status::CODE_300, res));
            }
            if (const auto OS_INFO = atom::system::getOperatingSystemInfo(); OS_INFO.osName.starts_with("Windows")) {
                res->status = "error";
                res->error =
                    "Windows detected, INDI server cannot currently run on Windows";
                return _return(
                    controller->createDtoResponse(Status::CODE_301, res));
            }

            // Get all instances of INDI server
            auto instances = atom::system::getProcessIdByName("indiserver");
            if (instances.size() > 1) {
            
                auto multiInstances = ReturnServerINDIScanMultiInstancesDto::createShared();
                for (const auto& pid : instances) {
                    auto instance = MultiInstancesDto::createShared();
                    instance->pid = pid;
                    instance->path = "/usr/bin/indiserver";
                    instance->version = "1.8.8";
                    instance->name = "INDI Server";
                    instance->port = 7624;
                    instance->canKill = true;
                    multiInstances->instance->push_back(instance);
                }
                return _return(
                    controller->createDtoResponse(Status::CODE_200, multiInstances));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // INDICONTROLLER_HPP
