/*
 * ModuleDto.hpp
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

Date: 2023-12-1

Description: Data Transform Object for Module Controller

**************************************************/

#ifndef MODULEDTO_HPP
#define MODULEDTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class LoadPluginDTO : public oatpp::DTO
{
    DTO_INIT(LoadPluginDTO, DTO)

    DTO_FIELD_INFO(plugin_path)
    {
        info->description = "Path of the device plugin to add";
        info->required = true;
    }
    DTO_FIELD(String, plugin_path);

    DTO_FIELD_INFO(plugin_name)
    {
        info->description = "Name of the device plugin to add";
        info->required = true;
    }
    DTO_FIELD(String, plugin_name);

    DTO_FIELD_INFO(plugin_type)
    {
        info->description = "Type of the device plugin to add";
        info->required = true;
    }
    DTO_FIELD(String, plugin_type);

    DTO_FIELD_INFO(need_check)
    {
        info->description = "Whether to check the plugin";
        info->required = false;
    }
    DTO_FIELD(Boolean, need_check);
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif