#ifndef SCRIPTCONTROLLER_HPP
#define SCRIPTCONTROLLER_HPP

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include "data/ScriptDto.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/env.hpp"
#include "atom/system/software.hpp"
#include "atom/system/user.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/container.hpp"
#if ENABLE_ASYNC
#include <asio/io_context.hpp>
#include "atom/io/async_io.hpp"
#endif
#include "atom/macro.hpp"

#include "addon/version.hpp"
#include "config/configor.hpp"

#include "utils/constant.hpp"

#if __has_include(<yaml-cpp/yaml.h>)
#include <yaml-cpp/yaml.h>
#endif

#if __has_include(<tinyxml2/tinyxml2.h>)
#include <tinyxml2/tinyxml2.h>
#elif __has_include(<tinyxml2.h>)
#include <tinyxml2.h>
#endif

#include <fstream>
#include <regex>
#include <string>

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

    struct ScriptInfo {
        std::string interpreter;
        std::string description;
        std::string author;
        std::string version;
        std::string license;
        std::vector<std::string> additionalLines;
    } ATOM_ALIGNAS(128);

    static auto parseScriptHeader(const std::string& filePath) -> ScriptInfo {
        std::ifstream file(filePath);
        ScriptInfo info;
        std::string line;

        std::regex shebangRegex(R"(^#!\s*(\S+))");
        std::regex descriptionRegex(R"(^#\s*Description:\s*(.*))");
        std::regex authorRegex(R"(^#\s*Author:\s*(.*))");
        std::regex versionRegex(R"(^#\s*Version:\s*(.*))");
        std::regex licenseRegex(R"(^#\s*License:\s*(.*))");

        if (file.is_open()) {
            while (std::getline(file, line)) {
                std::smatch match;
                if (std::regex_search(line, match, shebangRegex)) {
                    info.interpreter = match[1];
                    if (info.interpreter.find("bash") != std::string::npos ||
                        info.interpreter.find("sh") != std::string::npos) {
                        info.interpreter = "Bash";
                    } else if (info.interpreter.find("pwsh") !=
                                   std::string::npos ||
                               info.interpreter.find("powershell") !=
                                   std::string::npos) {
                        info.interpreter = "PowerShell";
                    }
                } else if (std::regex_search(line, match, descriptionRegex)) {
                    info.description = match[1];
                } else if (std::regex_search(line, match, authorRegex)) {
                    info.author = match[1];
                } else if (std::regex_search(line, match, versionRegex)) {
                    info.version = match[1];
                } else if (std::regex_search(line, match, licenseRegex)) {
                    info.license = match[1];
                } else if (!line.empty() && line[0] == '#') {
                    info.additionalLines.push_back(line);
                }

                if (line.empty() || line[0] != '#') {
                    break;
                }
            }
            file.close();
        } else {
            LOG_F(ERROR, "Cannot open file: {}", filePath);
        }

        return info;
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

    ENDPOINT_INFO(getUIApiScriptGetAll) {
        info->summary = "Get All Scripts";
        info->addConsumes<Object<RequestScriptListDto>>("application/json");
        info->addResponse<Object<ReturnScriptListDto>>(Status::CODE_200,
                                                       "application/json");
        info->addResponse<Object<StatusDto>>(
            Status::CODE_500, "application/json", "Unable to get script list");
    }
    ENDPOINT_ASYNC("GET", "/api/script/list", getUIApiScriptGetAll) {
        ENDPOINT_ASYNC_INIT(getUIApiScriptGetAll);

        static constexpr auto COMMAND = "lithium.script.list";  // Command name
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
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestScriptListDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiScriptGetAll::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<RequestScriptListDto>& body) -> Action {
            try {
                auto dtoPath = body->path;
                std::string scriptPath;

                auto getScriptPathFromConfig =
                    []() -> std::optional<std::string> {
                    std::weak_ptr<lithium::ConfigManager> configWeekPtr;
                    GET_OR_CREATE_WEAK_PTR(configWeekPtr,
                                           lithium::ConfigManager,
                                           Constants::CONFIG_MANAGER);
                    if (configWeekPtr.expired()) {
                        LOG_F(ERROR, "ConfigManager is not initialized");
                        return std::nullopt;
                    }

                    auto configPath =
                        configWeekPtr.lock()->getValue("/lithium/script/path");
                    if (configPath.has_value() && !configPath.value().empty() &&
                        configPath->type() == nlohmann::json::value_t::string) {
                        std::string scriptPath = configPath.value();
                        if (!atom::io::isAbsolutePath(scriptPath)) {
                            scriptPath = atom::system::getHomeDirectory() +
                                         Constants::PATH_SEPARATOR + scriptPath;
                        }
                        if (!atom::io::isFolderExists(scriptPath)) {
                            LOG_F(ERROR, "Script path is not a directory");
                            return std::nullopt;
                        }
                        return scriptPath;
                    }
                    return std::nullopt;
                };

                auto getScriptPathFromEnv = []() -> std::optional<std::string> {
                    std::weak_ptr<atom::utils::Env> envWeekPtr;
                    GET_OR_CREATE_WEAK_PTR(envWeekPtr, atom::utils::Env,
                                           Constants::ENVIRONMENT);
                    if (envWeekPtr.expired()) {
                        LOG_F(ERROR, "Environment is not initialized");
                        return std::nullopt;
                    }

                    auto env = envWeekPtr.lock()->get("LITHIUM_SCRIPT_PATH");
                    if (!env.empty()) {
                        if (!atom::io::isAbsolutePath(env)) {
                            env = atom::system::getHomeDirectory() +
                                  Constants::PATH_SEPARATOR + env;
                        }
                        if (atom::io::isFolderExists(env)) {
                            return env;
                        }
                    }
                    return std::nullopt;
                };

                if (!dtoPath->empty()) {
                    scriptPath = dtoPath.getValue("");
                    if (!atom::io::isAbsolutePath(scriptPath)) {
                        scriptPath = atom::system::getHomeDirectory() +
                                     Constants::PATH_SEPARATOR + scriptPath;
                    }
                    if (atom::io::checkPathType(scriptPath) !=
                        atom::io::PathType::DIRECTORY) {
                        LOG_F(ERROR, "Script path is not a directory");
                        return _return(createErrorResponse(
                            "Script path is not a directory",
                            Status::CODE_500));
                    }
                } else {
                    auto configPath = getScriptPathFromConfig();
                    if (configPath.has_value()) {
                        scriptPath = configPath.value();
                    } else {
                        auto envPath = getScriptPathFromEnv();
                        if (envPath.has_value()) {
                            scriptPath = envPath.value();
                        } else {
                            LOG_F(ERROR, "Script path is not set");
                            return _return(createErrorResponse(
                                "Script path is not set", Status::CODE_500));
                        }
                    }
                }

                auto res = ReturnScriptListDto::createShared();
                auto jsonScriptDes = atom::io::checkFileTypeInFolder(
                    scriptPath, "json", atom::io::FileOption::PATH);
                for (const auto& script : jsonScriptDes) {
                    LOG_F(INFO, "Trying to load script descriptor: {}", script);
                    json j;
                    try {
                        std::fstream file(script);
                        if (!file.is_open()) {
                            LOG_F(ERROR, "Unable to open script descriptor: {}",
                                  script);
                            continue;
                        }
                        file >> j;
                    } catch (const json::parse_error& e) {
                        LOG_F(ERROR, "Unable to parse script descriptor: {}",
                              e.what());
                        continue;
                    }

                    auto scriptDto = ScriptDto::createShared();
                    try {
                        if (j.contains("name") && j["name"].is_string()) {
                            scriptDto->name = j["name"].get<std::string>();
                        }
                        if (j.contains("type") && j["type"].is_string()) {
                            scriptDto->type = j["type"].get<std::string>();
                            if (!atom::utils::contains(
                                    "shell, powershell, python"_vec,
                                    *scriptDto->type)) {
                                LOG_F(ERROR, "Invalid script type: {}",
                                      *scriptDto->type);
                                continue;
                            }
                        }
                        if (j.contains("description") &&
                            j["description"].is_string()) {
                            scriptDto->description =
                                j["description"].get<std::string>();
                        }
                        if (j.contains("author") && j["author"].is_string()) {
                            scriptDto->author = j["author"].get<std::string>();
                        }
                        if (j.contains("version") && j["version"].is_string()) {
                            scriptDto->version =
                                j["version"].get<std::string>();
                        }
                        if (j.contains("license") && j["license"].is_string()) {
                            scriptDto->license =
                                j["license"].get<std::string>();
                        }
                        if (j.contains("interpreter") &&
                            j["interpreter"].is_object()) {
                            auto interpreter = j["interpreter"];
                            if (interpreter.contains("path") &&
                                interpreter["path"].is_string()) {
                                scriptDto->interpreter->path =
                                    interpreter["path"].get<std::string>();
                                if (!atom::io::isExecutableFile(
                                        scriptDto->interpreter->path, "")) {
                                    LOG_F(ERROR,
                                          "Interpreter is not executable: {}",
                                          scriptDto->interpreter->path);
                                    continue;
                                }
                            }
                            if (interpreter.contains("name") &&
                                interpreter["name"].is_string()) {
                                scriptDto->interpreter->interpreter =
                                    interpreter["name"].get<std::string>();
                                if (scriptDto->interpreter->path->empty()) {
                                    scriptDto->interpreter->path =
                                        atom::system::getAppPath(
                                            scriptDto->interpreter->interpreter)
                                            .string();
                                    if (scriptDto->interpreter->path->empty()) {
                                        LOG_F(ERROR,
                                              "Unable to get interpreter path: "
                                              "{}",
                                              scriptDto->interpreter
                                                  ->interpreter);
                                        continue;
                                    }
                                }
                            }
                            if (interpreter.contains("version") &&
                                interpreter["version"].is_string()) {
                                scriptDto->interpreter->version =
                                    interpreter["version"].get<std::string>();
                                auto interpreterVersion =
                                    atom::system::getAppVersion(
                                        *scriptDto->interpreter->path);
                                if (interpreterVersion.empty()) {
                                    LOG_F(
                                        ERROR,
                                        "Unable to get interpreter version: {}",
                                        scriptDto->interpreter->path);
                                    continue;
                                }
                                if (!lithium::checkVersion(
                                        lithium::Version::parse(
                                            interpreterVersion),
                                        *scriptDto->interpreter->version)) {
                                    LOG_F(ERROR,
                                          "Interpreter version is lower than "
                                          "required: {}",
                                          scriptDto->interpreter->version);
                                    continue;
                                }
                            }
                        }
                        if (j.contains("platform") &&
                            j["platform"].is_string()) {
                            scriptDto->platform =
                                j["platform"].get<std::string>();
                            if (!atom::utils::contains(
                                    "windows, linux, macos"_vec,
                                    *scriptDto->platform)) {
                                LOG_F(ERROR, "Invalid platform: {}",
                                      *scriptDto->platform);
                                continue;
                            }
                        }
                        if (j.contains("permission") &&
                            j["permission"].is_string()) {
                            scriptDto->permission =
                                j["permission"].get<std::string>();
                            if (!atom::utils::contains(
                                    "user, admin"_vec,
                                    *scriptDto->permission)) {
                                LOG_F(ERROR, "Invalid permission: {}",
                                      *scriptDto->permission);
                                continue;
                            }
                            if (*scriptDto->permission == "admin" &&
                                !atom::system::isRoot()) {
                                LOG_F(ERROR, "User is not admin");
                                continue;
                            }
                        }

                        auto lineOpt = atom::io::countLinesInFile(script);
                        if (lineOpt.has_value()) {
                            scriptDto->line = lineOpt.value();
                        }

                        if (j.contains("args") && j["args"].is_array()) {
                            for (const auto& arg : j["args"]) {
                                if (arg.is_object()) {
                                    auto argDto =
                                        ArgumentRequirementDto::createShared();
                                    if (arg.contains("name") &&
                                        arg["name"].is_string()) {
                                        argDto->name =
                                            arg["name"].get<std::string>();
                                    }
                                    if (arg.contains("type") &&
                                        arg["type"].is_string()) {
                                        argDto->type =
                                            arg["type"].get<std::string>();
                                        if (!atom::utils::contains(
                                                "string, int, float, bool"_vec,
                                                *argDto->type)) {
                                            LOG_F(ERROR,
                                                  "Invalid argument type: {}",
                                                  *argDto->type);
                                            continue;
                                        }
                                    }
                                    if (arg.contains("description") &&
                                        arg["description"].is_string()) {
                                        argDto->description =
                                            arg["description"]
                                                .get<std::string>();
                                    }
                                    if (arg.contains("defaultValue") &&
                                        arg["defaultValue"].is_string()) {
                                        argDto->defaultValue =
                                            arg["defaultValue"]
                                                .get<std::string>();
                                    }
                                    if (arg.contains("required") &&
                                        arg["required"].is_boolean()) {
                                        argDto->required =
                                            arg["required"].get<bool>();
                                    }
                                    scriptDto->args->emplace_back(argDto);
                                }
                            }
                        }
                        res->scripts->emplace_back(scriptDto);
                    } catch (const json::type_error& e) {
                        LOG_F(ERROR, "Unable to parse script value: {}",
                              e.what());
                        continue;
                    }
                }

#if __has_include(<yaml-cpp/yaml.h>)
                auto yamlScriptDes = atom::io::checkFileTypeInFolder(
                    scriptPath, "yaml", atom::io::FileOption::PATH);
                for (const auto& script : yamlScriptDes) {
                    LOG_F(INFO, "Trying to load script descriptor: {}", script);
                    auto scriptDto = ScriptDto::createShared();
                    try {
                        YAML::Node node = YAML::LoadFile(script);
                        if (node["name"] && node["name"].IsScalar()) {
                            scriptDto->name = node["name"].as<std::string>();
                        }
                        if (node["type"] && node["type"].IsScalar()) {
                            scriptDto->type = node["type"].as<std::string>();
                            if (!atom::utils::contains(
                                    "shell, powershell, python"_vec,
                                    *scriptDto->type)) {
                                LOG_F(ERROR, "Invalid script type: {}",
                                      *scriptDto->type);
                                continue;
                            }
                        }
                        if (node["description"] &&
                            node["description"].IsScalar()) {
                            scriptDto->description =
                                node["description"].as<std::string>();
                        }
                        if (node["author"] && node["author"].IsScalar()) {
                            scriptDto->author =
                                node["author"].as<std::string>();
                        }
                        if (node["version"] && node["version"].IsScalar()) {
                            scriptDto->version =
                                node["version"].as<std::string>();
                        }
                        if (node["license"] && node["license"].IsScalar()) {
                            scriptDto->license =
                                node["license"].as<std::string>();
                        }
                        if (node["interpreter"] &&
                            node["interpreter"].IsMap()) {
                            auto interpreter = node["interpreter"];
                            if (interpreter["path"] &&
                                interpreter["path"].IsScalar()) {
                                scriptDto->interpreter->path =
                                    interpreter["path"].as<std::string>();
                                if (!atom::io::isExecutableFile(
                                        scriptDto->interpreter->path, "")) {
                                    LOG_F(ERROR,
                                          "Interpreter is not executable: {}",
                                          scriptDto->interpreter->path);
                                    continue;
                                }
                            }
                            if (interpreter["name"] &&
                                interpreter["name"].IsScalar()) {
                                scriptDto->interpreter->interpreter =
                                    interpreter["name"].as<std::string>();
                                if (scriptDto->interpreter->path->empty()) {
                                    scriptDto->interpreter->path =
                                        atom::system::getAppPath(
                                            scriptDto->interpreter->interpreter)
                                            .string();
                                    if (scriptDto->interpreter->path->empty()) {
                                        LOG_F(ERROR,
                                              "Unable to get interpreter path: "
                                              "{}",
                                              scriptDto->interpreter
                                                  ->interpreter);
                                        continue;
                                    }
                                }
                            }
                            if (interpreter["version"] &&
                                interpreter["version"].IsScalar()) {
                                scriptDto->interpreter->version =
                                    interpreter["version"].as<std::string>();
                                auto interpreterVersion =
                                    atom::system::getAppVersion(
                                        *scriptDto->interpreter->path);
                                if (interpreterVersion.empty()) {
                                    LOG_F(
                                        ERROR,
                                        "Unable to get interpreter version: {}",
                                        scriptDto->interpreter->path);
                                    continue;
                                }
                                if (!lithium::checkVersion(
                                        lithium::Version::parse(
                                            interpreterVersion),
                                        *scriptDto->interpreter->version)) {
                                    LOG_F(ERROR,
                                          "Interpreter version is lower than "
                                          "required: {}",
                                          scriptDto->interpreter->version);
                                    continue;
                                }
                            }
                        }
                        if (node["platform"] && node["platform"].IsScalar()) {
                            scriptDto->platform =
                                node["platform"].as<std::string>();
                            if (!atom::utils::contains(
                                    "windows, linux, macos"_vec,
                                    *scriptDto->platform)) {
                                LOG_F(ERROR, "Invalid platform: {}",
                                      *scriptDto->platform);
                                continue;
                            }
                        }
                        if (node["permission"] &&
                            node["permission"].IsScalar()) {
                            scriptDto->permission =
                                node["permission"].as<std::string>();
                            if (!atom::utils::contains(
                                    "user, admin"_vec,
                                    *scriptDto->permission)) {
                                LOG_F(ERROR, "Invalid permission: {}",
                                      *scriptDto->permission);
                                continue;
                            }
                            if (*scriptDto->permission == "admin" &&
                                !atom::system::isRoot()) {
                                LOG_F(ERROR, "User is not admin");
                                continue;
                            }
                        }

                        auto lineOpt = atom::io::countLinesInFile(script);
                        if (lineOpt.has_value()) {
                            scriptDto->line = lineOpt.value();
                        }

                        if (node["args"] && node["args"].IsSequence()) {
                            for (const auto& arg : node["args"]) {
                                if (arg.IsMap()) {
                                    auto argDto =
                                        ArgumentRequirementDto::createShared();
                                    if (arg["name"] && arg["name"].IsScalar()) {
                                        argDto->name =
                                            arg["name"].as<std::string>();
                                    }
                                    if (arg["type"] && arg["type"].IsScalar()) {
                                        argDto->type =
                                            arg["type"].as<std::string>();
                                        if (!atom::utils::contains(
                                                "string, int, float, bool"_vec,
                                                *argDto->type)) {
                                            LOG_F(ERROR,
                                                  "Invalid argument type: {}",
                                                  *argDto->type);
                                            continue;
                                        }
                                    }
                                    if (arg["description"] &&
                                        arg["description"].IsScalar()) {
                                        argDto->description =
                                            arg["description"]
                                                .as<std::string>();
                                    }
                                    if (arg["defaultValue"] &&
                                        arg["defaultValue"].IsScalar()) {
                                        argDto->defaultValue =
                                            arg["defaultValue"]
                                                .as<std::string>();
                                    }
                                    if (arg["required"] &&
                                        arg["required"].IsScalar()) {
                                        argDto->required =
                                            arg["required"].as<bool>();
                                    }
                                    scriptDto->args->emplace_back(argDto);
                                }
                            }
                        }
                    } catch (const YAML::ParserException& e) {
                        LOG_F(ERROR, "Unable to parse script descriptor: {}",
                              e.what());
                        continue;
                    }
#endif
#if __has_include(<tinyxml2/tinyxml2.h>) || __has_include(<tinyxml2.h>)
                    auto xmlScriptDes = atom::io::checkFileTypeInFolder(
                        scriptPath, "xml", atom::io::FileOption::PATH);

                    for (const auto& script : xmlScriptDes) {
                        LOG_F(INFO, "Trying to load script descriptor: {}",
                              script);
                        tinyxml2::XMLDocument doc;
                        if (doc.LoadFile(script.c_str()) !=
                            tinyxml2::XML_SUCCESS) {
                            LOG_F(ERROR, "Unable to load script descriptor: {}",
                                  script);
                            continue;
                        }

                        auto scriptDto = ScriptDto::createShared();
                        auto *root = doc.FirstChildElement("script");
                        if (root == nullptr) {
                            LOG_F(ERROR, "Invalid script descriptor: {}",
                                  script);
                            continue;
                        }

                        if (auto *name = root->FirstChildElement("name")) {
                            scriptDto->name = name->GetText();
                        }
                        if (auto *type = root->FirstChildElement("type")) {
                            scriptDto->type = type->GetText();
                            if (!atom::utils::contains(
                                    "shell, powershell, python"_vec,
                                    *scriptDto->type)) {
                                LOG_F(ERROR, "Invalid script type: {}",
                                      *scriptDto->type);
                                continue;
                            }
                        }
                        if (auto *description =
                                root->FirstChildElement("description")) {
                            scriptDto->description = description->GetText();
                        }
                        if (auto *author = root->FirstChildElement("author")) {
                            scriptDto->author = author->GetText();
                        }
                        if (auto *version = root->FirstChildElement("version")) {
                            scriptDto->version = version->GetText();
                        }
                        if (auto *license = root->FirstChildElement("license")) {
                            scriptDto->license = license->GetText();
                        }
                        if (auto *interpreter =
                                root->FirstChildElement("interpreter")) {
                            if (auto *path =
                                    interpreter->FirstChildElement("path")) {
                                scriptDto->interpreter->path = path->GetText();
                                if (!atom::io::isExecutableFile(
                                        scriptDto->interpreter->path, "")) {
                                    LOG_F(ERROR,
                                          "Interpreter is not executable: {}",
                                          scriptDto->interpreter->path);
                                    continue;
                                }
                            }
                            if (auto *name =
                                    interpreter->FirstChildElement("name")) {
                                scriptDto->interpreter->interpreter =
                                    name->GetText();
                                if (scriptDto->interpreter->path->empty()) {
                                    scriptDto->interpreter->path =
                                        atom::system::getAppPath(
                                            scriptDto->interpreter->interpreter)
                                            .string();
                                    if (scriptDto->interpreter->path == "") {
                                        LOG_F(ERROR,
                                              "Unable to get interpreter path: "
                                              "{}",
                                              scriptDto->interpreter
                                                  ->interpreter);
                                        continue;
                                    }
                                }
                            }
                            if (auto *version =
                                    interpreter->FirstChildElement("version")) {
                                scriptDto->interpreter->version =
                                    version->GetText();
                                auto interpreterVersion =
                                    atom::system::getAppVersion(
                                        *scriptDto->interpreter->path);
                                if (interpreterVersion.empty()) {
                                    LOG_F(
                                        ERROR,
                                        "Unable to get interpreter version: {}",
                                        scriptDto->interpreter->path);
                                    continue;
                                }
                                if (!lithium::checkVersion(
                                        lithium::Version::parse(
                                            interpreterVersion),
                                        *scriptDto->interpreter->version)) {
                                    LOG_F(ERROR,
                                          "Interpreter version is lower than "
                                          "required: {}",
                                          scriptDto->interpreter->version);
                                    continue;
                                }
                            }
                        }
                        if (auto *platform =
                                root->FirstChildElement("platform")) {
                            scriptDto->platform = platform->GetText();
                            if (!atom::utils::contains(
                                    "windows, linux, macos"_vec,
                                    *scriptDto->platform)) {
                                LOG_F(ERROR, "Invalid platform: {}",
                                      *scriptDto->platform);
                                continue;
                            }
                        }
                        if (auto *permission =
                                root->FirstChildElement("permission")) {
                            scriptDto->permission = permission->GetText();
                            if (!atom::utils::contains(
                                    "user, admin"_vec,
                                    *scriptDto->permission)) {
                                LOG_F(ERROR, "Invalid permission: {}",
                                      *scriptDto->permission);
                                continue;
                            }
                            if (*scriptDto->permission == "admin" &&
                                !atom::system::isRoot()) {
                                LOG_F(ERROR, "User is not admin");
                                continue;
                            }
                        }

                        auto lineOpt = atom::io::countLinesInFile(script);
                        if (lineOpt.has_value()) {
                            scriptDto->line = lineOpt.value();
                        }

                        if (auto *args = root->FirstChildElement("args")) {
                            for (auto *arg = args->FirstChildElement("arg");
                                 arg != nullptr;
                                 arg = arg->NextSiblingElement("arg")) {
                                auto argDto =
                                    ArgumentRequirementDto::createShared();
                                if (auto *name =
                                        arg->FirstChildElement("name")) {
                                    argDto->name = name->GetText();
                                }
                                if (auto *type =
                                        arg->FirstChildElement("type")) {
                                    argDto->type = type->GetText();
                                    if (!atom::utils::contains(
                                            "string, int, float, bool"_vec,
                                            *argDto->type)) {
                                        LOG_F(ERROR,
                                              "Invalid argument type: {}",
                                              *argDto->type);
                                        continue;
                                    }
                                }
                                if (auto *description =
                                        arg->FirstChildElement("description")) {
                                    argDto->description =
                                        description->GetText();
                                }
                                if (auto *defaultValue = arg->FirstChildElement(
                                        "defaultValue")) {
                                    argDto->defaultValue =
                                        defaultValue->GetText();
                                }
                                if (auto *required =
                                        arg->FirstChildElement("required")) {
                                    argDto->required =
                                        required->GetText() == "true";
                                }
                                scriptDto->args->emplace_back(argDto);
                            }
                        }
                    }
#endif
                    res->scripts->emplace_back(scriptDto);
                }

                return _return(
                    controller->createDtoResponse(Status::CODE_200, res));
            } catch (const std::exception& e) {
                LOG_F(ERROR, "Unable to get script list: {}", e.what());
                return _return(createErrorResponse(e.what(), Status::CODE_500));
            }
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // SCRIPTCONTROLLER_HPP
