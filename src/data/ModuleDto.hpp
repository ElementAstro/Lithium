/*
 * ModuleDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

class BaseModuleDto : public oatpp::DTO
{
    DTO_INIT(LoadPluginDto, DTO)

    DTO_FIELD_INFO(module_name)
    {
        info->description = "Name of the module to modify";
        info->required = true;
    }
    DTO_FIELD(String, module_name);

    DTO_FIELD_INFO(need_check)
    {
        info->description = "Whether to check the operation";
        info->required = false;
    }
    DTO_FIELD(Boolean, need_check);
};

class LoadModuleDto : public BaseModuleDto
{
    DTO_INIT(LoadModuleDto, BaseModuleDto)

    DTO_FIELD_INFO(module_path)
    {
        info->description = "Path of the module to load";
        info->required = true;
    }
    DTO_FIELD(String, module_path);
};

class UnloadPluginDto : public BaseModuleDto
{
    DTO_INIT(UnloadPluginDto, BaseModuleDto)
};

class GetModuleListDto : public BaseModuleDto
{
    DTO_INIT(GetModuleListDto, BaseModuleDto)
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

class GetEnableModuleDto : public BaseModuleDto
{
    DTO_INIT(GetEnableModuleDto, BaseModuleDto)
};

class ReturnEnableModuleDto : public StatusDto
{
    DTO_INIT(ReturnEnableModuleDto, DTO)

    DTO_FIELD_INFO(status)
    {
        info->description = "Enable status of the module";
        info->required = true;
    }
    DTO_FIELD(Boolean, status);
};

class GetDisableModuleDto : public BaseModuleDto
{
    DTO_INIT(GetDisableModuleDto, BaseModuleDto)
};

class ReturnDisableModuleDto : public StatusDto
{
    DTO_INIT(ReturnDisableModuleDto, DTO)

    DTO_FIELD_INFO(status)
    {
        info->description = "Disable status of the module";
        info->required = true;
    }
    DTO_FIELD(Boolean, status);
};

class GetModuleStatusDto : public BaseModuleDto
{
    DTO_INIT(GetModuleStatusDto, BaseModuleDto)
};

class ReturnModuleStatusDto : public StatusDto
{
    DTO_INIT(ReturnModuleStatusDto, DTO)

    DTO_FIELD_INFO(status)
    {
        info->description = "Status of the module";
        info->required = true;
    }
    DTO_FIELD(Boolean, status);
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