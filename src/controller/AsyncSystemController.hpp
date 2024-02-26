/*
 * SystemController.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: System Route

**************************************************/

#ifndef LITHIUM_ASYNC_SYSTEM_CONTROLLER_HPP
#define LITHIUM_ASYNC_SYSTEM_CONTROLLER_HPP

#include "atom/system/system.hpp"
#include "atom/system/module/cpu.hpp"
#include "atom/system/module/memory.hpp"
#include "atom/system/module/disk.hpp"
#include "atom/system/module/battery.hpp"

#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "data/SystemDto.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class SystemController : public oatpp::web::server::api::ApiController
{
public:
    SystemController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

    // ----------------------------------------------------------------
    // Pointer creator
    // ----------------------------------------------------------------

    static std::shared_ptr<SystemController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    {
        return std::make_shared<SystemController>(objectMapper);
    }

    // ----------------------------------------------------------------
    // System Http Handler
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUICpuUsage)
    {
        info->summary = "Get current CPU usage";
        // info->addResponse<StatusDto>(Status::CODE_200, "application/json", "Usage of CPU");
    }
    ENDPOINT_ASYNC("GET", "/api/system/cpu", getUICpuUsage)
    {
        ENDPOINT_ASYNC_INIT(getUICpuUsage)
        Action act() override
        {
            auto res = BaseReturnSystemDto::createShared();
            if(float cpu_usage = Atom::System::getCurrentCpuUsage(); cpu_usage <= 0.0f)
            {
                res->status = "error";
                res->message = "Failed to get current CPU usage";
                res->error = "System Error";
                res->code = 500;
            }
            else
            {
                res->status = "success";
                res->code = 200;
                res->value = cpu_usage;
                res->message = "Success get current CPU usage";
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIMemoryUsage)
    {
        info->summary = "Get current RAM usage";
        // info->addResponse<StatusDto>(Status::CODE_200, "application/json", "Usage of RAM");
    }
    ENDPOINT_ASYNC("GET", "/api/system/memory", getUIMemoryUsage)
    {
        ENDPOINT_ASYNC_INIT(getUIMemoryUsage)
        Action act() override
        {
            auto res = BaseReturnSystemDto::createShared();
            if(float memory_usage = Atom::System::getMemoryUsage(); memory_usage <= 0.0f)
            {
                res->code = 500;
                res->status = "error";
                res->message = "Failed to get current RAM usage";
                res->error = "System Error";
            }
            else
            {
                res->status = "success";
                res->code = 200;
                res->value = memory_usage;
                res->message = "Success get current RAM usage";
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUICpuTemperature)
    {
        info->summary = "Get current CPU temperature";
        // info->addResponse<StatusDto>(Status::CODE_200, "application/json", "Temperature of CPU");
    }
    ENDPOINT_ASYNC("GET", "/api/system/cpu_temp", getUICpuTemperature)
    {
        ENDPOINT_ASYNC_INIT(getUICpuTemperature)
        Action act() override
        {
            auto res = BaseReturnSystemDto::createShared();
            if(float cpu_temp = Atom::System::getCurrentCpuTemperature(); cpu_temp <= 0.0f)
            {
                res->code = 500;
                res->status = "error";
                res->message = "Failed to get current CPU temperature";
                res->error = "System Error";
            }
            else
            {
                res->status = "success";
                res->code = 200;
                res->value = cpu_temp;
                res->message = "Success get current CPU temperature";
            }
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIDiskUsage)
    {
        info->summary = "Get current disks usage";
        // info->addResponse<StatusDto>(Status::CODE_200, "application/json", "Usage of disks");
    }
    ENDPOINT_ASYNC("GET", "/api/system/disk", getUIDiskUsage)
    {
        ENDPOINT_ASYNC_INIT(getUIDiskUsage)
        Action act() override
        {
            auto res = ReturnDiskUsageDto::createShared();
            if (const auto tmp = Atom::System::getDiskUsage(); tmp.empty())
            {
                res->code = 500;
                res->status = "error";
                res->message = "Failed to get current disks usage";
                res->error = "System Error";
            }
            else
            {
                for (auto &disk : tmp)
                {
                    // TODO: Add disk usage to response
                }
                res->status = "success";
                res->code = 200;
                res->message = "Success get current disks usage";
            }
            /*
            for (const auto &disk : Atom::System::GetDiskUsage())
            {
                OATPP_LOGD("System", "Disk %s Usage: %f", disk.first.c_str(), disk.second);
                res["value"][disk.first] = disk.second;
            }
            */
            return _return(controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIProcesses)
    {
        info->summary = "Get all running processes";
        // info->addResponse<String>(Status::CODE_200, "application/json", "Processes");
        // info->addResponse<String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/system/process", getUIProcesses)
    {
        ENDPOINT_ASYNC_INIT(getUIProcesses)
        Action act() override
        {
            nlohmann::json res;
            for (const auto &process : Atom::System::GetProcessInfo())
            {
                OATPP_LOGD("System", "Process Name: %s File Address: %s", process.first.c_str(), process.second.c_str());
                res["value"][process.first] = process.second;
            }
            auto response = controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };

    ENDPOINT_INFO(getUIShutdown)
    {
        info->summary = "Shutdown system";
    }
    ENDPOINT_ASYNC("GET", "/api/system/shutdown", getUIShutdown)
    {
        ENDPOINT_ASYNC_INIT(getUIShutdown)
        Action act() override
        {
            Atom::System::Shutdown();
            return _return(controller->createResponse(Status::CODE_200, "Wtf, how can you do that?"));
        }
    };

    ENDPOINT_INFO(getUIReboot)
    {
        info->summary = "Reboot system";
    }
    ENDPOINT_ASYNC("GET", "/api/system/reboot", getUIReboot)
    {
        ENDPOINT_ASYNC_INIT(getUIReboot)
        Action act() override
        {
            Atom::System::Reboot();
            return _return(controller->createResponse(Status::CODE_200, "Wtf, how can you do that?"));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // LITHIUM_ASYNC_SYSTEM_CONTROLLER_HPP