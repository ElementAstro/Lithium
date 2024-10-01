/*
 * RequestDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-25

Description: Request Data Transform Object

**************************************************/

#ifndef REQUESTDTO_HPP
#define REQUESTDTO_HPP

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

class RequestDto : public oatpp::DTO {
    DTO_INIT(RequestDto, DTO)

    DTO_FIELD_INFO(force) {
        info->description =
            "Whether to force the operation, applicable for both start and "
            "stop";
        info->required = true;
    }
    DTO_FIELD(Boolean, force);

    DTO_FIELD_INFO(timeout) {
        info->description = "Timeout for the operation, default is 10 seconds";
        info->required = false;
    }
    DTO_FIELD(Int32, timeout);

    DTO_FIELD_INFO(retry) {
        info->description = "Retry times for the operation, default is 3";
        info->required = false;
    }
    DTO_FIELD(Int32, retry);
};

#include OATPP_CODEGEN_END(DTO)

#endif  // REQUESTDTO_HPP
