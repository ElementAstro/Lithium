/*
 * SystemController.cpp
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

Date: 2023-7-13

Description: System Route

**************************************************/

#ifndef Lithium_SystemCONTROLLER_HPP
#define Lithium_SystemCONTROLLER_HPP

#include "atom/system/system.hpp"
#include "LithiumApp.hpp"
#include "config.h"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "atom/type/json.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController) //<- Begin Codegen

class SystemController : public oatpp::web::server::api::ApiController
{
public:
    SystemController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
    {
    }

public:
    static std::shared_ptr<SystemController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
    {
        return std::make_shared<SystemController>(objectMapper);
    }

    ENDPOINT_INFO(getUICpuUsage)
    {
        info->summary = "Get current CPU usage";
        info->addResponse<String>(Status::CODE_200, "application/json", "Usage of CPU");
        info->addResponse<String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/system/cpu", getUICpuUsage)
    {
        ENDPOINT_ASYNC_INIT(getUICpuUsage)
        Action act() override
        {
            float cpu_usage = Lithium::System::GetCpuUsage();
            nlohmann::json res;
            if (cpu_usage == 0.0)
            {
                OATPP_LOGE("System", "Failed to get cpu usage!");
                res["message"] = "Failed to get cpu usage!";
            }
            else
            {
                OATPP_LOGD("System", "Get current cpu usage : %f", cpu_usage);
                res["value"] = cpu_usage;
            }

            auto response = controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };

    ENDPOINT_INFO(getUIMemoryUsage)
    {
        info->summary = "Get current RAM usage";
        info->addResponse<String>(Status::CODE_200, "application/json", "Usage of RAM");
        info->addResponse<String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/system/memory", getUIMemoryUsage)
    {
        ENDPOINT_ASYNC_INIT(getUIMemoryUsage)
        Action act() override
        {
            float memory_usage = Lithium::System::GetMemoryUsage();
            nlohmann::json res;
            if (memory_usage == 0.0)
            {
                OATPP_LOGE("System", "Failed to get cpu usage!");
                res["message"] = "Failed to get cpu usage!";
            }
            else
            {
                OATPP_LOGD("System", "Get current memory usage : %f", memory_usage);
                res["value"] = memory_usage;
            }

            auto response = controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };

    ENDPOINT_INFO(getUICpuTemperature)
    {
        info->summary = "Get current CPU temperature";
        info->addResponse<String>(Status::CODE_200, "application/json", "Temperature of CPU");
        info->addResponse<String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/system/cpu_temp", getUICpuTemperature)
    {
        ENDPOINT_ASYNC_INIT(getUICpuTemperature)
        Action act() override
        {
            float cpu_temp = Lithium::System::GetCpuTemperature();
            nlohmann::json res;
            if (cpu_temp == 0.0)
            {
                OATPP_LOGE("System", "Failed to get cpu usage!");
                res["message"] = "Failed to get cpu usage!";
            }
            else
            {
                OATPP_LOGD("System", "Get current cpu temperature : %f", cpu_temp);
                res["value"] = cpu_temp;
            }

            auto response = controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };

    ENDPOINT_INFO(getUIDiskUsage)
    {
        info->summary = "Get current disks usage";
        info->addResponse<String>(Status::CODE_200, "application/json", "Usage of disks");
        info->addResponse<String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/system/disk", getUIDiskUsage)
    {
        ENDPOINT_ASYNC_INIT(getUIDiskUsage)
        Action act() override
        {
            nlohmann::json res;
            for (const auto &disk : Lithium::System::GetDiskUsage())
            {
                OATPP_LOGD("System", "Disk %s Usage: %f", disk.first.c_str(), disk.second);
                res["value"][disk.first] = disk.second;
            }
            auto response = controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };

    ENDPOINT_INFO(getUIProcesses)
    {
        info->summary = "Get all running processes";
        info->addResponse<String>(Status::CODE_200, "application/json", "Processes");
        info->addResponse<String>(Status::CODE_404, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/system/process", getUIProcesses)
    {
        ENDPOINT_ASYNC_INIT(getUIProcesses)
        Action act() override
        {
            nlohmann::json res;
            for (const auto &process : Lithium::System::GetProcessInfo())
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
#ifdef _WIN32
            Lithium::MyApp->createProcess("Start-Sleep -Seconds 1; Stop-Computer","shutdown");
#else
            Lithium::MyApp->createProcess("sleep 1 && shutdown -h now","shutdown");
#endif
            auto response = controller->createResponse(Status::CODE_200);
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
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
#ifdef _WIN32
            Lithium::MyApp->createProcess("Start-Sleep -Seconds 1; Restart-Computer","reboot");
#else
            Lithium::MyApp->createProcess("sleep 1 && reboot","reboot");
#endif
            auto response = controller->createResponse(Status::CODE_200);
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };

    ENDPOINT_INFO(getUISleep)
    {
        info->summary = "Sleep system";
    }
    ENDPOINT_ASYNC("GET", "/api/system/sleep", getUISleep)
    {
        ENDPOINT_ASYNC_INIT(getUISleep)
        Action act() override
        {
    #ifdef _WIN32
            Lithium::MyApp->createProcess("Start-Sleep -Seconds 5", "sleep");
    #else
            Lithium::MyApp->createProcess("sleep 5", "sleep");
    #endif
            auto response = controller->createResponse(Status::CODE_200);
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };

};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // Lithium_SystemCONTROLLER_HPP