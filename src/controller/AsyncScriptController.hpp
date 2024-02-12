/*
 * AsyncScriptController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-11-18

Description: Async Script Controller

**************************************************/

#ifndef Lithium_SCRIPTCONTROLLER_HPP
#define Lithium_SCRIPTCONTROLLER_HPP

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "data/ScriptDto.hpp"
#include "data/StatusDto.hpp"

#include "LithiumApp.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class ScriptController : public oatpp::web::server::api::ApiController
{
public:
    ScriptController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    // ----------------------------------------------------------------
    // Pointer creator
    // ----------------------------------------------------------------
    /**
     * Create shared pointer to &id:oatpp::web::server::api::ApiController;.
     * @param objectMapper - &id:oatpp::data::mapping::ObjectMapper;.
     */
    static std::shared_ptr<ScriptController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<ScriptController>(objectMapper);
    }

    /**
     * Create unique pointer to &id:oatpp::web::server::api::ApiController;.
     * @param objectMapper - &id:oatpp::data::mapping::ObjectMapper;.
     */
    static std::unique_ptr<ScriptController> createUnique(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_unique<ScriptController>(objectMapper);
    }

public:
    // ----------------------------------------------------------------
    // Script Http Handler
    // ----------------------------------------------------------------


    ENDPOINT_INFO(getUIRunScript)
    {
        info->summary = "Run a single line script and get the result";
        info->addConsumes<Object<RunCScriptDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/script/run", getUIRunScript)
    {
        ENDPOINT_ASYNC_INIT(getUIRunScript)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RunCScriptDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIRunScript::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RunCScriptDTO>& body)
        {
            auto res = StatusDto::createShared();

            auto script = body->script.getValue("");
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRunCScriptFile)
    {
        info->summary = "Run a script file and get the result";
        info->addConsumes<Object<RunCScriptFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/script/run", getUIRunCScriptFile)
    {
        ENDPOINT_ASYNC_INIT(getUIRunCScriptFile)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RunCScriptFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIRunCScriptFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RunCScriptFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    // ----------------------------------------------------------------
    // Some useful Functions about chaiscript
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUICheckScriptFile)
    {
        info->summary = "Check script file";
        info->addConsumes<Object<RunCScriptFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/apt/script/check",getUICheckScriptFile)
    {
        ENDPOINT_ASYNC_INIT(getUICheckScriptFile)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RunCScriptFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUICheckScriptFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RunCScriptFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIGetScriptFile)
    {
        info->summary = "Get script file";
        info->addConsumes<Object<RunCScriptFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/script/get", getUIGetScriptFile)
    {
        ENDPOINT_ASYNC_INIT(getUIGetScriptFile)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RunCScriptFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIGetScriptFile::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RunCScriptFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIListScriptFiles)
    {
        info->summary = "List script files";
        info->addConsumes<Object<RunCScriptFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/script/list", getUIListScriptFiles)
    {
        ENDPOINT_ASYNC_INIT(getUIListScriptFiles)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RunCScriptFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIListScriptFiles::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RunCScriptFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUILoadScript)
    {
        info->summary = "Load script into cache";
        info->addConsumes<Object<RunCScriptFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/script/load", getUILoadScript)
    {
        ENDPOINT_ASYNC_INIT(getUILoadScript)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RunCScriptFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUILoadScript::returnResponse);
        }
        Action returnResponse(const oatpp::Object<RunCScriptFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIUnloadScriptFile)
    {
        info->summary = "Unload script from cache";
        info->addConsumes<Object<RunCScriptFileDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/script/unload", getUIUnloadScriptFile)
    {
        ENDPOINT_ASYNC_INIT(getUIUnloadScriptFile)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<RunCScriptFileDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUIUnloadScriptFile::returnResponse);
        }
        Action returnResponse(const oatpp::Object<RunCScriptFileDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_SCRIPTCONTROLLER_HPP