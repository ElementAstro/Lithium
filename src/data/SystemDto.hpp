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

class BaseReturnSystemDto : public StatusDto
{
    DTO_INIT(BaseReturnSystemDto, StatusDto)

    DTO_FIELD_INFO(value)
    {
        info->description = "The value of the system info";
    }
    DTO_FIELD(Float32, value);
};

class ReturnDiskUsageDto : public StatusDto
{
    DTO_INIT(ReturnDiskUsageDto, StatusDto)

    DTO_FIELD_INFO(value)
    {
        info->description = "The value of the disk usage";
    }
    DTO_FIELD(UnorderedFields<Float32>, value);
};

#include OATPP_CODEGEN_END(DTO)

#endif // SYSTEMDTO_HPP
