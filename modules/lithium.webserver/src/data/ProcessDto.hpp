/*
 * ProcessDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-25

Description: Data Transform Object for Process Controller

**************************************************/

#ifndef PROCESSDTO_HPP
#define PROCESSDTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

class CreateProcessDTO : public oatpp::DTO {
    DTO_INIT(CreateProcessDTO, DTO)
    DTO_FIELD_INFO(process_name) { info->description = "process name"; }
    DTO_FIELD(String, process_name);

    DTO_FIELD_INFO(process_id) {
        info->description = "process id (used for more operation)";
    }
    DTO_FIELD(String, process_id);
};

class TerminateProcessDTO : public oatpp::DTO {
    DTO_INIT(TerminateProcessDTO, DTO)

    DTO_FIELD_INFO(process_id) {
        info->description = "process id to terminate";
    }
    DTO_FIELD(String, process_id);
};

class RunScriptDTO : public oatpp::DTO {
    DTO_INIT(RunScriptDTO, DTO)
    DTO_FIELD_INFO(script_name) { info->description = "script name"; }
    DTO_FIELD(String, script_name);

    DTO_FIELD_INFO(script_id) {
        info->description = "script id (used for more operation)";
    }
    DTO_FIELD(String, script_id);
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif
