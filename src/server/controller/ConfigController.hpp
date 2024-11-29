/*
 * ConfigController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP
#define LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP

#include <type_traits>
#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "data/ConfigDto.hpp"
#include "data/RequestDto.hpp"
#include "data/StatusDto.hpp"

#include "config/configor.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/user.hpp"
#include "atom/type/json.hpp"

#include "utils/constant.hpp"
#include "web/protocol/http/Http.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class ConfigController : public oatpp::web::server::api::ApiController {
private:
    static std::shared_ptr<lithium::ConfigManager> mConfigManager;

    template <typename ResponseDtoType>
    static auto createErrorResponse(const std::string& command,
                                    const std::string& status, int code,
                                    const std::string& message,
                                    const std::string& error) {
        auto res = ResponseDtoType::createShared();
        res->command = command;
        res->status = status;
        res->code = code;
        res->message = message;
        res->error = error;
        return res;
    }

    template <typename DtoType, typename ResponseDtoType>
    static auto handleConfigManagerNull(auto controller,
                                        const std::string& command) {
        auto res = createErrorResponse<ResponseDtoType>(
            command, "error", Status::CODE_500.code,
            "ConfigManager instance is null.", "Internal Server Error");
        LOG_F(ERROR,
              "ConfigManager instance is null. Unable to proceed with the "
              "command: {}",
              command);
        return controller->createDtoResponse(Status::CODE_200, res);
    }

    template <typename DtoType, typename ResponseDtoType>
    static auto handlePathNotFound(auto controller,
                                   const std::string& command) {
        auto res = createErrorResponse<ResponseDtoType>(
            command, "error", Status::CODE_404.code,
            "The specified path could not be found or the operation failed.",
            "Path Not Found");
        return controller->createDtoResponse(Status::CODE_200, res);
    }

    template <typename DtoType, typename ResponseDtoType, typename Func>
    static auto handleSuccessOrFailure(auto controller,
                                       const oatpp::Object<DtoType>& body,
                                       const std::string& command, Func func) {
        auto res = ResponseDtoType::createShared();
        res->command = command;
        auto success = func(mConfigManager, res);
        res->status = success ? "success" : "error";
        res->code = success ? Status::CODE_200.code : Status::CODE_404.code;
        res->error = success ? ""
                             : "Not Found: The specified path could not be "
                               "found or the operation failed.";

        if constexpr (std::is_same_v<DtoType, RequestDto>) {
            if (success) {
                LOG_F(INFO, "Successfully executed command: {}", command);
            } else {
                LOG_F(WARNING, "Failed to execute command: {}", command);
            }

        } else {
            if (success) {
                LOG_F(INFO, "Successfully executed command: {} for path: {}",
                      command, *body->path);
            } else {
                LOG_F(WARNING, "Failed to execute command: {} for path: {}",
                      command, *body->path);
            }
        }

        return controller->createDtoResponse(Status::CODE_200, res);
    }

    template <typename DtoType, typename ResponseDtoType, typename Func>
    static auto handleConfigAction(auto controller,
                                   const oatpp::Object<DtoType>& body,
                                   const std::string& command, Func func) {
        try {
            if (!mConfigManager) {
                return handleConfigManagerNull<DtoType, ResponseDtoType>(
                    controller, command);
            }

            if constexpr (std::is_same_v<DtoType, GetConfigDTO> ||
                          std::is_same_v<DtoType, SetConfigDTO> ||
                          std::is_same_v<DtoType, HasConfigDTO> ||
                          std::is_same_v<DtoType, DeleteConfigDTO> ||
                          std::is_same_v<DtoType, SaveConfigDTO> ||
                          std::is_same_v<DtoType, LoadConfigDTO>) {
                return handlePathNotFound<DtoType, ResponseDtoType>(controller,
                                                                    command);
            }

            return handleSuccessOrFailure<DtoType, ResponseDtoType>(
                controller, body, command, func);

        } catch (const std::invalid_argument& e) {
            auto res = createErrorResponse<InvalidParametersDto>(
                command, "error", Status::CODE_400.code,
                "Bad Request: {}"_fmt(e.what()), "Invalid Parameters");
            LOG_F(ERROR,
                  "Invalid argument while executing command: {}. Exception: {}",
                  command, e.what());
            return controller->createDtoResponse(Status::CODE_200, res);
        } catch (const std::exception& e) {
            auto res = createErrorResponse<InternalServerErrorDto>(
                command, "error", Status::CODE_500.code,
                "Internal Server Error: {}"_fmt(e.what()),
                "Internal Server Error");
            LOG_F(
                ERROR,
                "Exception occurred while executing command: {}. Exception: {}",
                command, e.what());
            return controller->createDtoResponse(Status::CODE_200, res);
        } catch (...) {
            auto res = createErrorResponse<UnknownErrorDto>(
                command, "error", Status::CODE_500.code,
                "Unknown exception occurred.", "Unknown Error");
            LOG_F(ERROR,
                  "Unknown exception occurred while executing command: {}",
                  command);
            return controller->createDtoResponse(Status::CODE_200, res);
        }
    }

public:
    explicit ConfigController(const std::shared_ptr<ObjectMapper>& objectMapper)
        : ApiController(objectMapper) {
        GET_OR_CREATE_PTR(mConfigManager, lithium::ConfigManager,
                          Constants::CONFIG_MANAGER);
    }

    static auto createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                             objectMapper))
        -> std::shared_ptr<ConfigController> {
        return std::make_shared<ConfigController>(objectMapper);
    }

    ENDPOINT_INFO(getUIGetConfig) {
        info->summary = "Get config from ConfigManager";
        info->description =
            "Retrieves the configuration value from ConfigManager based on the "
            "provided path.";
        info->tags = {"Config"};
        info->addConsumes<Object<GetConfigDTO>>("application/json");
        info->addResponse<Object<ReturnGetConfigDTO>>(Status::CODE_200,
                                                      "application/json");
        info->addResponse<Object<PathNotFoundDto>>(Status::CODE_404,
                                                   "application/json");
        info->addResponse<Object<InternalServerErrorDto>>(Status::CODE_500,
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
            return _return(handleConfigAction<GetConfigDTO, ReturnGetConfigDTO>(
                this->controller, body, "lithium.config.get",
                [&](auto mConfigManager, auto res) {
                    if (auto tmp = mConfigManager->getValue(body->path)) {
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
        info->description = "Set config value to ConfigManager by path.";
        info->tags = {"Config"};
        info->addConsumes<Object<SetConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<PathNotFoundDto>>(Status::CODE_404,
                                                   "application/json");
        info->addResponse<Object<InvalidParametersDto>>(Status::CODE_400,
                                                        "application/json");
        info->addResponse<Object<InternalServerErrorDto>>(Status::CODE_500,
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
            OATPP_ASSERT_HTTP(!body->value->empty(), Status::CODE_400,
                              "Missing Parameters");

            return _return(handleConfigAction<SetConfigDTO, StatusDto>(
                this->controller, body, "lithium.config.set",
                [&](auto mConfigManager, auto res) {
                    return mConfigManager->setValue(body->path, body->value);
                }));
        }
    };

    ENDPOINT_INFO(getUIDeleteConfig) {
        info->summary = "Delete config from ConfigManager";
        info->description = "Delete config value from ConfigManager by path.";
        info->tags = {"Config"};
        info->addConsumes<Object<DeleteConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<PathNotFoundDto>>(Status::CODE_404,
                                                   "application/json");
        info->addResponse<Object<InternalServerErrorDto>>(Status::CODE_500,
                                                          "application/json");
        info->addResponse<Object<ForbiddenDto>>(Status::CODE_403,
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
            return _return(handleConfigAction<DeleteConfigDTO, StatusDto>(
                this->controller, body, "lithium.config.delete",
                [&](auto mConfigManager, auto res) {
                    return mConfigManager->deleteValue(body->path);
                }));
        }
    };

    ENDPOINT_INFO(getUIHasConfig) {
        info->summary = "Check if config exists in ConfigManager";
        info->description =
            "Check if the configuration value exists in ConfigManager based on "
            "the provided path.";
        info->tags = {"Config"};
        info->addConsumes<Object<HasConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<InternalServerErrorDto>>(Status::CODE_500,
                                                          "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/config/has", getUIHasConfig) {
        ENDPOINT_ASYNC_INIT(getUIHasConfig);

        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<HasConfigDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIHasConfig::returnResponse);
        }

        Action returnResponse(const oatpp::Object<HasConfigDTO>& body) {
            return _return(handleConfigAction<HasConfigDTO, StatusDto>(
                this->controller, body, "lithium.config.has",
                [&](auto mConfigManager, [[maybe_unused]] auto res) {
                    return mConfigManager->hasValue(body->path);
                }));
        }
    };

    ENDPOINT_INFO(getUIListConfig) {
        info->summary = "List config from ConfigManager";
        info->description = "List all configuration values from ConfigManager.";
        info->tags = {"Config"};
        info->addResponse<Object<ReturnListConfigDTO>>(Status::CODE_200,
                                                       "application/json");
        info->addResponse<Object<InternalServerErrorDto>>(Status::CODE_500,
                                                          "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/config/list", getUIListConfig) {
        ENDPOINT_ASYNC_INIT(getUIListConfig);

        Action act() override {
            return _return(handleConfigAction<RequestDto, ReturnListConfigDTO>(
                this->controller, nullptr, "lithium.config.list",
                [&](auto mConfigManager, auto res) {
                    res->config = mConfigManager->dumpConfig();
                    return true;
                }));
        }
    };

    ENDPOINT_INFO(getUITidyConfig) {
        info->summary = "Tidy config from ConfigManager";
        info->description =
            "Cleans up the configuration by removing unused entries or "
            "optimizing data.";
        info->tags = {"Config"};
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<InternalServerErrorDto>>(Status::CODE_500,
                                                          "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/config/tidy", getUITidyConfig) {
        ENDPOINT_ASYNC_INIT(getUITidyConfig);

        auto act() -> Action override {
            return _return(handleConfigAction<RequestDto, StatusDto>(
                this->controller, nullptr, "lithium.config.tidy",
                [&](auto mConfigManager, [[maybe_unused]] auto res) {
                    mConfigManager->tidyConfig();
                    return true;
                }));
        }
    };

    ENDPOINT_INFO(getUILoadConfig) {
        info->summary = "Load config from file or directory";
        info->description =
            "Load configuration data from a file or directory into "
            "ConfigManager.";
        info->tags = {"Config"};
        info->addConsumes<Object<LoadConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<PathNotFoundDto>>(Status::CODE_404,
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
            return _return(
                handleConfigAction<LoadConfigDTO, ReturnListConfigDTO>(
                    this->controller, body, "lithium.config.load",
                    [&](auto mConfigManager, auto res) {
                        auto refresh = body->refresh;
                        String path =
                            body->rootPath->empty()
                                ? (atom::io::isAbsolutePath(body->path) &&
                                           body->isAbsolute
                                       ? body->path
                                       : atom::system::
                                                 getCurrentWorkingDirectory() +
                                             Constants::PATH_SEPARATOR +
                                             body->path)
                                : body->rootPath + Constants::PATH_SEPARATOR +
                                      body->path;
                        auto type = atom::io::checkPathType(*path);
                        bool success = false;
                        if (type == atom::io::PathType::REGULAR_FILE) {
                            OATPP_ASSERT_HTTP(atom::io::isFileNameValid(path),
                                              Status::CODE_404,
                                              "File name is not valid")
                            success = mConfigManager->loadFromFile(*path);
                        }
                        if (type == atom::io::PathType::DIRECTORY) {
                            OATPP_ASSERT_HTTP(atom::io::isFolderExists(path),
                                              Status::CODE_404,
                                              "Folder does not exist")
                            success = mConfigManager->loadFromDir(*path);
                        }
                        if (success && refresh) {
                            res->config = mConfigManager->dumpConfig();
                        }
                        return success;
                    }));
        }
    };

    ENDPOINT_INFO(getUIReloadConfig) {
        info->summary = "Reload config from file";
        info->description =
            "Reload configuration data from a file into ConfigManager.";
        info->tags = {"Config"};
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<PathNotFoundDto>>(Status::CODE_404,
                                                   "application/json");
        info->addResponse<Object<InternalServerErrorDto>>(Status::CODE_500,
                                                          "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/config/reload", getUIReloadConfig) {
        ENDPOINT_ASYNC_INIT(getUIReloadConfig);

        Action act() override {
            return _return(handleConfigAction<RequestDto, StatusDto>(
                this->controller, nullptr, "reloadConfig",
                [&](auto mConfigManager, auto res) {
                    return mConfigManager->loadFromFile("config/config.json");
                }));
        }
    };

    ENDPOINT_INFO(getUISaveConfig) {
        info->summary = "Save config to file";
        info->addConsumes<Object<SaveConfigDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<ForbiddenDto>>(Status::CODE_403,
                                                "application/json");
        info->addResponse<Object<InternalServerErrorDto>>(Status::CODE_500,
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
            return _return(handleConfigAction<SaveConfigDTO, StatusDto>(
                this->controller, body, "saveConfig",
                [&](auto mConfigManager, auto res) {
                    return mConfigManager->saveToFile(
                        body->path.getValue("config/config.json"));
                }));
        }
    };
};

std::shared_ptr<lithium::ConfigManager> ConfigController::mConfigManager =
    nullptr;

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_SCRIPT_CONTROLLER_HPP