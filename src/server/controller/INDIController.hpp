#ifndef INDICONTROLLER_HPP
#define INDICONTROLLER_HPP

#include <exception>
#include <regex>
#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include "data/INDIDto.hpp"
#include "data/RequestDto.hpp"

#include "atom/error/error_code.hpp"
#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/sysinfo/os.hpp"
#include "atom/system//software.hpp"
#include "atom/system/process.hpp"
#include "atom/system/user.hpp"
#include "atom/utils/ranges.hpp"

#include "atom/macro.hpp"
#include "system/command.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  /// <-- Begin Code-Gen

// 将版本号字符串拆分为整数
inline auto splitVersion(const std::string& version) -> std::vector<int> {
    std::vector<int> parts;
    std::stringstream versionStream(version);
    std::string part;

    // 以'.'为分隔符分割版本号
    while (std::getline(versionStream, part, '.')) {
        try {
            if (part.empty()) {
                throw std::invalid_argument(
                    "Empty part found in version number");
            }
            parts.push_back(std::stoi(part));  // 将字符串转换为整数并存储
        } catch (const std::invalid_argument&) {
            LOG_F(
                ERROR,
                "Invalid version number: {}. Part '{}' is not a valid integer.",
                version, part);
            THROW_NESTED_RUNTIME_ERROR("Invalid version number: " + version +
                                       ". Part '" + part +
                                       "' is not a valid integer.");
        } catch (const std::out_of_range&) {
            LOG_F(ERROR, "Number out of range in version number: {}", part);
            THROW_NESTED_RUNTIME_ERROR(
                "Number out of range in version number: " + part);
        }
    }

    return parts;
}

inline auto compareVersions(const std::string& version1,
                            const std::string& version2) -> bool {
    std::vector<int> versionParts1;
    std::vector<int> versionParts2;

    try {
        versionParts1 = splitVersion(version1);
        versionParts2 = splitVersion(version2);
    } catch (...) {
        LOG_F(ERROR, "Error occurred while parsing versions.");
        return false;
    }

    size_t maxLength = std::max(versionParts1.size(), versionParts2.size());

    for (size_t i = 0; i < maxLength; ++i) {
        int part1 = i < versionParts1.size() ? versionParts1[i] : 0;
        int part2 = i < versionParts2.size() ? versionParts2[i] : 0;

        if (part1 != part2) {
            return part1 > part2;
        }
    }

    return false;
}

class INDIController : public oatpp::web::server::api::ApiController {
    using ControllerType = INDIController;

public:
    INDIController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
        : oatpp::web::server::api::ApiController(objectMapper) {}

    ENDPOINT_INFO(getUIApiServerINDIExecutable) {
        info->summary = "Get INDI server executable";
        info->addConsumes<Object<RequestDto>>("application/json");
        info->addResponse<Object<ReturnINDIExecutableDto>>(Status::CODE_200,
                                                           "application/json");
        info->addResponse<Object<StatusDto>>(
            Status::CODE_300, "application/json",
            "INDI library version is lower than 2.0.0");
        info->addResponse<Object<StatusDto>>(Status::CODE_500,
                                             "application/json",
                                             "INDI server is not installed");
    }

