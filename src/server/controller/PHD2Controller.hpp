#ifndef PHD2CONTROLLER_HPP
#define PHD2CONTROLLER_HPP

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

inline auto to_json(const oatpp::Vector<oatpp::String>& vec) -> json {
    json j = json::array();
    for (const auto& str : *vec) {
        j.push_back(str);
    }
    return j;
}

namespace lithium::controller::phd2 {
// Function to determine if the value is a special type (e.g., bounded by {})
auto isSpecialType(const std::string& value) -> bool {
    return value.find('{') != std::string::npos &&
           value.find('}') != std::string::npos;
}

// Function to parse special type values into an array
auto parseSpecialType(const std::string& value)
    -> std::vector<std::pair<std::string, std::string>> {
    std::vector<std::pair<std::string, std::string>> result;
    std::string trimmed = value;  // Remove the surrounding {}
    std::istringstream ss(trimmed);
    std::string item;
    while (std::getline(ss, item, '}')) {
        auto start = item.find('{');
        if (start != std::string::npos) {
            item = item.substr(start + 1);
            item.erase(
                0, item.find_first_not_of(" \t"));  // Trim leading whitespace
            item.erase(item.find_last_not_of(" \t") +
                       1);  // Trim trailing whitespace
            auto pos = item.find(' ');
            if (pos != std::string::npos) {
                std::string first = item.substr(0, pos);
                std::string second = item.substr(pos + 1);
                result.emplace_back(first, second);
            }
        }
    }
    return result;
}

// Function to parse each line correctly considering special cases
auto parseLine(const std::string& line)
    -> std::tuple<std::vector<std::string>, std::string> {
    std::istringstream iss(line);
    std::string key;
    std::string value;

    int temp;
    if (iss >> key >> temp) {
        key.erase(0, 1);  // Remove the leading '/'

        // Use getline to read the remainder of the line (value can contain
        // spaces)
        std::getline(iss, value);
        value.erase(0,
                    value.find_first_not_of(" \t"));  // Trim leading whitespace

        // Check for specific keys and extract device if necessary
        if ((key.find("camera/LastMenuChoice") != std::string::npos ||
             key.find("rotator/LastMenuChoice") != std::string::npos ||
             key.find("scope/LastMenuChoice") != std::string::npos) &&
            value.find("INDI") != std::string::npos &&
            value.find('[') != std::string::npos) {
            auto start = value.find('[');
            auto end = value.find(']');
            if (start != std::string::npos && end != std::string::npos &&
                end > start) {
                value = value.substr(
                    start + 1, end - start - 1);  // Extract the device name
            }
        }

        return {atom::utils::splitString(key, '/'),
                value};  // Split the key by '/'
    }
    return {std::vector<std::string>{}, std::string{}};
}
}  // namespace lithium::controller::phd2

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

