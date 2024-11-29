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

#include "data/INDIDto.hpp"
#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include "RequestDto.hpp"
#include "StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

ENUM(PathType, v_int32, VALUE(File, 0, "File"), VALUE(Folder, 1, "Folder"),
     VALUE(Symlink, 2, "Symlink"), VALUE(Other, 3, "Other"))

class GetConfigDTO : public RequestDto {
    DTO_INIT(GetConfigDTO, RequestDto)

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

class SetConfigDTO : public RequestDto {
    DTO_INIT(SetConfigDTO, RequestDto)

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

class HasConfigDTO : public RequestDto {
    DTO_INIT(HasConfigDTO, RequestDto)

    DTO_FIELD_INFO(path) {
        info->description =
            "The name of the config value to check, split by '/' or '.'";
        info->required = true;
    }
    DTO_FIELD(String, path);
};

class DeleteConfigDTO : public RequestDto {
    DTO_INIT(DeleteConfigDTO, RequestDto)

    DTO_FIELD_INFO(path) {
        info->description =
            "The name of the config value to delete, split by '/'";
        info->required = true;
    }
    DTO_FIELD(String, path);
};

class TidyConfigDto : public RequestDto {
    DTO_INIT(TidyConfigDto, RequestDto)
};

class LoadConfigDTO : public RequestDto {
    DTO_INIT(LoadConfigDTO, RequestDto)

    DTO_FIELD_INFO(path) {
        info->description = "The path of the config value to load";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(type) {
        info->description = "The type of the config value";
        info->required = true;
    }
    DTO_FIELD(Enum<PathType>, type);

    DTO_FIELD_INFO(refresh) {
        info->description = "Whether to refresh the config value";
    }
    DTO_FIELD(Boolean, refresh);

    DTO_FIELD_INFO(isAbsolute) {
        info->description = "Whether the path is absolute or not";
    }
    DTO_FIELD(Boolean, isAbsolute);

    DTO_FIELD_INFO(rootPath) {
        info->description = "The root path of the config value to load";
    }
    DTO_FIELD(String, rootPath);
};

class ReloadConfigDto : public RequestDto {
    DTO_INIT(ReloadConfigDto, RequestDto)

    DTO_FIELD_INFO(name) {
        info->description = "The name of the config value to reload";
        info->required = true;
    }
    DTO_FIELD(String, name);
};

class SaveConfigDTO : public RequestDto {
    DTO_INIT(SaveConfigDTO, RequestDto)

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

class ReturnGetConfigDTO : public ReturnConfigDTO {
    DTO_INIT(ReturnGetConfigDTO, ReturnConfigDTO)

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
};

class ReturnListConfigDTO : public StatusDto {
    DTO_INIT(ReturnListConfigDTO, StatusDto)

    DTO_FIELD_INFO(config) {
        info->description = "The object of the config value";
    }
    DTO_FIELD(String, config);
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif
