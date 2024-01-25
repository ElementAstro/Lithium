/*
 * StatusDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-25

Description: Status Data Transform Object

**************************************************/

#ifndef STATUSDTO_HPP
#define STATUSDTO_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class StatusDto : public oatpp::DTO
{
    DTO_INIT(StatusDto, DTO)

    DTO_FIELD_INFO(status)
    {
        info->description = "Short status text";
    }
    DTO_FIELD(String, status);

    DTO_FIELD_INFO(code)
    {
        info->description = "Status code";
    }
    DTO_FIELD(Int32, code);

    DTO_FIELD_INFO(message)
    {
        info->description = "Verbose message";
    }
    DTO_FIELD(String, message);

    DTO_FIELD_INFO(error)
    {
        info->description = "Verbose error message";
    }
    DTO_FIELD(String, error);

    DTO_FIELD_INFO(warning)
    {
        info->description = "Verbose warning messsage";
    }
    DTO_FIELD(String, warning);

    DTO_FIELD_INFO(command)
    {
        info->description = "Command";
        info->required = true;
    }
    DTO_FIELD(String, command);
};

#include OATPP_CODEGEN_END(DTO)

#endif // STATUSDTO_HPP
