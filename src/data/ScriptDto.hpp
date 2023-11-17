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

class RunScriptDTO : public oatpp::DTO
{
    DTO_INIT(RunScriptDTO, DTO)

    DTO_FIELD_INFO(library_path)
    {
        info->description = "Path of the device library to add";
        info->required = true;
    }
    DTO_FIELD(String, library_path);

    DTO_FIELD_INFO(library_name)
    {
        info->description = "Name of the device library to add";
        info->required = true;
    }
    DTO_FIELD(String, library_name);
};


#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif