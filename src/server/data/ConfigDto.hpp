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

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include "StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

class GetConfigDTO : public oatpp::DTO {
    DTO_INIT(GetConfigDTO, DTO)

    DTO_FIELD_INFO(path) {
        info->description = "The name of the config value to get, split by '/'";
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(type) {
        info->description = "The type of the config value, not necessarily";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(defaultValue) {
        info->description =
            "Whether to get default value of the config value if the value is "
            "empty!";
    }
    DTO_FIELD(String, defaultValue);
};

class SetConfigDTO : public oatpp::DTO {
    DTO_INIT(SetConfigDTO, DTO)

    DTO_FIELD_INFO(path) {
        info->description = "The name of the config value to set, split by '/'";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(value) {
        info->description = "The value of the config value";
        info->required = true;
    }
    DTO_FIELD(String, value);

    DTO_FIELD_INFO(type) {
        info->description = "The type of the config value";
        info->required = true;
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(check) {
        info->description = "Whether to check the config value";
    }
    DTO_FIELD(Boolean, check);
};

class DeleteConfigDTO : public oatpp::DTO {
    DTO_INIT(DeleteConfigDTO, DTO)

    DTO_FIELD_INFO(path) {
        info->description =
            "The name of the config value to delete, split by '/'";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(force) {
        info->description = "Whether to force delete the config value";
    }
    DTO_FIELD(Boolean, force);
};

class LoadConfigDTO : public oatpp::DTO {
    DTO_INIT(LoadConfigDTO, DTO)

    DTO_FIELD_INFO(path) {
        info->description = "The path of the config value to load";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(isAbsolute) {
        info->description = "Whether the path is absolute or not";
    }
    DTO_FIELD(Boolean, isAbsolute);

    DTO_FIELD_INFO(rootPath) {
        info->description = "The root path of the config value to load";
    }
    DTO_FIELD(String, rootPath);
};

class SaveConfigDTO : public oatpp::DTO {
    DTO_INIT(SaveConfigDTO, DTO)

    DTO_FIELD_INFO(path) {
        info->description = "The path of the config value to save";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(isAbsolute) {
        info->description = "Whether the path is absolute or not";
    }
    DTO_FIELD(Boolean, isAbsolute);

    DTO_FIELD_INFO(rootPath) {
        info->description = "The root path of the config value to save";
    }
    DTO_FIELD(String, rootPath);

    DTO_FIELD_INFO(overwrite) {
        info->description = "Whether to overwrite the config value";
    }
    DTO_FIELD(Boolean, overwrite);
};

class ReturnConfigDTO : public StatusDto {
    DTO_INIT(ReturnConfigDTO, DTO)

    DTO_FIELD_INFO(value) {
        info->description = "The value of the config value";
    }
    DTO_FIELD(String, value);

    DTO_FIELD_INFO(type) { info->description = "The type of the config value"; }
    DTO_FIELD(String, type);
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif
