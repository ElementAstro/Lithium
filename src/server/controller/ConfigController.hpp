/*
 * AsyncConfigController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP
#define LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP

#include "config.h"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/type/json.hpp"
#include "config/configor.hpp"
#include "data/ConfigDto.hpp"
#include "data/RequestDto.hpp"
#include "data/StatusDto.hpp"

#include "atom/log/loguru.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class ConfigController : public oatpp::web::server::api::ApiController {
private:
    static std::weak_ptr<lithium::ConfigManager> mConfigManager;

    template <typename DtoType, typename Func>
    static auto handleConfigAction(auto controller,
                                   const oatpp::Object<DtoType>& body,
                                   const std::string& command, Func func) {
        if constexpr (!std::is_same_v<DtoType, RequestDto>) {
            OATPP_ASSERT_HTTP(
                !body->path->empty(), Status::CODE_400,
                "The 'path' parameter is required and cannot be empty.");
        }
        auto res = StatusDto::createShared();
        res->command = command;

        try {
            auto configManager = mConfigManager.lock();
            if (!configManager) {
                res->status = "error";
                res->code = Status::CODE_500.code;
                res->error =
                    "Internal Server Error: ConfigManager instance is null.";
                LOG_F(ERROR,
                      "ConfigManager instance is null. Unable to proceed with "
                      "the command: {}",
                      command);
            } else {
                auto success = func(configManager);
                if (success) {
                    res->status = "success";
                    res->code = Status::CODE_200.code;
                    if constexpr (std::is_same_v<DtoType, RequestDto>) {
                        LOG_F(INFO, "Successfully executed command: {}",
                              command);
                    } else {
                        LOG_F(INFO,
                              "Successfully executed command: {} for path: {}",
                              command, *body->path);
                    }

                } else {
                    res->status = "error";
                    res->code = Status::CODE_404.code;
                    res->error =
                        "Not Found: The specified path could not be found or "
                        "the operation failed.";
                    if constexpr (std::is_same_v<DtoType, RequestDto>) {
                        LOG_F(WARNING, "Failed to execute command: {}",
                              command);
                    } else {
                        LOG_F(WARNING,
                              "Failed to execute command: {} for path: {}",
                              command, *body->path);
                    }
                }
            }
        } catch (const std::exception& e) {
            res->status = "error";
            res->code = Status::CODE_500.code;
            res->error =
                std::string("Internal Server Error: Exception occurred - ") +
                e.what();
            LOG_F(
                ERROR,
                "Exception occurred while executing command: {}. Exception: {}",
                command, e.what());
        }

        return controller->createDtoResponse(Status::CODE_200, res);
    }

public:
    explicit ConfigController(const std::shared_ptr<ObjectMapper>& objectMapper)
        : ApiController(objectMapper) {
        GET_OR_CREATE_WEAK_PTR(mConfigManager, lithium::ConfigManager,
                               Constants::CONFIG_MANAGER);
    }

    static auto createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                             objectMapper))
        -> std::shared_ptr<ConfigController> {
        return std::make_shared<ConfigController>(objectMapper);
    }

    ENDPOINT_INFO(getUIGetConfig) {
        info->summary = "Get config from ConfigManager";
        info->addConsumes<Object<GetConfigDTO>>("application/json");
        info->addResponse<Object<ReturnConfigDTO>>(Status::CODE_200,
                                                   "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/get", getUIGetConfig) {
        ENDPOINT_ASYNC_INIT(getUIGetConfig);

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<GetConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIGetConfig::returnResponse);
        }

        auto returnResponse(const oatpp::Object<GetConfigDTO>& body) -> Action {
            return _return(handleConfigAction(
                this->controller, body, "getConfig", [&](auto configManager) {
                    if (auto tmp = configManager->getValue(body->path)) {
                        auto res = ReturnConfigDTO::createShared();
                        res->status = "success";
                        res->code = Status::CODE_200.code;
                        res->value = tmp.value().dump();
                        res->type = "string";
                        return true;
                    }
                    return false;
                }));
        }
    };

    ENDPOINT_INFO(getUISetConfig) {
        info->summary = "Set config to ConfigManager";
        info->addConsumes<Object<SetConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/set", getUISetConfig) {
        ENDPOINT_ASYNC_INIT(getUISetConfig);

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<SetConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUISetConfig::returnResponse);
        }

        auto returnResponse(const oatpp::Object<SetConfigDTO>& body) -> Action {
            OATPP_ASSERT_HTTP(!body->value->empty(), Status::CODE_400,
                              "Missing Parameters");

            return _return(handleConfigAction(
                this->controller, body, "setConfig", [&](auto configManager) {
                    return configManager->setValue(body->path, body->value);
                }));
        }
    };

    ENDPOINT_INFO(getUIDeleteConfig) {
        info->summary = "Delete config from ConfigManager";
        info->addConsumes<Object<DeleteConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/delete", getUIDeleteConfig) {
        ENDPOINT_ASYNC_INIT(getUIDeleteConfig);

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<DeleteConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIDeleteConfig::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<DeleteConfigDTO>& body) -> Action {
            return _return(handleConfigAction(
                this->controller, body, "deleteConfig",
                [&](auto configManager) {
                    return configManager->deleteValue(body->path);
                }));
        }
    };

    ENDPOINT_INFO(getUILoadConfig) {
        info->summary = "Load config from file";
        info->addConsumes<Object<LoadConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/load", getUILoadConfig) {
        ENDPOINT_ASYNC_INIT(getUILoadConfig);

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<LoadConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUILoadConfig::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<LoadConfigDTO>& body) -> Action {
            return _return(handleConfigAction(
                this->controller, body, "loadConfig", [&](auto configManager) {
                    return configManager->loadFromFile(
                        body->path.getValue("config/config.json"));
                }));
        }
    };

    // Endpoint to reload configuration from file
    ENDPOINT_INFO(getUIReloadConfig) {
        info->summary = "Reload config from file";
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/config/reload", getUIReloadConfig) {
        ENDPOINT_ASYNC_INIT(getUIReloadConfig);

        auto act() -> Action override {
            return _return(handleConfigAction<RequestDto>(
                this->controller, {}, "reloadConfig", [&](auto configManager) {
                    return configManager->loadFromFile("config/config.json");
                }));
        }
    };

    ENDPOINT_INFO(getUISaveConfig) {
        info->summary = "Save config to file";
        info->addConsumes<Object<SaveConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/save", getUISaveConfig) {
        ENDPOINT_ASYNC_INIT(getUISaveConfig);

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<SaveConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUISaveConfig::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<SaveConfigDTO>& body) -> Action {
            return _return(handleConfigAction(
                this->controller, body, "saveConfig", [&](auto configManager) {
                    return configManager->saveToFile(
                        body->path.getValue("config/config.json"));
                }));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP
