/*
 * SystemDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-25

Description: System Data Transform Object

**************************************************/

#ifndef SYSTEMDTO_HPP
#define SYSTEMDTO_HPP

#include "StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class BaseReturnSystemDto : public StatusDto {
    DTO_INIT(BaseReturnSystemDto, StatusDto)

    DTO_FIELD_INFO(value) {
        info->description = "The value of the system info";
    }
    DTO_FIELD(Float32, value);
};

class ReturnCpuInfoDto : public StatusDto {
    DTO_INIT(ReturnCpuInfoDto, StatusDto)

    DTO_FIELD_INFO(model) { info->description = "The value of the CPU model"; }
    DTO_FIELD(String, model);

    DTO_FIELD_INFO(frequency) {
        info->description = "The value of the CPU frequency";
    }
    DTO_FIELD(Float32, frequency);

    DTO_FIELD_INFO(identifier) {
        info->description = "The value of the CPU identifier";
    }
    DTO_FIELD(String, identifier);

    DTO_FIELD_INFO(packages) {
        info->description = "The value of the number of physical CPU packages";
    }
    DTO_FIELD(Int32, packages);

    DTO_FIELD_INFO(cpus) {
        info->description = "The value of the number of physical CPUs";
    }
    DTO_FIELD(Int32, cpus);
};

class ReturnMemoryInfoDto : public StatusDto {
    DTO_INIT(ReturnMemoryInfoDto, StatusDto)

    DTO_FIELD_INFO(memory_slot) {
        info->description = "The value of the memory slot";
    }
    DTO_FIELD(UnorderedFields<String>, memory_slot);
    DTO_FIELD_INFO(virtual_memory_max) {
        info->description = "The value of the virtual memory max";
    }
    DTO_FIELD(Float64, virtual_memory_max);
    DTO_FIELD_INFO(virtual_memory_used) {
        info->description = "The value of the virtual memory used";
    }
    DTO_FIELD(Float64, virtual_memory_used);
    DTO_FIELD_INFO(swap_memory_total) {
        info->description = "The value of the swap memory total";
    }
    DTO_FIELD(Float64, swap_memory_total);
    DTO_FIELD_INFO(swap_memory_used) {
        info->description = "The value of the swap memory used";
    }
    DTO_FIELD(Float64, swap_memory_used);

    DTO_FIELD_INFO(total_memory) {
        info->description = "The value of the total memory";
    }
    DTO_FIELD(Float64, total_memory);

    DTO_FIELD_INFO(available_memory) {
        info->description = "The value of the available memory";
    }
    DTO_FIELD(Float64, available_memory);
};

class ReturnBatteryInfoDto : public StatusDto {
    DTO_INIT(ReturnBatteryInfoDto, StatusDto)

    DTO_FIELD_INFO(isBatteryPresent) {
        info->description = "The value of the battery present";
    }
    DTO_FIELD(Boolean, isBatteryPresent);

    DTO_FIELD_INFO(isCharging) {
        info->description = "The value of the battery charging";
    }
    DTO_FIELD(Boolean, isCharging);

    DTO_FIELD_INFO(batteryLifePercent) {
        info->description = "The value of the battery life percent";
    }
    DTO_FIELD(Float32, batteryLifePercent);

    DTO_FIELD_INFO(batteryLifeTime) {
        info->description = "The value of the battery life time";
    }
    DTO_FIELD(Float32, batteryLifeTime);

    DTO_FIELD_INFO(batteryFullLifeTime) {
        info->description = "The value of the battery full life time";
    }
    DTO_FIELD(Float32, batteryFullLifeTime);

    DTO_FIELD_INFO(energyNow) {
        info->description = "The value of the energy now";
    }
    DTO_FIELD(Float32, energyNow);

    DTO_FIELD_INFO(energyFull) {
        info->description = "The value of the energy full";
    }
    DTO_FIELD(Float32, energyFull);

    DTO_FIELD_INFO(energyDesign) {
        info->description = "The value of the energy design";
    }
    DTO_FIELD(Float32, energyDesign);

    DTO_FIELD_INFO(voltageNow) {
        info->description = "The value of the voltage now";
    }
    DTO_FIELD(Float32, voltageNow);

    DTO_FIELD_INFO(currentNow) {
        info->description = "The value of the current now";
    }
    DTO_FIELD(Float32, currentNow);
};

class ReturnDiskUsageDto : public StatusDto {
    DTO_INIT(ReturnDiskUsageDto, StatusDto)

    DTO_FIELD_INFO(value) { info->description = "The value of the disk usage"; }
    DTO_FIELD(UnorderedFields<Float32>, value);
};

class ReturnAvailableDrivesDto : public StatusDto {
    DTO_INIT(ReturnAvailableDrivesDto, StatusDto)

    DTO_FIELD_INFO(value) { info->description = "The value of the available drives"; }
    DTO_FIELD(Vector<String>, value);
};

class ReturnNetworkInfoDto : public StatusDto {
    DTO_INIT(ReturnNetworkInfoDto, StatusDto)

    DTO_FIELD_INFO(wifi) { info->description = "The value of the wifi"; }
    DTO_FIELD(String, wifi);
    DTO_FIELD_INFO(wired) { info->description = "The value of the wired"; }
    DTO_FIELD(String, wired);
    DTO_FIELD_INFO(hotspot) { info->description = "The value of the hotspot"; }
    DTO_FIELD(Boolean, hotspot);
};

#include OATPP_CODEGEN_END(DTO)

#endif  // SYSTEMDTO_HPP
