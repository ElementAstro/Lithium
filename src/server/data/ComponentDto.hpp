/*
 * ComponentDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-17

Description: Data Transform Object for Component Controller

**************************************************/

#ifndef LITHIUM_SERVER_DTO_COMPONENT_DTO_HPP
#define LITHIUM_SERVER_DTO_COMPONENT_DTO_HPP

#include "PackageDto.hpp"
#include "RequestDto.hpp"
#include "StatusDto.hpp"
#include "Types.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

// General DTO for component operations

class ComponentDto : public oatpp::DTO {
    DTO_INIT(ComponentDto, DTO)

    DTO_FIELD_INFO(name) {
        info->description = "Component name";
        info->required = true;
        info->pattern = "^[a-zA-Z0-9_]+$";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(path) {
        info->description = "Component path";
        info->required = true;
        info->pattern = "^[a-zA-Z0-9_]+$";
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(instance) {
        info->description = "Component instance";
        info->required = true;
        info->pattern = "^[a-zA-Z0-9_]+$";
    }
    DTO_FIELD(String, instance);

    DTO_FIELD_INFO(package) {
        info->description = "Component package.json or package.yaml";
        info->required = true;
    }
    DTO_FIELD(oatpp::ObjectWrapper<PackageJsonDto>, package);
};

class ComponentFunctionDto : public oatpp::DTO {
    DTO_INIT(ComponentFunctionDto, DTO)

    DTO_FIELD_INFO(name) {
        info->description = "Component function name";
        info->required = true;
        info->pattern = "^[a-zA-Z0-9_]+$";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(group) {
        info->description = "Component function group";
        info->required = true;
        info->pattern = "^[a-zA-Z0-9_]+$";
    }
    DTO_FIELD(String, group);

    DTO_FIELD_INFO(description) {
        info->description = "Component function description";
    }
    DTO_FIELD(String, description);

    DTO_FIELD_INFO(argsType) {
        info->description = "Component function arguments type";
    }
    DTO_FIELD(List<String>, argsType);

    DTO_FIELD_INFO(returnType) {
        info->description = "Component function return type";
    }
    DTO_FIELD(String, returnType);
};

class ComponentInstanceDto : public oatpp::DTO {
    DTO_INIT(ComponentInstanceDto, DTO)

    DTO_FIELD_INFO(name) {
        info->description = "Component name";
        info->required = true;
        info->pattern = "^[a-zA-Z0-9_]+$";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(instance) {
        info->description = "Component instance";
        info->required = true;
        info->pattern = "^[a-zA-Z0-9_]+$";
    }
    DTO_FIELD(String, instance);

    DTO_FIELD_INFO(description) { info->description = "Component description"; }
    DTO_FIELD(String, description);

    DTO_FIELD_INFO(functions) { info->description = "Component functions"; }
    DTO_FIELD(List<ComponentFunctionDto>, functions);
};

class RequestComponentLoadDto : public RequestDto {
    DTO_INIT(RequestComponentLoadDto, RequestDto)

    DTO_FIELD_INFO(components) {
        info->description = "List of components to load";
        info->required = true;
    }
    DTO_FIELD(List<ComponentDto>, components);
};

class RequestComponentUnloadDto : public RequestDto {
    DTO_INIT(RequestComponentUnloadDto, RequestDto)

    DTO_FIELD_INFO(components) {
        info->description = "List of components to unload";
        info->required = true;
    }
    DTO_FIELD(List<ComponentInstanceDto>, components);
};

class RequestComponentReloadDto : public RequestDto {
    DTO_INIT(RequestComponentReloadDto, RequestDto)

    DTO_FIELD_INFO(components) {
        info->description = "List of components to reload";
        info->required = true;
    }
    DTO_FIELD(List<ComponentInstanceDto>, components);
};

class RequestComponentInfoDto : public RequestDto {
    DTO_INIT(RequestComponentInfoDto, RequestDto)

    DTO_FIELD_INFO(component) {
        info->description = "Component to get info";
        info->required = true;
    }
    DTO_FIELD(String, component);
};

class RequestComponentRunFunctionDto : public RequestDto {
    DTO_INIT(RequestComponentRunFunctionDto, RequestDto)

    DTO_FIELD_INFO(component) {
        info->description = "Component to run function";
        info->required = true;
    }
    DTO_FIELD(String, component);

    DTO_FIELD_INFO(function) {
        info->description = "Function to run";
        info->required = true;
    }
    DTO_FIELD(String, function);

    DTO_FIELD_INFO(args) {
        info->description =
            "Function arguments. The order is important and "
            "should match the function signature";
    }
    DTO_FIELD(List<String>, args);

    DTO_FIELD_INFO(anyArgs) {
        info->description = "Function arguments in any type";
    }
    DTO_FIELD(List<Any>, anyArgs);

    DTO_FIELD_INFO(ignore) { info->description = "Ignore the return value"; }
    DTO_FIELD(Boolean, ignore);
};

class ReturnComponentLoadNotFoundDto : public StatusDto {
    DTO_INIT(ReturnComponentLoadNotFoundDto, StatusDto)

    DTO_FIELD_INFO(component) {
        info->description = "The component that was not found";
    }
    DTO_FIELD(String, component);
};

class ReturnComponentFailToLoadDto : public StatusDto {
    DTO_INIT(ReturnComponentFailToLoadDto, StatusDto)

    DTO_FIELD_INFO(component) {
        info->description = "The component that failed to load";
    }
    DTO_FIELD(String, component);

    DTO_FIELD_INFO(stacktrace) {
        info->description = "The stacktrace of the error";
    }
    DTO_FIELD(String, stacktrace);
};

class ReturnComponentUnloadNotFoundDto : public StatusDto {
    DTO_INIT(ReturnComponentUnloadNotFoundDto, StatusDto)

    DTO_FIELD_INFO(component) {
        info->description = "The component that was not found";
    }
    DTO_FIELD(String, component);
};

class ReturnComponentFailToUnloadDto : public StatusDto {
    DTO_INIT(ReturnComponentFailToUnloadDto, StatusDto)

    DTO_FIELD_INFO(component) {
        info->description = "The component that failed to unload";
    }
    DTO_FIELD(String, component);

    DTO_FIELD_INFO(stacktrace) {
        info->description = "The stacktrace of the error";
    }
    DTO_FIELD(String, stacktrace);

    DTO_FIELD_INFO(related) {
        info->description = "The related component that failed to unload";
    }
    DTO_FIELD(List<String>, related);

    DTO_FIELD_INFO(relatedStacktrace) {
        info->description = "The stacktrace of the related component error";
    }
    DTO_FIELD(List<String>, relatedStacktrace);
};

class ReturnComponentListDto : public StatusDto {
    DTO_INIT(ReturnComponentListDto, StatusDto)

    DTO_FIELD_INFO(components) { info->description = "List of components"; }
    DTO_FIELD(List<ComponentInstanceDto>, components);
};

class ReturnComponentInfoDto : public StatusDto {
    DTO_INIT(ReturnComponentInfoDto, StatusDto)

    DTO_FIELD_INFO(component_info) {
        info->description = "Component infomation, just like package.json";
    }
    DTO_FIELD(List<PackageJsonDto>, component_info);
};

class ReturnComponentFunctionNotFoundDto : public StatusDto {
    DTO_INIT(ReturnComponentFunctionNotFoundDto, StatusDto)

    DTO_FIELD_INFO(component) {
        info->description = "The component that was not found";
    }
    DTO_FIELD(String, component);

    DTO_FIELD_INFO(function) {
        info->description = "The function that was not found";
    }
    DTO_FIELD(String, function);
};

class ReturnComponentFunctionFailToRunDto : public StatusDto {
    DTO_INIT(ReturnComponentFunctionFailToRunDto, StatusDto)

    DTO_FIELD_INFO(component) {
        info->description = "The component that failed to run function";
    }
    DTO_FIELD(String, component);

    DTO_FIELD_INFO(function) {
        info->description = "The function that failed to run";
    }
    DTO_FIELD(String, function);

    DTO_FIELD_INFO(stacktrace) {
        info->description = "The stacktrace of the error";
    }
    DTO_FIELD(String, stacktrace);
};
#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif  // LITHIUM_SERVER_DTO_COMPONENT_DTO_HPP