class PHD2Controller : public oatpp::web::server::api::ApiController {
private:
    typedef PHD2Controller __ControllerType;
    static std::shared_ptr<lithium::ConfigManager> configManagerPtr;
    static std::shared_ptr<atom::system::ProcessManager> processManagerPtr;
    static std::shared_ptr<atom::utils::Env> envPtr;

public:
    PHD2Controller(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    static auto createShared() -> std::shared_ptr<PHD2Controller> {
        return std::make_shared<PHD2Controller>();
    }

    ENDPOINT_INFO(getUIApiPHD2Scan) {
        info->summary = "Scan PHD2 server";
        info->addConsumes<RequestDto>("application/json");
        info->addResponse<ReturnPHD2ScanDto>(Status::CODE_200,
                                             "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/client/phd2/scan"_path, getUIApiPHD2Scan) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2Scan);

        static constexpr auto COMMAND = "lithium.client.phd2.scan";

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
            // Check if PHD2 is installed
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
                    // Here we cannot find PHD2 in normal way, so we will try to
                    // find it in the PATH
#if _WIN32

#else
#define PROCESS_PHD2_PATHS(var, paths)                             \
    auto var = atom::io::searchExecutableFiles(paths, "phd2");     \
    for (const auto& path : var) {                                 \
        auto phd2Dto = PHD2ExecutableDto::createShared();          \
        auto version = atom::system::getAppVersion(path.string()); \
        auto permission = atom::system::getAppPermissions(path);   \
        phd2Dto->executable = path.string();                       \
        phd2Dto->version = version;                                \
        for (const auto& perm : permission) {                      \
            phd2Dto->permission->emplace_back(perm);               \
        }                                                          \
        res->server->try_emplace("phd2", phd2Dto);                 \
    }
                    PROCESS_PHD2_PATHS(phd2PathInUsrBin, "/usr/bin")
                    PROCESS_PHD2_PATHS(phd2PathInUsrLocalBin, "/usr/local/bin")
                    PROCESS_PHD2_PATHS(phd2PathInOpt, "/opt")
#undef PROCESS_PHD2_PATHS
#endif
                }
                // Save the PHD2 server configurations to the config manager
                GET_OR_CREATE_PTR(configManagerPtr, lithium::ConfigManager,
                                  Constants::CONFIG_MANAGER)
                json j;
                for (auto& it : *res->server) {
                    j[it.first] = {
                        {"name", atom::utils::generateRandomString(5)},
                        {"executable", it.second.executable},
                        {"version", it.second.version},
                        {"permission", to_json(it.second.permission)}};
                }
                configManagerPtr->appendValue("/lithium/client/phd2/servers",
                                              j);

            } catch (const std::exception& e) {
                LOG_F(ERROR, "getUIApiPHD2Scan: {}", e.what());
                return _return(createErrorResponse("Failed to scan PHD2",
                                                   Status::CODE_500));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIApiPHD2Configs) {
        info->summary = "Get PHD2 configurations";
        info->description =
            "Get the PHD2 server configurations from specified "
            " directory";
        info->addConsumes<RequestPHD2ConfigDto>("application/json");
        info->addResponse<ReturnPHD2ConfigDto>(Status::CODE_200,
                                               "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/client/phd2/configs"_path,
                   getUIApiPHD2Configs) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2Configs);

        static constexpr auto COMMAND = "lithium.client.phd2.configs";

        auto createErrorResponse(const std::string& message, Status status) {
            LOG_F(ERROR, "getUIApiPHD2Configs: {}", message);
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "error";
            res->error = message;
            return controller->createDtoResponse(status, res);
        }

        auto createWarningResponse(const std::string& message, Status status) {
            LOG_F(WARNING, "getUIApiPHD2Configs: {}", message);
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "warning";
            res->warning = message;
            return controller->createDtoResponse(status, res);
        }

    public:
        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestPHD2ConfigDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiPHD2Configs::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<RequestPHD2ConfigDto>& body) -> Action {
            auto path = body->path;
            OATPP_ASSERT_HTTP(atom::io::isFolderNameValid(path),
                              Status::CODE_400,
                              "The specified path is invalid");
            OATPP_ASSERT_HTTP(atom::io::isFolderExists(path), Status::CODE_400,
                              "The specified path does not exist");
            auto res = PHDConfigDto::createShared();
            try {
#ifdef _WIN32

#else
                auto configPath = atom::io::checkFileTypeInFolder(
                    path, {".phd2", ".sodium", ".ini"},
                    atom::io::FileOption::PATH);
                if (configPath.empty()) {
                    return _return(createWarningResponse(
                        "No PHD2 configuration found", Status::CODE_404));
                }
                for (const auto& config : configPath) {
                    
                }
#endif
            } catch (const std::exception& e) {
                return _return(createErrorResponse(
                    "Failed to get PHD2 configuration", Status::CODE_500));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIApiPHD2IsRunning) {
        info->summary = "Check if PHD2 server is running";
        info->addConsumes<RequestDto>("application/json");
        info->addResponse<StatusDto>(Status::CODE_200, "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
        info->addResponse<StatusDto>(Status::CODE_400, "application/json");
        info->addResponse<StatusDto>(Status::CODE_404, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/client/phd2/isrunning"_path,
                   getUIApiPHD2IsRunning) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2IsRunning);

        static constexpr auto COMMAND = "lithium.client.phd2.isrunning";

        auto createErrorResponse(const std::string& message, Status status) {
            LOG_F(ERROR, "getUIApiPHD2IsRunning: {}", message);
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "error";
            res->error = message;
            return controller->createDtoResponse(status, res);
        }

        auto createWarningResponse(const std::string& message, Status status) {
            LOG_F(WARNING, "getUIApiPHD2IsRunning: {}", message);
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "warning";
            res->warning = message;
            return controller->createDtoResponse(status, res);
        }

        auto createSuccessResponse() {
            // Set the PHD2 running status to true
            if (configManagerPtr) {
                configManagerPtr->setValue("/lithium/client/phd2/running",
                                           true);
            } else {
                THROW_BAD_CONFIG_EXCEPTION("ConfigManager is not initialized");
            }
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "success";
            return controller->createDtoResponse(Status::CODE_200, res);
        }

    public:
        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiPHD2IsRunning::returnResponse);
        }

        static auto checkPHD2Status() -> bool {
            if (!atom::system::isProcessRunning("phd2")) {
                LOG_F(WARNING, "No PHD2 process found");
                return false;
            }
            return true;
        }

        auto returnResponse(const oatpp::Object<RequestDto>& body) -> Action {
            auto retry = body->retry;
            auto timeout = body->timeout;
            OATPP_ASSERT_HTTP(retry >= 0 && retry <= 5, Status::CODE_400,
                              "Invalid retry value");
            OATPP_ASSERT_HTTP(timeout >= 0 && timeout <= 300, Status::CODE_400,
                              "Invalid timeout");

            auto callback = []() { LOG_F(INFO, "PHD2 process is running"); };

            auto exceptionHandler = [](const std::exception& e) {
                LOG_F(ERROR, "getUIApiPHD2IsRunning: {}", e.what());
            };

            auto completeHandler = []() {
                LOG_F(INFO, "Completed PHD2 status check");
            };

            try {
                auto future = atom::async::asyncRetryE(
                    checkPHD2Status, retry, std::chrono::milliseconds(1000),
                    atom::async::BackoffStrategy::EXPONENTIAL,
                    std::chrono::milliseconds(timeout), callback,
                    exceptionHandler, completeHandler);

                auto sharedFuture =
                    std::make_shared<decltype(future)>(std::move(future));

                sharedFuture->then([this](bool result) {
                    if (result) {
                        return _return(createSuccessResponse());
                    }
                    return _return(createWarningResponse("PHD2 is not running",
                                                         Status::CODE_404));
                });
            } catch (const std::exception& e) {
                return _return(createErrorResponse(
                    "Failed to check PHD2 status", Status::CODE_500));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, nullptr));
        }
    };

