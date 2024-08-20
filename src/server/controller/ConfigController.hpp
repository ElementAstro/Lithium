/*
 * AsyncConfigController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP
#define LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP

#include "config.h"
#include "oatpp/data/mapping/ObjectMapper.hpp"
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/type/json.hpp"
#include "config/configor.hpp"
#include "data/ConfigDto.hpp"
#include "data/StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class ConfigController : public oatpp::web::server::api::ApiController {
private:
    static inline std::shared_ptr<lithium::ConfigManager> m_configManager =
        GetPtr<lithium::ConfigManager>("lithium.config").value();

public:
    explicit ConfigController(const std::shared_ptr<ObjectMapper>& objectMapper)
        : ApiController(objectMapper) {}

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
            OATPP_ASSERT_HTTP(!body->path->empty(), Status::CODE_400,
                              "Missing Parameters");

            auto res = ReturnConfigDTO::createShared();
            res->status = "getConfig";
            if (!m_configManager) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else if (auto tmp = m_configManager->getValue(body->path)) {
                res->status = "success";
                res->code = 200;
                res->value = tmp.value().dump();
                res->type = "string";
            } else {
                res->status = "error";
                res->code = 404;
                res->error = "ConfigManager can't find the path";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
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
            OATPP_ASSERT_HTTP(!body->path->empty() && !body->value->empty(),
                              Status::CODE_400, "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "setConfig";
            if (!m_configManager) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else if (m_configManager->setValue(body->path, body->value)) {
                res->status = "success";
                res->code = 200;
            } else {
                res->status = "error";
                res->code = 404;
                res->error = "Failed to set the value";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
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
            OATPP_ASSERT_HTTP(!body->path->empty(), Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "deleteConfig";
            if (!m_configManager) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else if (m_configManager->deleteValue(body->path)) {
                res->status = "success";
                res->code = 200;
            } else {
                res->status = "error";
                res->code = 404;
                res->error = "ConfigManager can't find the path";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
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
            OATPP_ASSERT_HTTP(!body->path->empty(), Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "loadConfig";
            if (!m_configManager) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else if (m_configManager->loadFromFile(
                           body->path.getValue("config/config.json"))) {
                res->status = "success";
                res->code = 200;
            } else {
                res->status = "error";
                res->code = 404;
                res->error = "ConfigManager can't find the path";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
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

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP
