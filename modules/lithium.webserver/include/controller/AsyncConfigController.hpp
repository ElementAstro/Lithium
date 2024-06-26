/*
 * AsyncConfigController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-23

Description: Async Config Controller

**************************************************/

#ifndef LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP
#define LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "data/ConfigDto.hpp"
#include "data/StatusDto.hpp"

#include "atom/server/global_ptr.hpp"
#include "config/configor.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class ConfigController : public oatpp::web::server::api::ApiController {
public:
    static std::weak_ptr<lithium::ConfigManager> m_configManager;

    ConfigController(const std::shared_ptr<ObjectMapper>& objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper) {
            m_configManager = GetWeakPtr<lithium::ConfigManager>("lithium.config");
        }

    // ----------------------------------------------------------------
    // Pointer creator
    // ----------------------------------------------------------------

    static std::shared_ptr<ConfigController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<ConfigController>(objectMapper);
    }

    // ----------------------------------------------------------------
    // Config Http Handler
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUIGetConfig) {
        info->summary = "Get config from global ConfigManager (thread safe)";
        info->addConsumes<Object<GetConfigDTO>>("application/json");
        info->addResponse<Object<ReturnConfigDTO>>(Status::CODE_200,
                                                   "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/get", getUIGetConfig) {
        ENDPOINT_ASYNC_INIT(getUIGetConfig);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<GetConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIGetConfig::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetConfigDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");

            auto res = ReturnConfigDTO::createShared();
            res->status = "getConfig";
            if (m_configManager.expired()) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else {
                std::string path = body->path.getValue("");
                if (auto tmp = m_configManager.lock()->getValue(path); tmp) {
                    res->status = "success";
                    res->code = 200;
                    res->value = tmp.value().dump();
                    res->type = "string";
                } else {
                    res->status = "error";
                    res->code = 404;
                    res->error = "ConfigManager can't find the path";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUISetConfig) {
        info->summary = "Set config to global ConfigManager (thread safe)";
        info->addConsumes<Object<SetConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/set", getUISetConfig) {
        ENDPOINT_ASYNC_INIT(getUISetConfig);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<SetConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUISetConfig::returnResponse);
        }

        Action returnResponse(const oatpp::Object<SetConfigDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");
            OATPP_ASSERT_HTTP(body->value.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "setConfig";

            if (m_configManager.expired()) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else {
                std::string path = body->path.getValue("");
                std::string value = body->value.getValue("");
                if (m_configManager.lock()->setValue(path, value)) {
                    res->status = "success";
                    res->code = 200;
                } else {
                    res->status = "error";
                    res->code = 404;
                    res->error = "Failed to set the value";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIDeleteConfig) {
        info->summary = "Delete config from global ConfigManager (thread safe)";
        info->addConsumes<Object<DeleteConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/delete", getUIDeleteConfig) {
        ENDPOINT_ASYNC_INIT(getUIDeleteConfig);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<DeleteConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIDeleteConfig::returnResponse);
        }

        Action returnResponse(const oatpp::Object<DeleteConfigDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "deleteConfig";

            if (m_configManager.expired()) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else {
                std::string path = body->path.getValue("");
                if (m_configManager.lock()->deleteValue(path)) {
                    res->status = "success";
                    res->code = 200;
                } else {
                    res->status = "error";
                    res->code = 404;
                    res->error = "ConfigManager can't find the path";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUILoadConfig) {
        info->summary =
            "Load config from file, this will be merged into the main config";
        info->addConsumes<Object<LoadConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/load", getUILoadConfig) {
        ENDPOINT_ASYNC_INIT(getUILoadConfig);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<LoadConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUILoadConfig::returnResponse);
        }

        Action returnResponse(const oatpp::Object<LoadConfigDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "loadConfig";

            if (m_configManager.expired()) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else {
                std::string path = body->path.getValue("");
                if (m_configManager.lock()->loadFromFile(path)) {
                    res->status = "success";
                    res->code = 200;
                } else {
                    res->status = "error";
                    res->code = 404;
                    res->error = "ConfigManager can't find the path";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUISaveConfig) {
        info->summary =
            "Save config to file (this will auto load the config after saving)";
        info->addConsumes<Object<SaveConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/config/save", getUISaveConfig) {
        ENDPOINT_ASYNC_INIT(getUISaveConfig);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<SaveConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUISaveConfig::returnResponse);
        }
        Action returnResponse(const oatpp::Object<SaveConfigDTO>& body) {
            OATPP_ASSERT_HTTP(body->path.getValue("") != "", Status::CODE_400,
                              "Missing Parameters");
            OATPP_ASSERT_HTTP(body->isAbsolute.getValue(true) != true,
                              Status::CODE_400, "Missing Parameters");

            auto res = StatusDto::createShared();
            res->command = "saveConfig";
            if (m_configManager.expired()) {
                res->status = "error";
                res->code = 500;
                res->error = "ConfigManager is null";
            } else {
                std::string path = body->path.getValue("");
                bool isAbsolute = body->isAbsolute.getValue(true);
                if (m_configManager.lock()->saveToFile(path)) {
                    res->status = "success";
                    res->code = 200;
                } else {
                    res->status = "error";
                    res->code = 404;
                    res->error = "Failed to save the config";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };
};

std::weak_ptr<lithium::ConfigManager> ConfigController::m_configManager = {};

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP
