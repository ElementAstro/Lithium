/*
 * INDIDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-17

Description: Data Transform Object for INDI Controller

**************************************************/

#ifndef INDIDTO_HPP
#define INDIDTO_HPP

#include "data/RequestDto.hpp"
#include "data/StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

// General DTO for handling force start/stop operations
class ReturnScriptEnvDto: public StatusDto {
    DTO_INIT(ReturnScriptEnvDto, StatusDto)

    DTO_FIELD_INFO(env) {
        info->description = "Environment variables";
        info->required = true;
    }
    DTO_FIELD(UnorderedFields<String>, env);
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif  // INDIDTO_HPP
