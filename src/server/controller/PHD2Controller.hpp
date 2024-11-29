#ifndef PHD2CONTROLLER_HPP
#define PHD2CONTROLLER_HPP

#include "Types.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include "ControllerCheck.hpp"

#include "data/PHD2Dto.hpp"

#include "client/phd2/profile.hpp"

#include "config/configor.hpp"

#include "atom/async/async.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/env.hpp"
#include "atom/system/process.hpp"
#include "atom/system/process_manager.hpp"
#include "atom/system/software.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/random.hpp"
#include "atom/utils/string.hpp"

#include "utils/constant.hpp"

#include <fstream>
#include <string>

namespace lithium::controller::phd2 {

class AsyncEndpointHelper {
protected:
    static auto createErrorResponse(
        const std::string& command, const std::string& message,
        oatpp::web::server::api::ApiController* controller,
        oatpp::web::protocol::http::Status status =
            oatpp::web::protocol::http::Status::CODE_500) {
        LOG_F(ERROR, "Command '{}' failed with error: {}", command, message);
        auto res = StatusDto::createShared();
        res->command = command;
        res->status = "error";
        res->error = message;
        return controller->createDtoResponse(status, res);
    }

    static auto createWarningResponse(
        const std::string& command, const std::string& message,
        oatpp::web::server::api::ApiController* controller,
        oatpp::web::protocol::http::Status status =
            oatpp::web::protocol::http::Status::CODE_400) {
        LOG_F(WARNING, "Command '{}' encountered a warning: {}", command,
              message);
        auto res = StatusDto::createShared();
        res->command = command;
        res->status = "warning";
        res->warning = message;
        return controller->createDtoResponse(status, res);
    }

    static auto createSuccessResponse(
        const std::string& command,
        oatpp::web::server::api::ApiController* controller) {
        LOG_F(INFO, "Command '{}' executed successfully", command);
        auto res = StatusDto::createShared();
        res->command = command;
        res->status = "success";
        return controller->createDtoResponse(
            oatpp::web::protocol::http::Status::CODE_200, res);
    }

    static void updatePHD2RunningStatus(bool isRunning) {
        if (configManagerPtr) {
            configManagerPtr->setValue("/lithium/client/phd2/running",
                                       isRunning);
            LOG_F(INFO, "PHD2 running status updated to: {}",
                  isRunning ? "true" : "false");
        } else {
            THROW_BAD_CONFIG_EXCEPTION("ConfigManager is not initialized");
        }
    }

    static std::shared_ptr<lithium::ConfigManager> configManagerPtr;
    static std::shared_ptr<atom::system::ProcessManager> processManagerPtr;
    static std::shared_ptr<atom::utils::Env> envPtr;
};

std::shared_ptr<lithium::ConfigManager> AsyncEndpointHelper::configManagerPtr =
    nullptr;
std::shared_ptr<atom::system::ProcessManager>
    AsyncEndpointHelper::processManagerPtr = nullptr;
std::shared_ptr<atom::utils::Env> AsyncEndpointHelper::envPtr = nullptr;

}  // namespace lithium::controller::phd2

#include OATPP_CODEGEN_BEGIN(ApiController)  // <-- Begin Code-Gen

inline auto to_json(const oatpp::Vector<oatpp::String>& vec) {
    nlohmann::json j;
    for (int i = 0; i < vec->size(); ++i) {
        j.push_back(vec[i]->c_str());
    }
    return j;
}

