/*
 * AsyncProcessController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Process Route

**************************************************/

#ifndef LITHIUM_ASYNC_PROCESS_CONTROLLER_HPP
#define LITHIUM_ASYNC_PROCESS_CONTROLLER_HPP

#include "config.h"

#include "data/ProcessDto.hpp"
#include "data/StatusDto.hpp"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include <cstdlib>
#include <string>

#include "atom/server/global_ptr.hpp"
#include "atom/system/process.hpp"

void replaceAll(std::string &str, auto &from, auto &to) {
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class ProcessController : public oatpp::web::server::api::ApiController {
private:
    static std::shared_ptr<Atom::System::ProcessManager> m_processManager;

public:
    ProcessController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper) {
    }

public:
    static std::shared_ptr<ProcessController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<ProcessController>(objectMapper);
    }

    ENDPOINT_INFO(getUICreateProcess) {
        info->summary = "Create Process with process name and id";
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_400, "text/text");
        info->pathParams.add<String>("process-name").description =
            "Name of the process want to start (must be available to execute)";
        info->pathParams.add<String>("process-id").description =
            "ID of the process , used to stop or get output";
    }
    ENDPOINT_ASYNC("GET", "process/start/{process-name}/{process-id}",
                   getUICreateProcess) {
        ENDPOINT_ASYNC_INIT(getUICreateProcess);
        Action act() override {
            auto res = StatusDto::createShared();
            res->command = "createProcess";
            auto processName = request->getPathVariable("process-name");
            auto processId = request->getPathVariable("process-id");
            OATPP_ASSERT_HTTP(processName != "" && processId != "",
                              Status::CODE_400,
                              "process name and id should not be null");
            if (!m_processManager->createProcess(processName, processId)) {
                res->error = "Operate Error";
                res->message = "Failed to create process";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUICreateProcessAPI) {
        info->summary = "Create Process with process name and id";
        info->addConsumes<Object<CreateProcessDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_400, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/process/start", getUICreateProcessAPI) {
        ENDPOINT_ASYNC_INIT(getUICreateProcessAPI);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<CreateProcessDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUICreateProcessAPI::returnResponse);
        }
        Action returnResponse(const oatpp::Object<CreateProcessDTO> &body) {
            auto res = StatusDto::createShared();
            res->command = "createProcess";
            auto processName = body->process_name.getValue("");
            auto processId = body->process_id.getValue("");
            OATPP_ASSERT_HTTP(processName != "" && processId != "",
                              Status::CODE_400,
                              "process name and id should not be null");
            if (!m_processManager->createProcess(processName, processId)) {
                res->error = "Process Failed";
                res->message = "Failed to create process";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIStopProcess) {
        info->summary = "Stop Process with id";
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_400, "text/plain");
        info->pathParams.add<String>("process-id").description =
            "ID of the process";
    }
    ENDPOINT_ASYNC("GET", "process/stop/{process-id}", getUIStopProcess) {
        ENDPOINT_ASYNC_INIT(getUIStopProcess);
        Action act() override {
            auto res = StatusDto::createShared();
            res->command = "terminateProcess";
            auto processId = std::stoi(request->getPathVariable("process-id"));
            OATPP_ASSERT_HTTP(processId, Status::CODE_400,
                              "process id should not be null");
            if (!m_processManager->terminateProcess(processId)) {
                res->error = "Operate Error";
                res->message = "Failed to create process";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUITerminateProcessAPI) {
        info->summary = "Terminate process with process and id";
        info->addConsumes<Object<TerminateProcessDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_400, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/process/stop", getUITerminateProcessAPI) {
        ENDPOINT_ASYNC_INIT(getUITerminateProcessAPI)

            ;
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<TerminateProcessDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUITerminateProcessAPI::returnResponse);
        }

        Action returnResponse(const oatpp::Object<TerminateProcessDTO> &body) {
            auto res = StatusDto::createShared();
            res->command = "terminateProcess";
            auto processId = body->process_id.getValue("");
            OATPP_ASSERT_HTTP(processId != "", Status::CODE_400,
                              "process name and id should not be null");
            if (!m_processManager->terminateProcessByName(processId)) {
                res->error = "Process Failed";
                res->message = "Failed to terminate process";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRunScript) {
        info->summary = "Run script with script name and running id";
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_400, "text/plain");
        info->pathParams.add<String>("script-name").description =
            "Name of the script want to start (must be available to execute)";
        info->pathParams.add<String>("script-id").description =
            "ID of the script , used to stop or get output";
    }
    ENDPOINT_ASYNC("GET", "process/start/{script-name}/{script-id}",
                   getUIRunScript) {
        ENDPOINT_ASYNC_INIT(getUIRunScript)

            ;
        Action act() override {
            auto res = StatusDto::createShared();
            res->command = "runScript";
            auto scriptName = request->getPathVariable("script-name");
            auto scriptId = request->getPathVariable("script-id");
            OATPP_ASSERT_HTTP(scriptName && scriptId, Status::CODE_400,
                              "script name and id should not be null");
            if (!m_processManager->runScript(scriptName, scriptId)) {
                res->error = "Operate Error";
                res->message = "Failed to run script";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRunScriptAPI) {
        info->summary = "Run script with process name and id";
        info->addConsumes<Object<RunScriptDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "text/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_400, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/process/run", getUIRunScriptAPI) {
        ENDPOINT_ASYNC_INIT(getUIRunScriptAPI)

            ;
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RunScriptDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIRunScriptAPI::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RunScriptDTO> &body) {
            auto res = StatusDto::createShared();
            res->command = "runScript";
            auto scriptId = body->script_id.getValue("");
            auto scriptName = body->script_name.getValue("");
            OATPP_ASSERT_HTTP(scriptId != "" && scriptName != "",
                              Status::CODE_400,
                              "script name and id should not be null");
            if (!m_processManager->runScript(scriptName, scriptId)) {
                res->error = "Process Failed";
                res->message = "Failed to start script";
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };
};

std::shared_ptr<Atom::System::ProcessManager> ProcessController::m_processManager =
    GetPtr<Atom::System::ProcessManager>("lithium.system.process");

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_PROCESS_CONTROLLER_HPP