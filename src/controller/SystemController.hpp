#ifndef OPENAPT_SystemCONTROLLER_HPP
#define OPENAPT_SystemCONTROLLER_HPP

#include "modules/system/system.hpp"

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"

#include "nlohmann/json.hpp"

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
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)
    )
    {
        return std::make_shared<SystemController>(objectMapper);
    }

    ENDPOINT("GET", "/api/system/cpu", getUICpuUsage)
    {
        float cpu_usage = OpenAPT::System::GetCpuUsage();
        nlohmann::json res;
        if (cpu_usage == 0.0)
        {
            OATPP_LOGE("System","Failed to get cpu usage!");
            res["message"] = "Failed to get cpu usage!";
        }
        else
        {
            OATPP_LOGD("System", "Get current cpu usage : %f", cpu_usage);
            res["value"] = cpu_usage;
        }
        
        auto response = createResponse(Status::CODE_200, res.dump());
        response->putHeader(Header::CONTENT_TYPE, "text/json");
        return response;
    }

    ENDPOINT("GET", "/api/system/memory", getUIMemoryUsage)
    {
        float memory_usage = OpenAPT::System::GetMemoryUsage();
        nlohmann::json res;
        if (memory_usage == 0.0)
        {
            OATPP_LOGE("System","Failed to get cpu usage!");
            res["message"] = "Failed to get cpu usage!";
        }
        else
        {
            OATPP_LOGD("System", "Get current memory usage : %f", memory_usage);
            res["value"] = memory_usage;
        }
        
        auto response = createResponse(Status::CODE_200, res.dump());
        response->putHeader(Header::CONTENT_TYPE, "text/json");
        return response;
    }

    ENDPOINT("GET", "/api/system/cpu_temp", getUICpuTemperature)
    {
        float cpu_temp = OpenAPT::System::GetCpuTemperature();
        nlohmann::json res;
        if (cpu_temp == 0.0)
        {
            OATPP_LOGE("System","Failed to get cpu usage!");
            res["message"] = "Failed to get cpu usage!";
        }
        else
        {
            OATPP_LOGD("System", "Get current cpu temperature : %f", cpu_temp);
            res["value"] = cpu_temp;
        }
        
        auto response = createResponse(Status::CODE_200, res.dump());
        response->putHeader(Header::CONTENT_TYPE, "text/json");
        return response;
    }

    ENDPOINT("GET", "/api/system/disk", getUIDiskUsage)
    {
        nlohmann::json res;
        for (const auto &disk : OpenAPT::System::GetDiskUsage())
        {
            OATPP_LOGD("System","Disk %s Usage: %f %",disk.first.c_str(),disk.second);
            res["value"][disk.first] = disk.second;
        }
        auto response = createResponse(Status::CODE_200, res.dump());
        response->putHeader(Header::CONTENT_TYPE, "text/json");
        return response;
    }

    ENDPOINT("GET", "/api/system/process", getUIProcesses)
    {
        nlohmann::json res;
        for (const auto &process : OpenAPT::System::GetProcessInfo())
        {
            OATPP_LOGD("System","Process Name: %s File Address: %s",process.first.c_str(),process.second.c_str());
            res["value"][process.first] = process.second;
        }
        auto response = createResponse(Status::CODE_200, res.dump());
        response->putHeader(Header::CONTENT_TYPE, "text/json");
        return response;
    }
};

#include OATPP_CODEGEN_END(ApiController) //<- End Codegen

#endif // OPENAPT_SystemCONTROLLER_HPP