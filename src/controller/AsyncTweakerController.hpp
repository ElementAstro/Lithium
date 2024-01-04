/*
 * AsyncTweakerController.hpp
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

Date: 2023-11-17

Description: Tweaker Router to fit LightAPT (just a way)

**************************************************/

#ifndef Lithium_TWEAKERCONTROLLER_HPP
#define Lithium_TWEAKERCONTROLLER_HPP

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "data/StatusDto.hpp"
#include "data/TweakerDto.hpp"

#include "LithiumApp.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class TweakerController : public oatpp::web::server::api::ApiController
{
public:
    TweakerController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<TweakerController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<TweakerController>(objectMapper);
    }

public:

    // ----------------------------------------------------------------
    // Http Handler Interface of LightAPT
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUITDeviceConnect)
    {
        info->summary = "Connect to device";
        info->addConsumes<Object<TDeviceConnectDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/device_connect", getUITDeviceConnect)
    {
        ENDPOINT_ASYNC_INIT(getUITDeviceConnect)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<TDeviceConnectDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUITDeviceConnect::returnResponse);
        }

        Action returnResponse(const oatpp::Object<TDeviceConnectDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUITDriverConnect)
    {
        info->summary = "Connect to device";
        info->addConsumes<Object<TDriverConnectDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/driver_connect", getUITDriverConnect)
    {
        ENDPOINT_ASYNC_INIT(getUITDriverConnect)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<TDriverConnectDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUITDriverConnect::returnResponse);
        }

        Action returnResponse(const oatpp::Object<TDriverConnectDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUITGlobalParameter)
    {
        info->summary = "Connect to device";
        info->addConsumes<Object<TGlobalParameterDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/GlobalParameter", getUITGlobalParameter)
    {
        ENDPOINT_ASYNC_INIT(getUITGlobalParameter)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<TGlobalParameterDTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUITGlobalParameter::returnResponse);
        }

        Action returnResponse(const oatpp::Object<TGlobalParameterDTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUITPHD2)
    {
        info->summary = "Connect to device";
        info->addConsumes<Object<TPHD2DTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/phd2", getUITPHD2)
    {
        ENDPOINT_ASYNC_INIT(getUITPHD2)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<TPHD2DTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUITPHD2::returnResponse);
        }

        Action returnResponse(const oatpp::Object<TPHD2DTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUITPAA)
    {
        info->summary = "Connect to device";
        info->addConsumes<Object<TPAADTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200, "application/json");
    }
    ENDPOINT_ASYNC("GET", "/phd2", getUITPAA)
    {
        ENDPOINT_ASYNC_INIT(getUITPAA)
        Action act() override
        {
            return request->readBodyToDtoAsync<oatpp::Object<TPAADTO>>(controller->getDefaultObjectMapper()).callbackTo(&getUITPAA::returnResponse);
        }

        Action returnResponse(const oatpp::Object<TPAADTO>& body)
        {
            auto res = StatusDto::createShared();
            
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_TWEAKERCONTROLLER_HPP