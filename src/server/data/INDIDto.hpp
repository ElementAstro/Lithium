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
#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

// General DTO for handling force start/stop operations
class ForceOperationDto : public oatpp::DTO {
    DTO_INIT(ForceOperationDto, DTO)

    DTO_FIELD_INFO(force) {
        info->description =
            "Whether to force the operation, applicable for both start and "
            "stop";
    }
    DTO_FIELD(Boolean, force);
};

// General DTO for handling drivers or devices operations
class OperationDto : public oatpp::DTO {
    DTO_INIT(OperationDto, DTO)

    DTO_FIELD_INFO(type) {
        info->description =
            "Type of the entity (driver/device) to operate on, default is all";
        info->required = false;
    }
    DTO_FIELD(String, type);
};

class StartINDIDto : public ForceOperationDto {
    DTO_INIT(StartINDIDto, ForceOperationDto)

    DTO_FIELD_INFO(executable) {
        info->description =
            "The executable path of the INDI server, default is indiserver";
    }
    DTO_FIELD(String, executable);

    DTO_FIELD_INFO(port) {
        info->description = "The port of the INDI server, default is 7624";
    }
    DTO_FIELD(String, port);

    DTO_FIELD_INFO(log) {
        info->description = "Whether to log the INDI server, default is true";
    }
    DTO_FIELD(Boolean, log);

    DTO_FIELD_INFO(logLevel) {
        info->description = "The log level of the INDI server, default is INFO";
    }
    DTO_FIELD(String, logLevel);

    DTO_FIELD_INFO(logFile) {
        info->description =
            "The log file of the INDI server, default is /tmp/indi.log";
    }
    DTO_FIELD(String, logFile);
};

using StopINDIDto = ForceOperationDto;

class ReturnEntityListDto : public StatusDto {
    DTO_INIT(ReturnEntityListDto, StatusDto)

    DTO_FIELD_INFO(entities) {
        info->description = "Available entities (drivers or devices)";
    }
    DTO_FIELD(UnorderedFields<String>, entities);
};

// Common DTOs for starting and stopping drivers/devices
class StartEntitiesDto : public oatpp::DTO {
    DTO_INIT(StartEntitiesDto, DTO)

    DTO_FIELD_INFO(entities) {
        info->description = "Entities (drivers or devices) to start";
    }
    DTO_FIELD(UnorderedFields<String>, entities);
};

class StopEntityDto : public oatpp::DTO {
    DTO_INIT(StopEntityDto, DTO)

    DTO_FIELD_INFO(entity) {
        info->description = "Entity (driver or device) to stop";
    }
    DTO_FIELD(String, entity);
};

class ReturnServerINDIScanDto : public StatusDto {
    DTO_INIT(ReturnServerINDIScanDto, StatusDto)

    DTO_FIELD_INFO(path) {
        info->description = "Path of the INDI server";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(version) {
        info->description = "Version of the INDI server";
        info->required = true;
    }
    DTO_FIELD(String, version);

    DTO_FIELD_INFO(port) {
        info->description = "Port of the INDI server";
        info->required = true;
    }
    DTO_FIELD(Int32, port);
};

class MultiInstancesDto : public oatpp::DTO {
    DTO_INIT(MultiInstancesDto, DTO)

    DTO_FIELD_INFO(pid) {
        info->description = "Process ID of the INDI server";
        info->required = true;
    }
    DTO_FIELD(Int32, pid);

    DTO_FIELD_INFO(path) {
        info->description = "Path of the INDI server";
        info->required = true;
    }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(version) {
        info->description = "Version of the INDI server";
        info->required = true;
    }
    DTO_FIELD(String, version);

    DTO_FIELD_INFO(name) {
        info->description = "Name of the INDI server";
        info->required = true;
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(port) {
        info->description = "Port of the INDI server";
        info->required = true;
    }
    DTO_FIELD(Int32, port);

    DTO_FIELD_INFO(canKill) {
        info->description = "Whether the INDI server can be killed";
        info->required = true;
    }
    DTO_FIELD(Boolean, canKill);
};

class ReturnServerINDIScanMultiInstancesDto : public StatusDto {
    DTO_INIT(ReturnServerINDIScanMultiInstancesDto, StatusDto)

    DTO_FIELD_INFO(instance) {
        info->description = "A array of all instances";
        info->required = true;
    }
    DTO_FIELD(List<Object<MultiInstancesDto>>, instance);
};

class INDIExecutableDto : public oatpp::DTO {
    DTO_INIT(INDIExecutableDto, oatpp::DTO)

    DTO_FIELD_INFO(executable) {
        info->description = "The name of the executable file";
    }
    DTO_FIELD(String, executable);

    DTO_FIELD_INFO(version) {
        info->description = "The version of the INDI server";
    }
    DTO_FIELD(String, version);

    DTO_FIELD_INFO(path) { info->description = "The path of the INDI server"; }
    DTO_FIELD(String, path);

    DTO_FIELD_INFO(permissions) {
        info->description = "The permissions of the INDI server";
    }
    DTO_FIELD(List<String>, permissions);
};

class ReturnINDIExecutableDto : public StatusDto {
    DTO_INIT(ReturnINDIExecutableDto, StatusDto)

    DTO_FIELD_INFO(instances) {
        info->description = "The INDI server instances";
    }
    DTO_FIELD(List<Object<INDIExecutableDto>>, instances);
};

class RequestINDIStartDto : public RequestDto {
    DTO_INIT(RequestINDIStartDto, RequestDto)

    DTO_FIELD_INFO(executable) {
        info->description = "The executable path of the INDI server";
        info->required = true;
    }
    DTO_FIELD(String, executable);

    DTO_FIELD_INFO(port) {
        info->description = "The port of the INDI server";
        info->required = true;
    }
    DTO_FIELD(String, port);

    DTO_FIELD_INFO(logLevel) {
        info->description = "The log level of the INDI server";
        info->required = true;
    }
    DTO_FIELD(String, logLevel);

    DTO_FIELD_INFO(tmpPath) {
        info->description = "The temporary path of the INDI server log";
        info->required = true;
    }
    DTO_FIELD(String, tmpPath);

    DTO_FIELD_INFO(enableLog) {
        info->description = "Whether to enable the log of the INDI server";
        info->required = true;
    }
    DTO_FIELD(Boolean, enableLog);
};

class RequestINDIDriverListDto: public RequestDto {
    DTO_INIT(RequestINDIDriverListDto, RequestDto)

    DTO_FIELD_INFO(type) {
        info->description = "The type of the entity (driver/device)";
        info->required = true;
    }
    DTO_FIELD(Vector<String>, type);

    DTO_FIELD_INFO(path) {
        info->description = "The path of the INDI driver declaration files";
        info->required = true;
    }
    DTO_FIELD(String, path);
};

class ReturnINDIDriverListInvalidTypeDto: public StatusDto {
    DTO_INIT(ReturnINDIDriverListInvalidTypeDto, StatusDto)

    DTO_FIELD_INFO(invalidType) {
        info->description = "The invalid type of the entity (driver/device)";
        info->required = true;
    }
    DTO_FIELD(Vector<String>, invalidType);
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif  // INDIDTO_HPP
