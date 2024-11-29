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

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include "atom/utils/print.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class StatusDto : public oatpp::DTO {
    DTO_INIT(StatusDto, DTO)

    DTO_FIELD_INFO(status) { info->description = "Short status text"; }
    DTO_FIELD(String, status);

    DTO_FIELD_INFO(code) { info->description = "Status code"; }
    DTO_FIELD(Int32, code);

    DTO_FIELD_INFO(message) { info->description = "Verbose message"; }
    DTO_FIELD(String, message);

    DTO_FIELD_INFO(error) { info->description = "Verbose error message"; }
    DTO_FIELD(String, error);

    DTO_FIELD_INFO(warning) { info->description = "Verbose warning message"; }
    DTO_FIELD(String, warning);

    DTO_FIELD_INFO(command) {
        info->description = "Command";
        info->required = true;
    }
    DTO_FIELD(String, command);
};

class UnknownErrorDto : public StatusDto {
    DTO_INIT(UnknownErrorDto, StatusDto)
};

class InternalServerErrorDto : public StatusDto {
    DTO_INIT(InternalServerErrorDto, StatusDto)
};

class InvalidParametersDto : public StatusDto {
    DTO_INIT(InvalidParametersDto, StatusDto)
};

class PathNotFoundDto : public StatusDto {
    DTO_INIT(PathNotFoundDto, StatusDto)
};

class ForbiddenDto : public StatusDto {
    DTO_INIT(ForbiddenDto, StatusDto)
};

#include OATPP_CODEGEN_END(DTO)

#endif  // STATUSDTO_HPP
