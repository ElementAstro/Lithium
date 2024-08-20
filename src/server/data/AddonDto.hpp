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

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include "StatusDto.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

// 通用模块操作DTO
class BaseModuleDto : public oatpp::DTO {
    DTO_INIT(BaseModuleDto, DTO)

    DTO_FIELD_INFO(module_name) {
        info->description = "Name of the module";
        info->required = true;
    }
    DTO_FIELD(String, module_name);

    DTO_FIELD_INFO(need_check) {
        info->description = "Whether to check the operation";
        info->required = false;
    }
    DTO_FIELD(Boolean, need_check);
};

// 加载模块DTO
class LoadModuleDto : public BaseModuleDto {
    DTO_INIT(LoadModuleDto, BaseModuleDto)

    DTO_FIELD_INFO(module_path) {
        info->description = "Path of the module to load";
        info->required = true;
    }
    DTO_FIELD(String, module_path);
};

// 返回模块状态或配置的通用DTO
class ReturnStatusDto : public StatusDto {
    DTO_INIT(ReturnStatusDto, StatusDto)

    DTO_FIELD_INFO(status) {
        info->description = "Status or enable/disable state of the module";
        info->required = true;
    }
    DTO_FIELD(Boolean, status);

    DTO_FIELD_INFO(module_config) {
        info->description = "Config of the module (if applicable)";
        info->required = false;
    }
    DTO_FIELD(String, module_config);
};

// 返回模块列表DTO
class ReturnModuleListDto : public StatusDto {
    DTO_INIT(ReturnModuleListDto, StatusDto)

    DTO_FIELD_INFO(module_list) {
        info->description = "List of the modules";
        info->required = true;
    }
    DTO_FIELD(Vector<String>, module_list);
};

// 获取实例的DTO
class GetInstanceDto : public oatpp::DTO {
    DTO_INIT(GetInstanceDto, DTO)

    DTO_FIELD_INFO(module_name) {
        info->description = "Name of the module to get";
        info->required = true;
    }
    DTO_FIELD(String, module_name);

    DTO_FIELD_INFO(instance_name) {
        info->description = "Name of the instance to get";
        info->required = true;
    }
    DTO_FIELD(String, instance_name);

    DTO_FIELD_INFO(instance_type) {
        info->description = "Type of the instance to get";
        info->required = true;
    }
    DTO_FIELD(String, instance_type);

    DTO_FIELD_INFO(get_func) {
        info->description = "Function to get the instance";
        info->required = true;
    }
    DTO_FIELD(String, get_func);
};

// 返回实例的DTO
class ReturnInstanceDto : public StatusDto {
    DTO_INIT(ReturnInstanceDto, StatusDto)

    DTO_FIELD_INFO(instance) {
        info->description = "Instance of the module";
        info->required = true;
    }
    DTO_FIELD(String, instance);
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif  // MODULEDTO_HPP
