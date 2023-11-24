/*
 * DeviceDto.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-11-17

Description: Data Transform Object for Device Controller

**************************************************/

#ifndef SCRIPTDTO_HPP
#define SCRIPTDTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class RunCScriptFileDTO : public oatpp::DTO
{
    DTO_INIT(RunCScriptFileDTO, DTO)

    DTO_FIELD_INFO(script_path)
    {
        info->description = "Path of the script to run";
        info->required = true;
    }
    DTO_FIELD(String, script_path);

    DTO_FIELD_INFO(script_name)
    {
        info->description = "Name of the script to run";
        info->required = true;
    }
    DTO_FIELD(String, script_name);
};

class RunCScriptDTO : public oatpp::DTO
{
    DTO_INIT(RunCScriptDTO, DTO)

    DTO_FIELD_INFO(script)
    {
        info->description = "A single line script";
        info->required = true;
    }
    DTO_FIELD(String, script);

    DTO_FIELD_INFO(need_async)
    {
        info->description = "Whether to run in async mode (default: true)";
        info->required = false;
    }
    DTO_FIELD(Boolean, need_async);

    DTO_FIELD_INFO(need_result)
    {
        info->description = "Whether to return the result of the script (default: false)";
        info->required = false;
    }
    DTO_FIELD(Boolean, need_result);
};

class CheckScriptFileDTO: public oatpp::DTO
{
    DTO_INIT(CheckScriptFileDTO, DTO)

    DTO_FIELD_INFO(script_path)
    {
        info->description = "Path of the script to check";
        info->required = true;
    }
    DTO_FIELD(String, script_path);
};

class GetScriptFileDTO : public oatpp::DTO
{
    DTO_INIT(GetScriptFileDTO, DTO)

    DTO_FIELD_INFO(script_path)
    {
        info->description = "Path of the script to get";
        info->required = true;
    }
    DTO_FIELD(String, script_path);
};

class LoadScriptFileDTO : public oatpp::DTO
{
    DTO_INIT(LoadScriptFileDTO, DTO)

    DTO_FIELD_INFO(script_path)
    {
        info->description = "Path of the script to load";
        info->required = true;
    }
    DTO_FIELD(String, script_path);

    DTO_FIELD_INFO(script_name)
    {
        info->description = "Name of the script to load";
        info->required = true;
    }
    DTO_FIELD(String, script_name);
};

class UnloadScriptFileDTO : public oatpp::DTO
{
    DTO_INIT(UnloadScriptFileDTO, DTO)

    DTO_FIELD_INFO(script_name)
    {
        info->description = "Name of the script to unload";
        info->required = true;
    }
    DTO_FIELD(String, script_name);
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif