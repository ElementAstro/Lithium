/*
 * INDIDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-17

Description: Data Transform Object for INDI Controller

**************************************************/

#ifndef INDIDTO_HPP
#define INDIDTO_HPP

#include "data/RequestDto.hpp"
#include "data/StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

class InterperterRequirementDto : public oatpp::DTO {
    DTO_INIT(InterperterRequirementDto, oatpp::DTO)

    DTO_FIELD_INFO(interpreter) {
        info->description = "Interpreter name";
        info->required = true;
    }
    DTO_FIELD(String, interpreter);

    DTO_FIELD_INFO(path) { info->description = "Path to the interpreter"; }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(version) {
        info->description = "minimum version of the interpreter";
    }
    DTO_FIELD(String, version);
};

class ArgumentRequirementDto : public oatpp::DTO {
    DTO_INIT(ArgumentRequirementDto, oatpp::DTO)

    DTO_FIELD_INFO(name) {
        info->description = "Name of the argument";
        info->required = true;
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(type) {
        info->description = "Type of the argument";
        info->pattern = "^(string|int|float|bool)$";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(description) {
        info->description = "Description of the argument";
    }
    DTO_FIELD(String, description);

    DTO_FIELD_INFO(defaultValue) {
        info->description = "Default value of the argument";
    }
    DTO_FIELD(String, defaultValue);

    DTO_FIELD_INFO(required) {
        info->description = "Whether the argument is required";
    }
    DTO_FIELD(Boolean, required);
};

class ScriptDto : public oatpp::DTO {
    DTO_INIT(ScriptDto, oatpp::DTO)

    DTO_FIELD_INFO(name) { info->description = "Name of the script"; }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(type) {
        info->description = "Type of the script";
        info->pattern = "^(shell|powershell|python)$";
    }
    DTO_FIELD(String, type);

    DTO_FIELD_INFO(description) {
        info->description = "Description of the script";
    }
    DTO_FIELD(String, description);

    DTO_FIELD_INFO(author) { info->description = "Author of the script"; }
    DTO_FIELD(String, author);

    DTO_FIELD_INFO(version) { info->description = "Version of the script"; }
    DTO_FIELD(String, version);

    DTO_FIELD_INFO(license) { info->description = "License of the script"; }
    DTO_FIELD(String, license);

    DTO_FIELD_INFO(interpreter) {
        info->description = "Interpreter of the script";
    }
    DTO_FIELD(oatpp::Object<InterperterRequirementDto>, interpreter);

    DTO_FIELD_INFO(platform) {
        info->description = "Platform of the script";
        info->pattern = "^(windows|linux|macos)$";
    }
    DTO_FIELD(String, platform);

    DTO_FIELD_INFO(permission) {
        info->description = "Permission of the script";
        info->pattern = "^(user|admin)$";
    }
    DTO_FIELD(String, permission);

    DTO_FIELD_INFO(async) {
        info->description = "Asynchronous execution of the script";
    }
    DTO_FIELD(Boolean, async);

    DTO_FIELD_INFO(line) { info->description = "Line number of the script"; }
    DTO_FIELD(Int32, line);

    DTO_FIELD_INFO(args) { info->description = "Arguments of the script"; }
    DTO_FIELD(List<ArgumentRequirementDto>, args);

    DTO_FIELD_INFO(content) { info->description = "Content of the script"; }
    DTO_FIELD(String, content);
};

// General DTO for handling force start/stop operations
class ReturnScriptEnvDto : public StatusDto {
    DTO_INIT(ReturnScriptEnvDto, StatusDto)

    DTO_FIELD_INFO(env) {
        info->description = "Environment variables";
        info->required = true;
    }
    DTO_FIELD(UnorderedFields<String>, env);
};

class RequestScriptListDto : public RequestDto {
    DTO_INIT(RequestScriptListDto, RequestDto)

    DTO_FIELD_INFO(path) { info->description = "Path to the script directory"; }
    DTO_FIELD(String, path);
};

class ReturnScriptListDto : public StatusDto {
    DTO_INIT(ReturnScriptListDto, StatusDto)

    DTO_FIELD_INFO(scripts) {
        info->description = "List of scripts";
        info->required = true;
    }
    DTO_FIELD(List<ScriptDto>, scripts);
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif  // INDIDTO_HPP
