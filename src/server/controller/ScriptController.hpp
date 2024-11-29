#ifndef SCRIPTCONTROLLER_HPP
#define SCRIPTCONTROLLER_HPP

#include "config.h"

#include "oatpp/async/Executor.hpp"
#include "oatpp/json/Deserializer.hpp"
#include "oatpp/json/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "oatpp/macro/codegen.hpp"
#include "oatpp/macro/component.hpp"

#include "ControllerCheck.hpp"

#include "data/ScriptDto.hpp"

#include "atom/extra/tinyxml2/tinyxml2.h"
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

#include <fstream>
#include <regex>
#include <string>
#include <utility>

#define CREATE_RESPONSE_MACRO(RESPONSE_TYPE, MESSAGE_FIELD)          \
    auto create##RESPONSE_TYPE##Response(const std::string &message, \
                                         Status status) {            \
        auto res = StatusDto::createShared();                        \
        res->command = COMMAND;                                      \
        res->status = #RESPONSE_TYPE;                                \
        res->MESSAGE_FIELD = message;                                \
        return controller->createDtoResponse(status, res);           \
    }

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

    static auto parseScriptHeader(const std::string &filePath) -> ScriptInfo {
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
    ENDPOINT_ASYNC("GET", "/api/script/env"_path, getUIApiScriptEnv) {
        ENDPOINT_ASYNC_INIT(getUIApiScriptEnv);

        static constexpr auto COMMAND = "lithium.script.env";  // Command name
    private:
        CREATE_RESPONSE_MACRO(Error, error)
        CREATE_RESPONSE_MACRO(Warning, warning)

    public:
        auto act() -> Action override {
            try {
                auto env = atom::utils::Env::Environ();

                auto res = ReturnScriptEnvDto::createShared();
                res->code = 200;
                res->status = "success";
                res->message = "Get script environment successfully";

                for (const auto &[key, value] : env) {
                    res->env[key] = value;
                }

                return _return(
                    controller->createDtoResponse(Status::CODE_200, res));

            } catch (const std::exception &e) {
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
    ENDPOINT_ASYNC("GET", "/api/script/list"_path, getUIApiScriptGetAll) {
        ENDPOINT_ASYNC_INIT(getUIApiScriptGetAll);

        static constexpr auto COMMAND = "lithium.script.list";  // Command name
    private:
        CREATE_RESPONSE_MACRO(Error, error)
        CREATE_RESPONSE_MACRO(Warning, warning)

        class OpenFileCoroutine
            : public oatpp::async::CoroutineWithResult<OpenFileCoroutine,
                                                       json> {
        private:
            std::string script_;
            std::fstream file_;

        public:
            OpenFileCoroutine(std::string script)
                : script_(std::move(script)) {}

            Action act() override {
                file_.open(script_);
                if (!file_.is_open()) {
                    LOG_F(ERROR, "Unable to open script descriptor: {}",
                          script_);
                    return _return(
                        R"("error", "Unable to open script descriptor"})"_json);
                }
                return yieldTo(&OpenFileCoroutine::readFile);
            }

            auto readFile() -> Action {
                json j;
                file_ >> j;
                return _return(j);
            }
        };

        class ParseJsonCoroutine
            : public oatpp::async::Coroutine<ParseJsonCoroutine> {
        private:
            std::string script_;
            json j_;
            oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res_;

        public:
            ParseJsonCoroutine(
                std::string script, json j,
                oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res)
                : script_(std::move(script)), j_(std::move(j)), res_(res) {}

            Action act() override {
                auto scriptDto = ScriptDto::createShared();
                try {
                    if (j_.contains("name") && j_["name"].is_string()) {
                        scriptDto->name = j_["name"].get<std::string>();
                    }
                    if (j_.contains("type") && j_["type"].is_string()) {
                        scriptDto->type = j_["type"].get<std::string>();
                        if (!atom::utils::contains(
                                "shell, powershell, python"_vec,
                                *scriptDto->type)) {
                            LOG_F(ERROR, "Invalid script type: {}",
                                  *scriptDto->type);
                            return finish();
                        }
                    }
                    if (j_.contains("description") &&
                        j_["description"].is_string()) {
                        scriptDto->description =
                            j_["description"].get<std::string>();
                    }
                    if (j_.contains("author") && j_["author"].is_string()) {
                        scriptDto->author = j_["author"].get<std::string>();
                    }
                    if (j_.contains("version") && j_["version"].is_string()) {
                        scriptDto->version = j_["version"].get<std::string>();
                    }
                    if (j_.contains("license") && j_["license"].is_string()) {
                        scriptDto->license = j_["license"].get<std::string>();
                    }
                    if (j_.contains("interpreter") &&
                        j_["interpreter"].is_object()) {
                        auto interpreter = j_["interpreter"];
                        if (interpreter.contains("path") &&
                            interpreter["path"].is_string()) {
                            scriptDto->interpreter->path =
                                interpreter["path"].get<std::string>();
                            if (!atom::io::isExecutableFile(
                                    scriptDto->interpreter->path, "")) {
                                LOG_F(ERROR,
                                      "Interpreter is not executable: {}",
                                      scriptDto->interpreter->path->c_str());
                                return finish();
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
                                          scriptDto->interpreter->interpreter->c_str());
                                    return finish();
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
                                LOG_F(ERROR,
                                      "Unable to get interpreter version: {}",
                                      scriptDto->interpreter->path);
                                return finish();
                            }
                            if (!lithium::checkVersion(
                                    lithium::Version::parse(interpreterVersion),
                                    *scriptDto->interpreter->version)) {
                                LOG_F(ERROR,
                                      "Interpreter version is lower than "
                                      "required: {}",
                                      scriptDto->interpreter->version);
                                return finish();
                            }
                        }
                    }
                    if (j_.contains("platform") && j_["platform"].is_string()) {
                        scriptDto->platform = j_["platform"].get<std::string>();
                        if (!atom::utils::contains("windows, linux, macos"_vec,
                                                   *scriptDto->platform)) {
                            LOG_F(ERROR, "Invalid platform: {}",
                                  *scriptDto->platform);
                            return finish();
                        }
                    }
                    if (j_.contains("permission") &&
                        j_["permission"].is_string()) {
                        scriptDto->permission =
                            j_["permission"].get<std::string>();
                        if (!atom::utils::contains("user, admin"_vec,
                                                   *scriptDto->permission)) {
                            LOG_F(ERROR, "Invalid permission: {}",
                                  *scriptDto->permission);
                            return finish();
                        }
                        if (*scriptDto->permission == "admin" &&
                            !atom::system::isRoot()) {
                            LOG_F(ERROR, "User is not admin");
                            return finish();
                        }
                    }

                    auto lineOpt = atom::io::countLinesInFile(script_);
                    if (lineOpt.has_value()) {
                        scriptDto->line = lineOpt.value();
                    }

                    if (j_.contains("args") && j_["args"].is_array()) {
                        for (const auto &arg : j_["args"]) {
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
                                        return finish();
                                    }
                                }
                                if (arg.contains("description") &&
                                    arg["description"].is_string()) {
                                    argDto->description =
                                        arg["description"].get<std::string>();
                                }
                                if (arg.contains("defaultValue") &&
                                    arg["defaultValue"].is_string()) {
                                    argDto->defaultValue =
                                        arg["defaultValue"].get<std::string>();
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
                    res_->scripts->emplace_back(scriptDto);
                } catch (const json::type_error &e) {
                    LOG_F(ERROR, "Unable to parse script value: {}", e.what());
                    return finish();
                }
                return finish();
            }
        };

        class GetScriptJsonCoroutine
            : public oatpp::async::Coroutine<GetScriptJsonCoroutine> {
        private:
            std::string script_;
            oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res_;

        public:
            GetScriptJsonCoroutine(
                std::string script,
                oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res)
                : script_(std::move(script)), res_(res) {}

            Action act() override {
                return OpenFileCoroutine::startForResult().callbackTo(
                    &GetScriptJsonCoroutine::onFileOpened);
            }

            Action onFileOpened(json j) {
                return ParseJsonCoroutine::start(script_, std::move(j), res_)
                    .next(finish());
            }
        };

        class GetScriptYamlCoroutine
            : public oatpp::async::Coroutine<GetScriptYamlCoroutine> {
        private:
            std::string script_;
            oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res_;

        public:
            GetScriptYamlCoroutine(
                std::string script,
                oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res)
                : script_(std::move(script)), res_(res) {}

            Action act() override {
                LOG_F(INFO, "Trying to load script descriptor: {}", script_);
                auto scriptDto = ScriptDto::createShared();
                try {
                    YAML::Node node = YAML::LoadFile(script_);
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
                            return finish();
                        }
                    }
                    if (node["description"] && node["description"].IsScalar()) {
                        scriptDto->description =
                            node["description"].as<std::string>();
                    }
                    if (node["author"] && node["author"].IsScalar()) {
                        scriptDto->author = node["author"].as<std::string>();
                    }
                    if (node["version"] && node["version"].IsScalar()) {
                        scriptDto->version = node["version"].as<std::string>();
                    }
                    if (node["license"] && node["license"].IsScalar()) {
                        scriptDto->license = node["license"].as<std::string>();
                    }
                    if (node["interpreter"] && node["interpreter"].IsMap()) {
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
                                return finish();
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
                                          scriptDto->interpreter->interpreter);
                                    return finish();
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
                                LOG_F(ERROR,
                                      "Unable to get interpreter version: {}",
                                      scriptDto->interpreter->path);
                                return finish();
                            }
                            if (!lithium::checkVersion(
                                    lithium::Version::parse(interpreterVersion),
                                    *scriptDto->interpreter->version)) {
                                LOG_F(ERROR,
                                      "Interpreter version is lower than "
                                      "required: {}",
                                      scriptDto->interpreter->version);
                                return finish();
                            }
                        }
                    }
                    if (node["platform"] && node["platform"].IsScalar()) {
                        scriptDto->platform =
                            node["platform"].as<std::string>();
                        if (!atom::utils::contains("windows, linux, macos"_vec,
                                                   *scriptDto->platform)) {
                            LOG_F(ERROR, "Invalid platform: {}",
                                  *scriptDto->platform);
                            return finish();
                        }
                    }
                    if (node["permission"] && node["permission"].IsScalar()) {
                        scriptDto->permission =
                            node["permission"].as<std::string>();
                        if (!atom::utils::contains("user, admin"_vec,
                                                   *scriptDto->permission)) {
                            LOG_F(ERROR, "Invalid permission: {}",
                                  *scriptDto->permission);
                            return finish();
                        }
                        if (*scriptDto->permission == "admin" &&
                            !atom::system::isRoot()) {
                            LOG_F(ERROR, "User is not admin");
                            return finish();
                        }
                    }

                    auto lineOpt = atom::io::countLinesInFile(script_);
                    if (lineOpt.has_value()) {
                        scriptDto->line = lineOpt.value();
                    }

                    if (node["args"] && node["args"].IsSequence()) {
                        for (const auto &arg : node["args"]) {
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
                                        return finish();
                                    }
                                }
                                if (arg["description"] &&
                                    arg["description"].IsScalar()) {
                                    argDto->description =
                                        arg["description"].as<std::string>();
                                }
                                if (arg["defaultValue"] &&
                                    arg["defaultValue"].IsScalar()) {
                                    argDto->defaultValue =
                                        arg["defaultValue"].as<std::string>();
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
                } catch (const YAML::ParserException &e) {
                    LOG_F(ERROR, "Unable to parse script descriptor: {}",
                          e.what());
                    return finish();
                }
                res_->scripts->emplace_back(scriptDto);
                return finish();
            }
        };

        class GetScriptXmlCoroutine
            : public oatpp::async::Coroutine<GetScriptXmlCoroutine> {
        private:
            std::string script_;
            oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res_;

        public:
            GetScriptXmlCoroutine(
                std::string script,
                oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res)
                : script_(std::move(script)), res_(res) {}

            Action act() override {
                LOG_F(INFO, "Trying to load script descriptor: {}", script_);
                tinyxml2::XMLDocument doc;
                if (doc.LoadFile(script_.c_str()) != tinyxml2::XML_SUCCESS) {
                    LOG_F(ERROR, "Unable to load script descriptor: {}",
                          script_);
                    return finish();
                }

                auto scriptDto = ScriptDto::createShared();
                auto *root = doc.FirstChildElement("script");
                if (root == nullptr) {
                    LOG_F(ERROR, "Invalid script descriptor: {}", script_);
                    return finish();
                }

                if (auto *name = root->FirstChildElement("name")) {
                    scriptDto->name = name->GetText();
                }
                if (auto *type = root->FirstChildElement("type")) {
                    scriptDto->type = type->GetText();
                    if (!atom::utils::contains("shell, powershell, python"_vec,
                                               *scriptDto->type)) {
                        LOG_F(ERROR, "Invalid script type: {}",
                              *scriptDto->type);
                        return finish();
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
                    if (auto *path = interpreter->FirstChildElement("path")) {
                        scriptDto->interpreter->path = path->GetText();
                        if (!atom::io::isExecutableFile(
                                scriptDto->interpreter->path, "")) {
                            LOG_F(ERROR, "Interpreter is not executable: {}",
                                  scriptDto->interpreter->path);
                            return finish();
                        }
                    }
                    if (auto *name = interpreter->FirstChildElement("name")) {
                        scriptDto->interpreter->interpreter = name->GetText();
                        if (scriptDto->interpreter->path->empty()) {
                            scriptDto->interpreter->path =
                                atom::system::getAppPath(
                                    scriptDto->interpreter->interpreter)
                                    .string();
                            if (scriptDto->interpreter->path == "") {
                                LOG_F(ERROR,
                                      "Unable to get interpreter path: "
                                      "{}",
                                      scriptDto->interpreter->interpreter);
                                return finish();
                            }
                        }
                    }
                    if (auto *version =
                            interpreter->FirstChildElement("version")) {
                        scriptDto->interpreter->version = version->GetText();
                        auto interpreterVersion = atom::system::getAppVersion(
                            *scriptDto->interpreter->path);
                        if (interpreterVersion.empty()) {
                            LOG_F(ERROR,
                                  "Unable to get interpreter version: {}",
                                  scriptDto->interpreter->path);
                            return finish();
                        }
                        if (!lithium::checkVersion(
                                lithium::Version::parse(interpreterVersion),
                                *scriptDto->interpreter->version)) {
                            LOG_F(ERROR,
                                  "Interpreter version is lower than "
                                  "required: {}",
                                  scriptDto->interpreter->version);
                            return finish();
                        }
                    }
                }
                if (auto *platform = root->FirstChildElement("platform")) {
                    scriptDto->platform = platform->GetText();
                    if (!atom::utils::contains("windows, linux, macos"_vec,
                                               *scriptDto->platform)) {
                        LOG_F(ERROR, "Invalid platform: {}",
                              *scriptDto->platform);
                        return finish();
                    }
                }
                if (auto *permission = root->FirstChildElement("permission")) {
                    scriptDto->permission = permission->GetText();
                    if (!atom::utils::contains("user, admin"_vec,
                                               *scriptDto->permission)) {
                        LOG_F(ERROR, "Invalid permission: {}",
                              *scriptDto->permission);
                        return finish();
                    }
                    if (*scriptDto->permission == "admin" &&
                        !atom::system::isRoot()) {
                        LOG_F(ERROR, "User is not admin");
                        return finish();
                    }
                }

                auto lineOpt = atom::io::countLinesInFile(script_);
                if (lineOpt.has_value()) {
                    scriptDto->line = lineOpt.value();
                }

                if (auto *args = root->FirstChildElement("args")) {
                    for (auto *arg = args->FirstChildElement("arg");
                         arg != nullptr; arg = arg->NextSiblingElement("arg")) {
                        auto argDto = ArgumentRequirementDto::createShared();
                        if (auto *name = arg->FirstChildElement("name")) {
                            argDto->name = name->GetText();
                        }
                        if (auto *type = arg->FirstChildElement("type")) {
                            argDto->type = type->GetText();
                            if (!atom::utils::contains(
                                    "string, int, float, bool"_vec,
                                    *argDto->type)) {
                                LOG_F(ERROR, "Invalid argument type: {}",
                                      *argDto->type);
                                return finish();
                            }
                        }
                        if (auto *description =
                                arg->FirstChildElement("description")) {
                            argDto->description = description->GetText();
                        }
                        if (auto *defaultValue =
                                arg->FirstChildElement("defaultValue")) {
                            argDto->defaultValue = defaultValue->GetText();
                        }
                        if (auto *required =
                                arg->FirstChildElement("required")) {
                            argDto->required = required->GetText() == "true";
                        }
                        scriptDto->args->emplace_back(argDto);
                    }
                }
                res_->scripts->emplace_back(scriptDto);
                return finish();
            }
        };

#define DEFINE_SCRIPT_GET_COROUTINE(COROUTINE_NAME, FILE_TYPE, GET_COROUTINE) \
    class COROUTINE_NAME : public oatpp::async::Coroutine<COROUTINE_NAME> {   \
    private:                                                                  \
        std::string scriptPath_;                                              \
        oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res_;             \
                                                                              \
    public:                                                                   \
        COROUTINE_NAME(                                                       \
            std::string scriptPath,                                           \
            oatpp::data::type::DTOWrapper<ReturnScriptListDto> &res)          \
            : scriptPath_(std::move(scriptPath)), res_(res) {}                \
                                                                              \
        Action act() override {                                               \
            auto scriptDes = atom::io::checkFileTypeInFolder(                 \
                scriptPath_, FILE_TYPE, atom::io::FileOption::PATH);          \
            oatpp::async::Executor executor;                                  \
            for (const auto &script : scriptDes) {                            \
                LOG_F(INFO, "Trying to load script descriptor: {}", script);  \
                executor.execute<GET_COROUTINE>(script, res_);                \
            }                                                                 \
            executor.waitTasksFinished();                                     \
            executor.stop();                                                  \
            executor.join();                                                  \
            return finish();                                                  \
        }                                                                     \
    };

        DEFINE_SCRIPT_GET_COROUTINE(ScriptJsonGetCoroutine, {"json"},
                                    GetScriptJsonCoroutine)
        DEFINE_SCRIPT_GET_COROUTINE(ScriptYamlGetCoroutine, {"yaml"},
                                    GetScriptYamlCoroutine)
        DEFINE_SCRIPT_GET_COROUTINE(ScriptXmlGetCoroutine, {"xml"},
                                    GetScriptXmlCoroutine)

    public:
        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestScriptListDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiScriptGetAll::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<RequestScriptListDto> &body) -> Action {
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

                oatpp::async::Executor executor;
                executor.execute<ScriptJsonGetCoroutine>(scriptPath, res);
                executor.execute<ScriptYamlGetCoroutine>(scriptPath, res);
                executor.execute<ScriptXmlGetCoroutine>(scriptPath, res);
                executor.waitTasksFinished();
                executor.stop();
                executor.join();

                // TODO: Here we need a better way to interact with oatpp and
                // nlohmann/json
                /* create serializer and deserializer configurations */
                auto serializeConfig =
                    std::make_shared<oatpp::json::Serializer::Config>();
                auto deserializeConfig =
                    std::make_shared<oatpp::json::Deserializer::Config>();
                serializeConfig->useBeautifier = true;
                auto jsonObjectMapper =
                    std::make_shared<oatpp::json::ObjectMapper>(
                        serializeConfig, deserializeConfig);
                auto jsonStr = jsonObjectMapper->writeToString(res->scripts);
                LOG_F(INFO, "Script list: {}", jsonStr);
                json j;
                try {
                    j = json::parse(jsonStr->c_str());
                } catch (const json::parse_error &e) {
                    LOG_F(ERROR, "Unable to parse script list: {}", e.what());
                    return _return(createErrorResponse(
                        "Unable to parse script list", Status::CODE_500));
                }
                std::weak_ptr<lithium::ConfigManager> configWeekPtr;
                GET_OR_CREATE_WEAK_PTR(configWeekPtr, lithium::ConfigManager,
                                       Constants::CONFIG_MANAGER);
                if (configWeekPtr.expired()) {
                    LOG_F(ERROR, "ConfigManager is not initialized");
                    return _return(createErrorResponse(
                        "ConfigManager is not initialized", Status::CODE_500));
                }

                if (configWeekPtr.lock()->setValue("/lithium/script/list", j)) {
                    LOG_F(INFO, "Save script list to config");
                } else {
                    LOG_F(ERROR, "Unable to save script list to config");
                }

                return _return(
                    controller->createDtoResponse(Status::CODE_200, res));
            } catch (const std::exception &e) {
                LOG_F(ERROR, "Unable to get script list: {}", e.what());
                return _return(createErrorResponse(e.what(), Status::CODE_500));
            }
        }
    };

    ENDPOINT_INFO(getUIApiScriptRun) {
        info->summary = "Run Script with Arguments";
        info->addConsumes<Object<RequestScriptRunDto>>("application/json");
        info->addResponse<Object<ReturnScriptRunDto>>(Status::CODE_200,
                                                      "application/json");
        info->addResponse<Object<StatusDto>>(
            Status::CODE_500, "application/json", "Unable to run script");
    }
    ENDPOINT_ASYNC("POST", "/api/script/run"_path, getUIApiScriptRun) {
        ENDPOINT_ASYNC_INIT(getUIApiScriptRun);

        static constexpr auto COMMAND = "lithium.script.run";  // Command name
    private:
        CREATE_RESPONSE_MACRO(Error, error)
        CREATE_RESPONSE_MACRO(Warning, warning)

    public:
        auto act() -> Action override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RequestScriptRunDto>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIApiScriptRun::returnResponse);
        }

        auto returnResponse(
            const oatpp::Object<RequestScriptRunDto> &body) -> Action {
            auto res = ReturnScriptRunDto::createShared();

            try {
                auto script = body->name;
                auto args = body->args;
                auto env = body->env;

                OATPP_ASSERT_HTTP((script && !script->empty()),
                                  Status::CODE_500, "Script is empty");

                res->code = 200;
                res->status = "success";
                res->message = "Run script successfully";

                auto scriptPath = atom::system::getAppPath(script);
                if (scriptPath.empty()) {
                    return _return(createErrorResponse(
                        "Unable to get script path", Status::CODE_500));
                }
            } catch (const std::exception &e) {
                return _return(createErrorResponse(e.what(), Status::CODE_500));
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  /// <-- End Code-Gen

#endif  // SCRIPTCONTROLLER_HPP
