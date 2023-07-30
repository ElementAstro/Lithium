#ifndef Lithium_PROCESSCONTROLLER_HPP
#define Lithium_PROCESSCONTROLLER_HPP

#include "LithiumApp.hpp"

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "nlohmann/json.hpp"

#include <string>
#include <cstdlib>

void replaceAll(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class ProcessController : public oatpp::web::server::api::ApiController
{
public:
    ProcessController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<ProcessController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    {
        return std::make_shared<ProcessController>(objectMapper);
    }

    ENDPOINT_INFO(getUICreateProcess)
    {
        info->summary = "Create Process with process name and id";
        info->addResponse<String>(Status::CODE_200, "text/json");
        info->addResponse<String>(Status::CODE_400, "text/plain");
        info->pathParams.add<String>("process-name").description = "Name of the process want to start (must be available to execute)";
        info->pathParams.add<String>("process-id").description = "ID of the process , used to stop or get output";
    }
#if ENABLE_ASYNC

    ENDPOINT_ASYNC("GET", "process/start/{process-name}/{process-id}", getUICreateProcess){

        ENDPOINT_ASYNC_INIT(getUICreateProcess)

        Action act() override {
            nlohmann::json res;
            res["command"] = "CreateProcess";
            auto processName = request->getPathVariable("process-name");
            auto processId = request->getPathVariable("process-id");
            OATPP_ASSERT_HTTP(processName && processId, Status::CODE_400, "process name and id should not be null");
            if (!Lithium::MyApp->createProcess(processName, processId))
            {
                res["error"] = "Operate Error";
                res["message"] = "Failed to create process";
            }
            auto response = controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };
#else
    ENDPOINT("GET", "process/start/{process-name}/{process-id}", getUIStopProcess)
    {
        nlohmann::json res;
        res["command"] = "CreateProcess";
        auto processName = request->getPathVariable("process-name");
        auto processId = request->getPathVariable("process-id");
        OATPP_ASSERT_HTTP(processName && processId, Status::CODE_400, "process name and id should not be null");
        if (!Lithium::MyApp->createProcess(processName, processId))
        {
            res["error"] = "Operate Error";
            res["message"] = "Failed to create process";
        }
        auto response = createResponse(Status::CODE_200, res.dump());
        response->putHeader(Header::CONTENT_TYPE, "text/json");
        return response;
    }
#endif

    ENDPOINT_INFO(getUIStopProcess)
    {
        info->summary = "Stop Process with id";
        info->addResponse<String>(Status::CODE_200, "text/json");
        info->addResponse<String>(Status::CODE_400, "text/plain");
        info->pathParams.add<String>("process-id").description = "ID of the process";
    }
#if ENABLE_ASYNC
    ENDPOINT_ASYNC("GET", "process/stop/{process-id}", getUIStopProcess)
    {
        ENDPOINT_ASYNC_INIT(getUIStopProcess)
        Action act() override {
            nlohmann::json res;
            res["command"] = "TerminateProcess";
            auto processId = std::stoi(request->getPathVariable("process-id"));
            OATPP_ASSERT_HTTP(processId, Status::CODE_400, "process id should not be null");
            if (!Lithium::MyApp->terminateProcess(processId))
            {
                res["error"] = "Operate Error";
                res["message"] = "Failed to create process";
            }
            auto response = controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };
#else
    ENDPOINT("GET", "process/stop/{process-id}", getUIStopProcess,
            PATH(String,processId))
    {
        nlohmann::json res;
        res["command"] = "TerminateProcess";
        OATPP_ASSERT_HTTP(processId, Status::CODE_400, "process id should not be null");
        if (!Lithium::MyApp->terminateProcess(processId->getValue()))
        {
            res["error"] = "Operate Error";
            res["message"] = "Failed to create process";
        }
        auto response = createResponse(Status::CODE_200, res.dump());
        response->putHeader(Header::CONTENT_TYPE, "text/json");
        return response;
    }
#endif

    ENDPOINT_INFO(getUIRunScript)
    {
        info->summary = "Run script with script name and running id";
        info->addResponse<String>(Status::CODE_200, "text/json");
        info->addResponse<String>(Status::CODE_400, "text/plain");
        info->pathParams.add<String>("script-name").description = "Name of the script want to start (must be available to execute)";
        info->pathParams.add<String>("script-id").description = "ID of the script , used to stop or get output";
    }
#if ENABLE_ASYNC

    ENDPOINT_ASYNC("GET", "process/start/{script-name}/{script-id}", getUIRunScript){

        ENDPOINT_ASYNC_INIT(getUIRunScript)

        Action act() override {
            nlohmann::json res;
            res["command"] = "RunScript";
            auto scriptName = request->getPathVariable("script-name");
            auto scriptId = request->getPathVariable("script-id");
            OATPP_ASSERT_HTTP(scriptName && scriptId, Status::CODE_400, "script name and id should not be null");
            if (!Lithium::MyApp->runScript(scriptName, scriptId))
            {
                res["error"] = "Operate Error";
                res["message"] = "Failed to run script";
            }
            auto response = controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };
#else
    ENDPOINT_ASYNC("GET", "process/start/{script-name}/{script-id}", getUIRunScript)
    {
        nlohmann::json res;
        res["command"] = "RunScript";
        auto scriptName = request->getPathVariable("script-name");
        auto scriptId = request->getPathVariable("script-id");
        OATPP_ASSERT_HTTP(scriptName && scriptId, Status::CODE_400, "script name and id should not be null");
        if (!Lithium::MyApp->runScript(scriptName, scriptId))
        {
            res["error"] = "Operate Error";
            res["message"] = "Failed to run script";
        }
        auto response = createResponse(Status::CODE_200, res.dump());
        response->putHeader(Header::CONTENT_TYPE, "text/json");
        return response;
    }
#endif

};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_PROCESSCONTROLLER_HPP