#define CREATE_RESPONSE_FUNCTIONS(COMMAND_NAME)                             \
    auto createErrorResponse(const std::string& message, Status status) {   \
        LOG_F(ERROR, "{}: {}", ATOM_FUNC_NAME, message);                    \
        auto res = StatusDto::createShared();                               \
        res->command = COMMAND_NAME;                                        \
        res->status = "error";                                              \
        res->error = message;                                               \
        return controller->createDtoResponse(status, res);                  \
    }                                                                       \
    auto createWarningResponse(const std::string& message, Status status) { \
        LOG_F(WARNING, "{}: {}", ATOM_FUNC_NAME, message);                  \
        auto res = StatusDto::createShared();                               \
        res->command = COMMAND_NAME;                                        \
        res->status = "warning";                                            \
        res->warning = message;                                             \
        return controller->createDtoResponse(status, res);                  \
    }                                                                       \
    auto createSuccessResponse() {                                          \
        if (configManagerPtr) {                                             \
            configManagerPtr->setValue(                                     \
                "/lithium/client/phd2/running",                             \
                COMMAND_NAME == "lithium.client.phd2.start");               \
        } else {                                                            \
            THROW_BAD_CONFIG_EXCEPTION("ConfigManager is not initialized"); \
        }                                                                   \
        auto res = StatusDto::createShared();                               \
        res->command = COMMAND_NAME;                                        \
        res->status = "success";                                            \
        return controller->createDtoResponse(Status::CODE_200, res);        \
    }

    ENDPOINT_INFO(getUIApiPHD2Start) {
        info->summary = "Start PHD2 server";
        info->addConsumes<RequestPHD2StartDto>("application/json");
        info->addResponse<StatusDto>(Status::CODE_200, "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/client/phd2/start"_path, getUIApiPHD2Start) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2Start);

        static constexpr auto COMMAND = "lithium.client.phd2.start";
        CREATE_RESPONSE_FUNCTIONS(COMMAND)

    public:
        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestPHD2StartDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiPHD2Start::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<RequestPHD2StartDto>& body) -> Action {
            if (configManagerPtr) {
                if (auto value = configManagerPtr->getValue(
                        "/lithium/client/phd2/running");
                    value.has_value() && value.value().get<bool>()) {
                    LOG_F(WARNING, "PHD2 is already running");
                    return _return(createWarningResponse(
                        "PHD2 is already running", Status::CODE_400));
                }
            } else {
                THROW_BAD_CONFIG_EXCEPTION("ConfigManager is not initialized");
            }

            auto name = body->name;
            auto args = body->args;
            auto env = body->env;
            try {
                auto serverList =
                    configManagerPtr->getValue("/lithium/client/phd2/servers");
                if (!serverList.has_value()) {
                    return _return(createWarningResponse("No PHD2 server found",
                                                         Status::CODE_404));
                }
                auto servers = serverList.value();
                if (!servers.is_array()) {
                    return _return(createErrorResponse(
                        "Invalid PHD2 server configurations",
                        Status::CODE_500));
                }
                for (const auto& server : servers) {
                    if (server["name"] == name) {
                        auto path = server["executable"].get<std::string>();
                        if (path.empty() || !atom::io::isFileNameValid(path)) {
                            return _return(createErrorResponse(
                                "Invalid PHD2 executable path",
                                Status::CODE_500));
                        }

                        GET_OR_CREATE_PTR(envPtr, atom::utils::Env,
                                          Constants::ENVIRONMENT)
                        for (const auto& [key, value] : *env) {
                            if (envPtr->setEnv(key, value)) {
                                LOG_F(INFO, "Set environment variable: {}={}",
                                      key, value);
                            } else {
                                LOG_F(WARNING,
                                      "Failed to set environment "
                                      "variable: {}={}",
                                      key, value);
                            }
                        }

                        GET_OR_CREATE_PTR(processManagerPtr,
                                          atom::system::ProcessManager,
                                          Constants::PROCESS_MANAGER)

                        if (!processManagerPtr->createProcess(path, "phd2",
                                                              true)) {
                            return _return(createErrorResponse(
                                "Failed to start PHD2", Status::CODE_500));
                        }

                        configManagerPtr->setValue(
                            "/lithium/client/phd2/running", true);
                        return _return(createSuccessResponse());
                    }
                }
            } catch (const std::exception& e) {
                return _return(createErrorResponse(
                    std::format("Failed to start PHD2: {}", e.what()),
                    Status::CODE_500));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, nullptr));
        }
    };

    ENDPOINT_INFO(getUIApiPHD2Stop) {
        info->summary = "Stop PHD2 server";
        info->addConsumes<RequestDto>("application/json");
        info->addResponse<StatusDto>(Status::CODE_200, "application/json");
        info->addResponse<StatusDto>(Status::CODE_500, "application/json");
    }
    ENDPOINT_ASYNC("POST", "/api/client/phd2/stop"_path, getUIApiPHD2Stop) {
        ENDPOINT_ASYNC_INIT(getUIApiPHD2Stop);

        static constexpr auto COMMAND = "lithium.client.phd2.stop";
        CREATE_RESPONSE_FUNCTIONS(COMMAND)

    public:
        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiPHD2Stop::returnResponse);
        }

        auto returnResponse(const oatpp::Object<RequestDto>& body) -> Action {
            if (configManagerPtr) {
                if (auto value = configManagerPtr->getValue(
                        "/lithium/client/phd2/running");
                    value.has_value() && !value.value().get<bool>()) {
                    LOG_F(WARNING, "PHD2 is not running");
                    return _return(createWarningResponse("PHD2 is not running",
                                                         Status::CODE_400));
                }
            } else {
                THROW_BAD_CONFIG_EXCEPTION("ConfigManager is not initialized");
            }

            try {
                GET_OR_CREATE_PTR(processManagerPtr,
                                  atom::system::ProcessManager,
                                  Constants::PROCESS_MANAGER)

                if (!processManagerPtr->terminateProcessByName("phd2")) {
                    return _return(createErrorResponse("Failed to stop PHD2",
                                                       Status::CODE_500));
                }

                configManagerPtr->setValue("/lithium/client/phd2/running",
                                           false);
                return _return(createSuccessResponse());
            } catch (const std::exception& e) {
                return _return(createErrorResponse(
                    std::format("Failed to stop PHD2: {}", e.what()),
                    Status::CODE_500));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, nullptr));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif /* PHD2CONTROLLER_HPP */