class PHD2Controller
    : public oatpp::web::server::api::ApiController,
      protected lithium::controller::phd2::AsyncEndpointHelper {
public:
    PHD2Controller(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    static std::shared_ptr<PHD2Controller> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<PHD2Controller>(objectMapper);
    }

public:
    ENDPOINT_INFO(getUIApiPHD2Scan) {
        info->summary = "Scan PHD2 Server";
        info->addConsumes<RequestDto>("application/json");
        info->addResponse<ReturnPHD2ScanDto>(Status::CODE_200,
                                             "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/client/phd2/scan"_path, getUIApiPHD2Scan) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2Scan)

        static constexpr auto COMMAND = "lithium.client.phd2.scan";

        Action act() override {
            auto res = ReturnPHD2ScanDto::createShared();
            try {
                if (atom::system::checkSoftwareInstalled("phd2")) {
                    auto phd2Dto = PHD2ExecutableDto::createShared();
                    LOG_F(INFO, "PHD2 is installed");
                    auto path = atom::system::getAppPath("phd2");
                    auto version = atom::system::getAppVersion("phd2");
                    auto permission = atom::system::getAppPermissions("phd2");
                    phd2Dto->executable = path.string();
                    phd2Dto->version = version;
                    for (const auto& perm : permission) {
                        phd2Dto->permission->emplace_back(perm);
                    }
                    res->server->try_emplace("phd2", phd2Dto);
                } else {
                    // Attempt to find PHD2 in other paths
#if defined(_WIN32)
                    // Windows implementation
#else
#define SEARCH_PATHS(paths)                                             \
    do {                                                                \
        auto pathList = atom::io::searchExecutableFiles(paths, "phd2"); \
        for (const auto& path : pathList) {                             \
            auto phd2Dto = PHD2ExecutableDto::createShared();           \
            auto version = atom::system::getAppVersion(path.string());  \
            auto permission = atom::system::getAppPermissions(path);    \
            phd2Dto->executable = path.string();                        \
            phd2Dto->version = version;                                 \
            for (const auto& perm : permission) {                       \
                phd2Dto->permission->emplace_back(perm);                \
            }                                                           \
            res->server->try_emplace("phd2", phd2Dto);                  \
        }                                                               \
    } while (0)

                    SEARCH_PATHS("/usr/bin");
                    SEARCH_PATHS("/usr/local/bin");
                    SEARCH_PATHS("/opt");
#undef SEARCH_PATHS
#endif
                }

                // Save PHD2 server configuration to config manager
                GET_OR_CREATE_PTR(configManagerPtr, lithium::ConfigManager,
                                  Constants::CONFIG_MANAGER)
                json j;
                for (const auto& it : *res->server) {
                    j[it.first] = {
                        {"name", atom::utils::generateRandomString(5)},
                        {"executable", it.second.executable},
                        {"version", it.second.version},
                        {"permission", to_json(it.second.permission)}};
                }
                configManagerPtr->appendValue("/lithium/client/phd2/servers",
                                              j);
            } catch (const std::exception& e) {
                LOG_F(ERROR, "getUIApiPHD2Scan: Exception occurred: {}",
                      e.what());
                return _return(createErrorResponse(
                    COMMAND, "Failed to scan PHD2", controller));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIApiPHD2Configs) {
        info->summary = "Get PHD2 Configurations";
        info->description =
            "Retrieve PHD2 server configurations from the specified directory";
        info->addConsumes<RequestPHD2ConfigDto>("application/json");
        info->addResponse<ReturnPHD2ConfigDto>(Status::CODE_200,
                                               "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/client/phd2/configs"_path,
                   getUIApiPHD2Configs) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2Configs)

        static constexpr auto COMMAND = "lithium.client.phd2.configs";

        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestPHD2ConfigDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiPHD2Configs::onRequestDtoParsed);
        }

        Action onRequestDtoParsed(
            const oatpp::Object<RequestPHD2ConfigDto>& body) {
            auto path = body->path;
            if (!atom::io::isFolderNameValid(path) ||
                !atom::io::isFolderExists(path)) {
                return _return(createWarningResponse(
                    COMMAND, "Invalid or non-existent path specified",
                    controller, Status::CODE_400));
            }
            auto res = ReturnPHD2ConfigDto::createShared();
            try {
#if defined(_WIN32)
                // Windows implementation
#else
                auto configPaths = atom::io::checkFileTypeInFolder(
                    path, {".phd2", ".sodium", ".ini"},
                    atom::io::FileOption::PATH);
                if (configPaths.empty()) {
                    return _return(createWarningResponse(
                        COMMAND, "No PHD2 configurations found", controller,
                        Status::CODE_404));
                }
                // TODO: Process configuration files and populate res
#endif
            } catch (const std::exception& e) {
                LOG_F(ERROR, "getUIApiPHD2Configs: Exception occurred: {}",
                      e.what());
                return _return(createErrorResponse(
                    COMMAND, "Failed to retrieve PHD2 configurations",
                    controller));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIApiPHD2IsRunning) {
        info->summary = "Check if PHD2 Server is Running";
        info->addConsumes<RequestDto>("application/json");
        info->addResponse<StatusDto>(Status::CODE_200, "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/client/phd2/isrunning"_path,
                   getUIApiPHD2IsRunning) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2IsRunning)

        static constexpr auto COMMAND = "lithium.client.phd2.isrunning";

        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiPHD2IsRunning::onRequestDtoParsed);
        }

        Action onRequestDtoParsed(const oatpp::Object<RequestDto>& body) {
            auto retry = body->retry;
            auto timeout = body->timeout;

            if (retry < 0 || retry > 5 || timeout < 0 || timeout > 300) {
                return _return(
                    createWarningResponse(COMMAND, "Invalid parameters",
                                          controller, Status::CODE_400));
            }

            try {
                auto isRunning = atom::system::isProcessRunning("phd2");
                updatePHD2RunningStatus(isRunning);
                if (isRunning) {
                    return _return(createSuccessResponse(COMMAND, controller));
                }                      return _return(
                        createWarningResponse(COMMAND, "PHD2 is not running",
                                              controller, Status::CODE_404));
               
            } catch (const std::exception& e) {
                LOG_F(ERROR, "getUIApiPHD2IsRunning: Exception occurred: {}",
                      e.what());
                return _return(createErrorResponse(
                    COMMAND, "Failed to check PHD2 status", controller));
            }
        }
    };

    ENDPOINT_INFO(getUIApiPHD2Start) {
        info->summary = "Start PHD2 Server";
        info->addConsumes<RequestPHD2StartDto>("application/json");
        info->addResponse<StatusDto>(Status::CODE_200, "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/client/phd2/start"_path, getUIApiPHD2Start) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2Start)

        static constexpr auto COMMAND = "lithium.client.phd2.start";

        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestPHD2StartDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiPHD2Start::onRequestDtoParsed);
        }

        Action onRequestDtoParsed(
            const oatpp::Object<RequestPHD2StartDto>& body) {
            if (configManagerPtr) {
                auto value =
                    configManagerPtr->getValue("/lithium/client/phd2/running");
                if (value.has_value() && value.value().get<bool>()) {
                    return _return(createWarningResponse(
                        COMMAND, "PHD2 is already running", controller,
                        Status::CODE_400));
                }
            } else {
                THROW_BAD_CONFIG_EXCEPTION("ConfigManager is not initialized");
            }

            auto name = body->name;
            auto args = body->args;
            auto envVars = body->env;

            try {
                auto serverList =
                    configManagerPtr->getValue("/lithium/client/phd2/servers");
                if (!serverList.has_value()) {
                    return _return(
                        createWarningResponse(COMMAND, "No PHD2 servers found",
                                              controller, Status::CODE_404));
                }
                auto servers = serverList.value();
                if (!servers.is_array()) {
                    return _return(createErrorResponse(
                        COMMAND, "Invalid PHD2 server configuration",
                        controller));
                }
                bool found = false;
                for (const auto& server : servers) {
                    if (server["name"] == name) {
                        auto path = server["executable"].get<std::string>();
                        if (path.empty() || !atom::io::isFileNameValid(path)) {
                            return _return(createErrorResponse(
                                COMMAND, "Invalid PHD2 executable path",
                                controller));
                        }

                        GET_OR_CREATE_PTR(envPtr, atom::utils::Env,
                                          Constants::ENVIRONMENT)
                        for (const auto& [key, value] : *envVars) {
                            envPtr->setEnv(key, value);
                        }

                        GET_OR_CREATE_PTR(processManagerPtr,
                                          atom::system::ProcessManager,
                                          Constants::PROCESS_MANAGER)
                        if (!processManagerPtr->createProcess(path, "phd2",
                                                              true)) {
                            return _return(createErrorResponse(
                                COMMAND, "Failed to start PHD2", controller));
                        }
                        updatePHD2RunningStatus(true);
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    return _return(createWarningResponse(
                        COMMAND, "Specified PHD2 server not found", controller,
                        Status::CODE_404));
                }
            } catch (const std::exception& e) {
                LOG_F(ERROR, "getUIApiPHD2Start: Exception occurred: {}",
                      e.what());
                return _return(createErrorResponse(
                    COMMAND, "Failed to start PHD2", controller));
            }
            return _return(createSuccessResponse(COMMAND, controller));
        }
    };

    ENDPOINT_INFO(getUIApiPHD2Stop) {
        info->summary = "Stop PHD2 Server";
        info->addConsumes<RequestDto>("application/json");
        info->addResponse<StatusDto>(Status::CODE_200, "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/client/phd2/stop"_path, getUIApiPHD2Stop) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2Stop)

        static constexpr auto COMMAND = "lithium.client.phd2.stop";

        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiPHD2Stop::onRequestDtoParsed);
        }

        Action onRequestDtoParsed(const oatpp::Object<RequestDto>& body) {
            if (configManagerPtr) {
                auto value =
                    configManagerPtr->getValue("/lithium/client/phd2/running");
                if (value.has_value() && !value.value().get<bool>()) {
                    return _return(
                        createWarningResponse(COMMAND, "PHD2 is not running",
                                              controller, Status::CODE_400));
                }
            } else {
                THROW_BAD_CONFIG_EXCEPTION("ConfigManager is not initialized");
            }

            try {
                GET_OR_CREATE_PTR(processManagerPtr,
                                  atom::system::ProcessManager,
                                  Constants::PROCESS_MANAGER)
                if (!processManagerPtr->terminateProcessByName("phd2")) {
                    return _return(createErrorResponse(
                        COMMAND, "Failed to stop PHD2", controller));
                }
                updatePHD2RunningStatus(false);
            } catch (const std::exception& e) {
                LOG_F(ERROR, "getUIApiPHD2Stop: Exception occurred: {}",
                      e.what());
                return _return(createErrorResponse(
                    COMMAND, "Failed to stop PHD2", controller));
            }
            return _return(createSuccessResponse(COMMAND, controller));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  // <-- End Code-Gen

#endif /* PHD2CONTROLLER_HPP */