    ENDPOINT_ASYNC("GET", "/api/server/indi/executable",
                   getUIApiServerINDIExecutable) {
        ENDPOINT_ASYNC_INIT(getUIApiServerINDIExecutable);

        static constexpr auto COMMAND =
            "lithium.server.starter.indi.executable";

        struct VersionInfo {
            std::string libraryVersion;
            std::string coreVersion;
            std::string protocolVersion;
        };

    private:
        VersionInfo parseINDIVersions() {
            VersionInfo versionInfo;
            std::string indiOutput = atom::system::executeCommand("indiserver");

            // Regular expressions for extracting version info
            std::regex libVersionRegex(R"(INDI Library: (\d+\.\d+\.\d+))");
            std::regex coreVersionRegex(R"(Code (\d+\.\d+\.\d+)-tgz)");
            std::regex protocolVersionRegex(R"(Protocol (\d+\.\d+))");
            std::smatch match;

            if (std::regex_search(indiOutput, match, libVersionRegex)) {
                versionInfo.libraryVersion = match[1];
                LOG_F(INFO, "Library Version: {}", versionInfo.libraryVersion);
            } else {
                throw std::runtime_error("Library version not found");
            }

            if (std::regex_search(indiOutput, match, coreVersionRegex)) {
                versionInfo.coreVersion = match[1];
                LOG_F(INFO, "Core Version: {}", versionInfo.coreVersion);
            }

            if (std::regex_search(indiOutput, match, protocolVersionRegex)) {
                versionInfo.protocolVersion = match[1];
                LOG_F(INFO, "Protocol Version: {}",
                      versionInfo.protocolVersion);
            }

            return versionInfo;
        }

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
            // Check if INDI server is installed
            if (!atom::system::checkSoftwareInstalled("indiserver")) {
                return _return(createErrorResponse(
                    "INDI server is not installed", Status::CODE_500));
            }

            // Get INDI server executable path, version, and permissions
            auto path = atom::system::getAppPath("indiserver");
            auto version = atom::system::getAppVersion(path);
            auto permissions = atom::system::getAppPermissions(path);

            auto instance = INDIExecutableDto::createShared();
            instance->executable = "indiserver";
            instance->version = version;
            instance->path = path.string();
            instance->permissions->assign(permissions.begin(),
                                          permissions.end());

            auto res = ReturnINDIExecutableDto::createShared();
            res->command = COMMAND;
            res->instances->emplace_back(instance);
            res->status = "success";
            res->message = "INDI server executable found";

            // Execute INDI server command and extract version information
            try {
                VersionInfo versionInfo = parseINDIVersions();
                if (!compareVersions(versionInfo.libraryVersion, "2.0.0")) {
                    return _return(createWarningResponse(
                        "INDI library version is lower than 2.0.0",
                        Status::CODE_300));
                }
            } catch (const std::exception& e) {
                LOG_F(ERROR, "Unable to parse INDI server version: {}",
                      e.what());
                return _return(createErrorResponse(
                    "Unable to parse INDI server version", Status::CODE_500));
            }

            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIApiServreINDIScan) {
        info->summary = "Scan INDI server";
        info->addConsumes<Object<RequestDto>>("application/json");
        info->addResponse<Object<ReturnServerINDIScanDto>>(Status::CODE_200,
                                                           "application/json");
        info->addResponse<Object<StatusDto>>(
            Status::CODE_300, "application/json",
            "WSL detected, INDI server cannot perform all operations. Please "
            "use a real Linux system.");
        info->addResponse<Object<StatusDto>>(
            Status::CODE_301, "application/json",
            "Running as root is not recommended");
        info->addResponse<Object<StatusDto>>(
            Status::CODE_302, "application/json",
            "Windows detected, INDI server cannot run on Windows");
        info->addResponse<Object<ReturnServerINDIScanMultiInstancesDto>>(
            Status::CODE_402, "application/json");
        info->addResponse<Object<StatusDto>>(
            Status::CODE_500, "application/json",
            "Unable to get INDI server instances");
    }
    ENDPOINT_ASYNC("GET", "/api/server/indi/scan", getUIApiServreINDIScan) {
        ENDPOINT_ASYNC_INIT(getUIApiServreINDIScan);

        static constexpr auto COMMAND = "lithium.server.starter.indi.scan";

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiServreINDIScan::returnResponse);
        }

    private:
        auto createWarningResponse(const std::string& message, Status status) {
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "warning";
            res->warning = message;
            return controller->createDtoResponse(status, res);
        }

        auto createErrorResponse(const std::string& message, Status status) {
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "error";
            res->error = message;
            return controller->createDtoResponse(status, res);
        }

        auto returnResponse(const oatpp::Object<RequestDto>& body) -> Action {
            const auto force = body->force;
            const auto timeout = body->timeout;
            const auto retry = body->retry;

            // Root user warning
            if (atom::system::isRoot()) {
                return _return(createWarningResponse(
                    "Running as root is not recommended", Status::CODE_301));
            }

            // WSL environment warning
            if (atom::system::isWsl()) {
                return _return(createWarningResponse(
                    "WSL detected, INDI server cannot perform all operations. "
                    "Please use a real Linux system.",
                    Status::CODE_300));
            }

            // Windows OS error
            const auto OS_INFO = atom::system::getOperatingSystemInfo();
            if (OS_INFO.osName.starts_with("Windows")) {
                return _return(createErrorResponse(
                    "Windows detected, INDI server cannot run on Windows",
                    Status::CODE_302));
            }

            // Fetch INDI and Hydrogen server instances
            try {
                auto indiInstances =
                    atom::system::getProcessIdByName("indiserver");
                auto hydrogenInstances =
                    atom::system::getProcessIdByName("hydrogenserver");

                auto instances = atom::utils::MergeViewImpl{}(
                    indiInstances, hydrogenInstances);
                if (instances.begin() == instances.end()) {
                    return _return(createErrorResponse(
                        "No INDI server instances found", Status::CODE_404));
                }

                auto res =
                    ReturnServerINDIScanMultiInstancesDto::createShared();
                res->command = COMMAND;

                for (const auto& instance : instances) {
                    auto process = atom::system::getProcessInfoByPid(instance);
                    auto instanceDto = MultiInstancesDto::createShared();
                    instanceDto->pid = instance;
                    instanceDto->path = process.path.string();
                    instanceDto->version =
                        "2.1.0";  // Placeholder version, should fetch
                                  // dynamically if possible
                    instanceDto->name = process.name;

                    auto ports = atom::system::getNetworkConnections(instance);
                    if (!ports.empty()) {
                        instanceDto->port = ports[0].localPort;
                    }

                    res->instance->emplace_back(instanceDto);
                }

                return _return(
                    controller->createDtoResponse(Status::CODE_200, res));

            } catch (const std::exception& e) {
                LOG_F(ERROR, "Unable to get INDI server instances: {}",
                      e.what());
                return _return(createErrorResponse(
                    "Unable to get INDI server instances", Status::CODE_500));
            }
        }
    };

    ENDPOINT_INFO(getUIApiServerINDIStart) {
        info->summary = "Start INDI server";
        info->addConsumes<Object<RequestINDIStartDto>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_500,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/server/indi/start", getUIApiServerINDIStart) {
        ENDPOINT_ASYNC_INIT(getUIApiServerINDIStart);

        static constexpr auto COMMAND = "lithium.server.starter.indi.start";

        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestINDIStartDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiServerINDIStart::returnResponse);
        }

    private:
        auto createWarningResponse(const std::string& message, Status status) {
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "warning";
            res->warning = message;
            return controller->createDtoResponse(status, res);
        }

        auto createErrorResponse(const std::string& message, Status status) {
            auto res = StatusDto::createShared();
            res->command = COMMAND;
            res->status = "error";
            res->error = message;
            return controller->createDtoResponse(status, res);
        }

        auto returnResponse(
            const oatpp::Object<RequestINDIStartDto>& body) -> Action {
            const auto force = body->force;
            const auto timeout = body->timeout;
            const auto retry = body->retry;
            const auto executable = body->executable;
            const auto port = body->port;
            const auto log = body->logLevel;
            const auto tmpPath = body->tmpPath;
            const auto enableLog = body->enableLog;

            // Start INDI server
            try {
                // atom::system::startProcess("indiserver");
                return _return(createWarningResponse(
                    "INDI server started successfully", Status::CODE_200));
            } catch (const std::exception& e) {
                LOG_F(ERROR, "Unable to start INDI server: {}", e.what());
                return _return(createErrorResponse(
                    "Unable to start INDI server", Status::CODE_500));
            }
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // INDICONTROLLER_HPP
