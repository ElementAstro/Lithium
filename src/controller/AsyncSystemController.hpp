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

#include "atom/system/module/battery.hpp"
#include "atom/system/module/cpu.hpp"
#include "atom/system/module/disk.hpp"
#include "atom/system/module/memory.hpp"
#include "atom/system/module/wifi.hpp"
#include "atom/system/system.hpp"

#include "config.h"

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "data/SystemDto.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class SystemController : public oatpp::web::server::api::ApiController {
public:
    SystemController(const std::shared_ptr<ObjectMapper> &objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper) {}

    // ----------------------------------------------------------------
    // Pointer creator
    // ----------------------------------------------------------------

    static std::shared_ptr<SystemController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<SystemController>(objectMapper);
    }

    // ----------------------------------------------------------------
    // System Http Handler
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUICpuUsage) {
        info->summary = "Get current CPU usage";
        info->addResponse<Object<BaseReturnSystemDto>>(
            Status::CODE_200, "application/json", "Usage of CPU");
    }
    ENDPOINT_ASYNC("GET", "/api/system/cpu_usage", getUICpuUsage) {
        ENDPOINT_ASYNC_INIT(getUICpuUsage);
        Action act() override {
            auto res = BaseReturnSystemDto::createShared();
            res->command = "getUICpuUsage";
            if (float cpu_usage = Atom::System::getCurrentCpuUsage();
                cpu_usage <= 0.0f) {
                res->status = "error";
                res->message = "Failed to get current CPU usage";
                res->error = "System Error";
                res->code = 500;
            } else {
                res->status = "success";
                res->code = 200;
                res->value = cpu_usage;
                res->message = "Success get current CPU usage";
            }
            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUICpuTemperature) {
        info->summary = "Get current CPU temperature";
        info->addResponse<Object<BaseReturnSystemDto>>(
            Status::CODE_200, "application/json", "Temperature of CPU");
    }
    ENDPOINT_ASYNC("GET", "/api/system/cpu_temp", getUICpuTemperature) {
        ENDPOINT_ASYNC_INIT(getUICpuTemperature);
        Action act() override {
            auto res = BaseReturnSystemDto::createShared();
            res->command = "getUICpuTemperature";
            if (float cpu_temp = Atom::System::getCurrentCpuTemperature();
                cpu_temp <= 0.0f) {
                res->code = 500;
                res->status = "error";
                res->message = "Failed to get current CPU temperature";
                res->error = "System Error";
            } else {
                res->status = "success";
                res->code = 200;
                res->value = cpu_temp;
                res->message = "Success get current CPU temperature";
            }
            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUICpuInfo) {
        info->summary = "Get current CPU model";
        info->addResponse<Object<ReturnCpuInfoDto>>(
            Status::CODE_200, "application/json", "Model of CPU");
    }
    ENDPOINT_ASYNC("GET", "/api/system/cpu_model", getUICpuInfo) {
        ENDPOINT_ASYNC_INIT(getUICpuInfo);
        Action act() override {
            auto res = ReturnCpuInfoDto::createShared();
            res->command = "getUICpuInfo";

            auto cpu_model = Atom::System::getCPUModel();
            auto cpu_freq = Atom::System::getProcessorFrequency();
            auto cpu_id = Atom::System::getProcessorIdentifier();
            auto cpu_package = Atom::System::getNumberOfPhysicalPackages();
            auto cpu_core = Atom::System::getNumberOfPhysicalCPUs();

            if (cpu_model.empty() || cpu_freq <= 0.0f || cpu_id.empty() ||
                cpu_package <= 0 || cpu_core <= 0) [[unlikely]] {
                res->code = 500;
                res->status = "error";
                res->error = "System Error";

                if (cpu_model.empty()) {
                    res->message = "Failed to get current CPU model";
                } else if (cpu_freq <= 0.0f) {
                    res->message = "Failed to get current processor frequency";
                } else if (cpu_id.empty()) {
                    res->message = "Failed to get current processor identifier";
                } else if (cpu_package <= 0) {
                    res->message = "Failed to get current processor package";
                } else if (cpu_core <= 0) {
                    res->message = "Failed to get current processor core";
                } else {
                    res->message =
                        "Failed to get current processor information";
                }
            } else [[likely]] {
                res->status = "success";
                res->code = 200;
                res->model = cpu_model;
                res->frequency = cpu_freq;
                res->identifier = cpu_id;
                res->packages = cpu_package;
                res->cpus = cpu_core;
                res->message = "Success get current processor information";
            }

            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIMemoryUsage) {
        info->summary = "Get current RAM usage";
        info->addResponse<Object<BaseReturnSystemDto>>(
            Status::CODE_200, "application/json", "Usage of RAM");
    }
    ENDPOINT_ASYNC("GET", "/api/system/memory_usage", getUIMemoryUsage) {
        ENDPOINT_ASYNC_INIT(getUIMemoryUsage);
        Action act() override {
            auto res = BaseReturnSystemDto::createShared();
            res->command = "getUIMemoryUsage";

            auto memory_usage = Atom::System::getMemoryUsage();
            if (memory_usage <= 0.0f) {
                res->code = 500;
                res->status = "error";
                res->message = "Failed to get current RAM usage";
                res->error = "System Error";
            } else {
                res->status = "success";
                res->code = 200;
                res->value = memory_usage;
                res->message = "Success get current RAM usage";
            }
            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIMemoryInfo) {
        info->summary = "Get memory static info";
        info->addResponse<Object<ReturnMemoryInfoDto>>(
            Status::CODE_200, "application/json",
            "Info of memory (usually static infomation)");
    }
    ENDPOINT_ASYNC("GET", "/api/system/memory_info", getUIMemoryInfo) {
        ENDPOINT_ASYNC_INIT(getUIMemoryInfo);
        Action act() override {
            auto res = ReturnMemoryInfoDto::createShared();
            res->command = "getUIMemoryInfo";

            auto total_memory = Atom::System::getTotalMemorySize();
            auto available_memory = Atom::System::getAvailableMemorySize();
            auto virtual_memory_max = Atom::System::getVirtualMemoryMax();
            auto virtual_memory_used = Atom::System::getVirtualMemoryUsed();
            auto swap_memory_total = Atom::System::getSwapMemoryTotal();
            auto swap_memory_used = Atom::System::getSwapMemoryUsed();

            auto physical_memory = Atom::System::getPhysicalMemoryInfo();

            if (total_memory <= 0 || available_memory <= 0 ||
                virtual_memory_max <= 0 || virtual_memory_used <= 0 ||
                swap_memory_total <= 0 || swap_memory_used <= 0) [[unlikely]] {
                res->code = 500;
                res->status = "error";
                res->message = "Failed to get memory info";
                res->error = "System Error";
            } else [[likely]] {
                res->status = "success";
                res->code = 200;
                res->total_memory = total_memory;
                res->available_memory = available_memory;
                res->virtual_memory_max = virtual_memory_max;
                res->virtual_memory_used = virtual_memory_used;
                res->swap_memory_total = swap_memory_total;
                res->swap_memory_used = swap_memory_used;
                if (!physical_memory.capacity.empty() &&
                    !physical_memory.clockSpeed.empty() &&
                    !physical_memory.manufacturer.empty() &&
                    !physical_memory.type.empty()) {
                    res->memory_slot["capacity"] = physical_memory.capacity;
                    res->memory_slot["clockSpeed"] = physical_memory.clockSpeed;
                    res->memory_slot["type"] = physical_memory.type;
                }
                res->message = "Success get memory info";
            }

            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIDiskUsage) {
        info->summary = "Get current disks usage";
        info->addResponse<Object<ReturnDiskUsageDto>>(
            Status::CODE_200, "application/json", "Usage of disks");
    }
    ENDPOINT_ASYNC("GET", "/api/system/disk_usage", getUIDiskUsage) {
        ENDPOINT_ASYNC_INIT(getUIDiskUsage);
        Action act() override {
            auto res = ReturnDiskUsageDto::createShared();
            res->command = "getUIDiskUsage";

            auto tmp = Atom::System::getDiskUsage();
            if (tmp.empty()) {
                res->code = 500;
                res->status = "error";
                res->message = "Failed to get current disks usage";
                res->error = "System Error";
            } else {
                for (auto &disk : tmp) {
                    res->value[disk.first] = disk.second;
                }
                res->status = "success";
                res->code = 200;
                res->message = "Success get current disks usage";
            }
            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIAvailableDrives) {
        info->summary = "Get available drives";
        info->addResponse<Object<ReturnAvailableDrivesDto>>(
            Status::CODE_200, "application/json", "Available drives");
    }
    ENDPOINT_ASYNC("GET", "/api/system/available_drives",
                   getUIAvailableDrives) {
        ENDPOINT_ASYNC_INIT(getUIAvailableDrives);
        Action act() override {
            auto res = ReturnAvailableDrivesDto::createShared();
            res->command = "getUIAvailableDrives";

            auto tmp = Atom::System::getAvailableDrives();

            if (tmp.empty()) {
                res->code = 500;
                res->status = "error";
                res->message = "Failed to get available drives";
                res->error = "System Error";
            } else {
                for (auto drive : tmp) {
                    res->value.push_back(drive);
                }

                res->status = "success";
                res->code = 200;
                res->message = "Success get available drives";
            }
            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIBatteryInfo) {
        info->summary = "Get battery info";
        info->addResponse<Object<ReturnBatteryInfoDto>>(
            Status::CODE_200, "application/json", "Battery info");
    }
    ENDPOINT_ASYNC("GET", "/api/system/battery", getUIBatteryInfo) {
        ENDPOINT_ASYNC_INIT(getUIBatteryInfo);
        Action act() override {
            auto res = ReturnBatteryInfoDto::createShared();
            res->command = "getUIBatteryInfo";

            const auto tmp = Atom::System::getBatteryInfo();

            res->isBatteryPresent = tmp.isBatteryPresent;
            res->isCharging = tmp.isCharging;
            res->batteryLifePercent = tmp.batteryLifePercent;
            res->batteryLifeTime = tmp.batteryLifeTime;
            res->batteryFullLifeTime = tmp.batteryFullLifeTime;
            res->energyNow = tmp.energyNow;
            res->energyFull = tmp.energyFull;
            res->energyDesign = tmp.energyDesign;
            res->voltageNow = tmp.voltageNow;
            res->currentNow = tmp.currentNow;

            res->message = "Success get battery info";
            res->code = 200;
            res->status = "success";

            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUINetworkInfo) {
        info->summary = "Get network info";
        info->addResponse<Object<ReturnNetworkInfoDto>>(
            Status::CODE_200, "application/json", "Network info");
    }
    ENDPOINT_ASYNC("GET", "/api/system/network", getUINetworkInfo) {
        ENDPOINT_ASYNC_INIT(getUINetworkInfo);
        Action act() override {
            auto res = ReturnNetworkInfoDto::createShared();
            res->command = "getUINetworkInfo";

            try {
                auto isHotspotConnected = Atom::System::isHotspotConnected();
                auto wifi = Atom::System::getCurrentWifi();
                auto wired = Atom::System::getCurrentWiredNetwork();

                res->hotspot = isHotspotConnected;
                res->wifi = wifi;
                res->wired = wired;

                res->message = "Success get network info";
                res->code = 200;
                res->status = "success";
            } catch (const std::exception &e) {
                res->message = e.what();
                res->code = 500;
                res->status = "error";
            }

            return _return(controller->createDtoResponse(
                res->code == 500 ? Status::CODE_500 : Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIProcesses) {
        info->summary = "Get all running processes";
        // info->addResponse<String>(Status::CODE_200, "application/json",
        // "Processes"); info->addResponse<String>(Status::CODE_404,
        // "text/plain");
        info->addResponse<String>(Status::CODE_200, "text/plain");
    }
    ENDPOINT_ASYNC("GET", "/api/system/process", getUIProcesses) {
        ENDPOINT_ASYNC_INIT(getUIProcesses);
        Action act() override {
            nlohmann::json res;
            for (const auto &process : Atom::System::GetProcessInfo()) {
                OATPP_LOGD("System", "Process Name: %s File Address: %s",
                           process.first.c_str(), process.second.c_str());
                res["value"][process.first] = process.second;
            }
            auto response =
                controller->createResponse(Status::CODE_200, res.dump());
            response->putHeader(Header::CONTENT_TYPE, "text/json");
            return _return(response);
        }
    };

    ENDPOINT_INFO(getUIShutdown) { info->summary = "Shutdown system"; }
    ENDPOINT_ASYNC("GET", "/api/system/shutdown", getUIShutdown) {
        ENDPOINT_ASYNC_INIT(getUIShutdown);
        Action act() override {
            Atom::System::Shutdown();
            return _return(controller->createResponse(
                Status::CODE_200, "Wtf, how can you do that?"));
        }
    };

    ENDPOINT_INFO(getUIReboot) { info->summary = "Reboot system"; }
    ENDPOINT_ASYNC("GET", "/api/system/reboot", getUIReboot) {
        ENDPOINT_ASYNC_INIT(getUIReboot);
        Action act() override {
            Atom::System::Reboot();
            return _return(controller->createResponse(
                Status::CODE_200, "Wtf, how can you do that?"));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_SYSTEM_CONTROLLER_HPP