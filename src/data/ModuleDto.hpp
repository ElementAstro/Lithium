/*
 * ModuleDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-12-1

Description: Data Transform Object for Module Controller

**************************************************/

#ifndef MODULEDTO_HPP
#define MODULEDTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class LoadPluginDto : public oatpp::DTO
{
    DTO_INIT(LoadPluginDto, DTO)

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

class UnloadPluginDto : public oatpp::DTO
{
    DTO_INIT(UnloadPluginDto, DTO)

    DTO_FIELD_INFO(plugin_name)
    {
        info->description = "Name of the device plugin to remove";
        info->required = true;
    }
    DTO_FIELD(String, plugin_name);
};

class GetModuleListDto : public oatpp::DTO
{
    DTO_INIT(GetModuleListDto, DTO)

    DTO_FIELD_INFO(plugin_path)
    {
        info->description = "Path of the module to get";
        info->required = true;
    }
    DTO_FIELD(String, plugin_path);
};

class ReturnModuleListDto : public StatusDto
{
    DTO_INIT(ReturnModuleListDto, DTO)

    DTO_FIELD_INFO(module_list)
    {
        info->description = "List of the modules";
        info->required = true;
    }
    DTO_FIELD(Vector<String>, module_list);
};

class RefreshModuleListDto : public StatusDto
{
    DTO_INIT(RefreshModuleListDto, DTO)
};

class GetEnableModuleDto : public oatpp::DTO
{
    DTO_INIT(GetEnableModuleDto, DTO)

    DTO_FIELD_INFO(plugin_name)
    {
        info->description = "Name of the module to enable";
        info->required = true;
    }
    DTO_FIELD(String, plugin_name);
};

class ReturnEnableModuleDto : public StatusDto
{
    DTO_INIT(ReturnEnableModuleDto, DTO)

    DTO_FIELD_INFO(enable_module)
    {
        info->description = "Enable status of the module";
        info->required = true;
    }
    DTO_FIELD(Boolean, enable_module);
};

class GetDisableModuleDto : public oatpp::DTO
{
    DTO_INIT(GetDisableModuleDto, DTO)

    DTO_FIELD_INFO(plugin_name)
    {
        info->description = "Name of the module to disable";
        info->required = true;
    }
    DTO_FIELD(String, plugin_name);
};

class ReturnDisableModuleDto : public StatusDto
{
    DTO_INIT(ReturnDisableModuleDto, DTO)

    DTO_FIELD_INFO(disable_module)
    {
        info->description = "Disable status of the module";
        info->required = true;
    }
    DTO_FIELD(Boolean, disable_module);
};

class GetModuleStatusDto : public oatpp::DTO
{
    DTO_INIT(GetModuleStatusDto, DTO)

    DTO_FIELD_INFO(module_name)
    {
        info->description = "Name of the module to get";
        info->required = true;
    }
    DTO_FIELD(String, module_name);
};

class ReturnModuleStatusDto : public StatusDto
{
    DTO_INIT(ReturnModuleStatusDto, DTO)

    DTO_FIELD_INFO(module_status)
    {
        info->description = "Status of the module";
        info->required = true;
    }
    DTO_FIELD(Boolean, module_status);
};

class GetModuleConfigDto : public oatpp::DTO
{
    DTO_INIT(GetModuleConfigDto, DTO)

    DTO_FIELD_INFO(module_name)
    {
        info->description = "Name of the module to get";
        info->required = true;
    }
    DTO_FIELD(String, module_name);
};

class ReturnModuleConfigDto : public StatusDto
{
    DTO_INIT(ReturnModuleConfigDto, DTO)

    DTO_FIELD_INFO(module_config)
    {
        info->description = "Config of the module";
        info->required = true;
    }
    DTO_FIELD(String, module_config);
};

class GetInstanceDto : public oatpp::DTO
{
    DTO_INIT(GetInstanceDto, DTO)

    DTO_FIELD_INFO(module_name)
    {
        info->description = "Name of the module to get";
        info->required = true;
    }
    DTO_FIELD(String, module_name);

    DTO_FIELD_INFO(instance_name)
    {
        info->description = "Name of the instance to get";
        info->required = true;
    }
    DTO_FIELD(String, instance_name);

    DTO_FIELD_INFO(instance_type)
    {
        info->description = "Type of the instance to get";
        info->required = true;
    }
    DTO_FIELD(String, instance_type);

    DTO_FIELD_INFO(get_func)
    {
        info->description = "Function to get the instance";
        info->required = true;
    }
    DTO_FIELD(String, get_func);
};

class ReturnInstanceDto : public StatusDto
{
    DTO_INIT(ReturnInstanceDto, DTO)

    DTO_FIELD_INFO(instance)
    {
        info->description = "Instance of the module";
        info->required = true;
    }
    DTO_FIELD(String, instance);
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif