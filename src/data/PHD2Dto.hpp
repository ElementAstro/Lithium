/*
 * PHD2Dto.hpp
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

Date: 2023-7-25

Description: Data Transform Object for Process Controller

**************************************************/

#ifndef PHD2DTO_HPP
#define PHD2DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class StartPHD2DTO : public oatpp::DTO
{
    DTO_INIT(StartPHD2DTO, DTO)

    DTO_FIELD_INFO(phd2_params)
    {
        info->description = "parameters of PHD2 (JSON)";
        info->required = false;
    }
    DTO_FIELD(String, phd2_params);
};

class ModifyPHD2ParamDTO : public oatpp::DTO
{
    DTO_INIT(ModifyPHD2ParamDTO, DTO)

    DTO_FIELD_INFO(param_name)
    {
        info->description = "name of parameters";
        info->required = true;
    }
    DTO_FIELD(String, param_name);

    DTO_FIELD_INFO(param_value)
    {
        info->description = "value of parameters";
        info->required = true;
    }
    DTO_FIELD(String, param_value);
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif