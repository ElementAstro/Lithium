/*
 * ConfigDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-17

Description: Data Transform Object for Config Controller

**************************************************/

#ifndef CONFIGDTO_HPP
#define CONFIGDTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include "StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class GetConfigDTO : public oatpp::DTO
{
    DTO_INIT(GetConfigDTO, DTO)

    DTO_FIELD_INFO(path)
    {
        info->description = "The name of the config value to get, split by '/'";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(type)
    {
        info->description = "The type of the config value, not necessarily";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(defaultValue)
    {
        info->description = "Whether to get default value of the config value if the value is empty!";
    }
    DTO_FIELD(Boolean, defaultValue);
};

class SetConfigDTO : public oatpp::DTO
{
    DTO_INIT(SetConfigDTO, DTO)

    DTO_FIELD_INFO(path)
    {
        info->description = "The name of the config value to set, split by '/'";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(value)
    {
        info->description = "The value of the config value";
        info->required = true;
    }
    DTO_FIELD(String, value);

    DTO_FIELD_INFO(type)
    {
        info->description = "The type of the config value";
        info->required = true;
    }
    DTO_FIELD(String, type);
};

class DeleteConfigDTO : public oatpp::DTO
{
    DTO_INIT(DeleteConfigDTO, DTO)

    DTO_FIELD_INFO(path)
    {
        info->description = "The name of the config value to delete, split by '/'";
        info->required = true;
    }
    DTO_FIELD(String, path);
};

class LoadConfigDTO : public oatpp::DTO
{
    DTO_INIT(LoadConfigDTO, DTO)

    DTO_FIELD_INFO(path)
    {
        info->description = "The path of the config value to load";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(isAbsolute)
    {
        info->description = "Whether the path is absolute or not";
        info->required = true;
    }
    DTO_FIELD(Boolean, isAbsolute);
};

class SaveConfigDTO : public oatpp::DTO
{
    DTO_INIT(SaveConfigDTO, DTO)

    DTO_FIELD_INFO(path)
    {
        info->description = "The path of the config value to save";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(isAbsolute)
    {
        info->description = "Whether the path is absolute or not";
        info->required = true;
    }
    DTO_FIELD(Boolean, isAbsolute);
};


class ReturnConfigDTO : public StatusDto
{
    DTO_INIT(ReturnConfigDTO, DTO)

    DTO_FIELD_INFO(value)
    {
        info->description = "The value of the config value";
    }
    DTO_FIELD(String, value);

    DTO_FIELD_INFO(type)
    {
        info->description = "The type of the config value";
    }
    DTO_FIELD(String, type);
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif