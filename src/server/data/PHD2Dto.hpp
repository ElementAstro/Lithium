/*
 * PHD2Dto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-17

Description: Data Transform Object for PHD2 Controller

**************************************************/

#ifndef PHD2DTO_HPP
#define PHD2DTO_HPP

#include "data/RequestDto.hpp"
#include "data/StatusDto.hpp"

#include "oatpp/Types.hpp"
#include "oatpp/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)  ///< Begin DTO codegen section

class RequestPHD2ScanDto : public RequestDto {
    DTO_INIT(RequestPHD2ScanDto, RequestDto)
};

class PHD2ExecutableDto : public oatpp::DTO {
    DTO_INIT(PHD2ExecutableDto, DTO)

    DTO_FIELD_INFO(executable) {
        info->description = "The executable path of the PHD2 server";
        info->required = true;
    }
    DTO_FIELD(String, executable);

    DTO_FIELD_INFO(version) {
        info->description = "The version of the PHD2 server";
    }
    DTO_FIELD(String, version);

    DTO_FIELD_INFO(permission) {
        info->description = "The permission of the PHD2 server";
    }
    DTO_FIELD(Vector<String>, permission);
};

class ReturnPHD2ScanDto : public StatusDto {
    DTO_INIT(ReturnPHD2ScanDto, StatusDto)

    DTO_FIELD_INFO(server) { info->description = "The INDI server status"; }
    DTO_FIELD(UnorderedFields<PHD2ExecutableDto>, server);
};

class PHDConfigDto : public oatpp::DTO {
    DTO_INIT(PHDConfigDto, DTO)

    DTO_FIELD_INFO(name) {
        info->description = "The name of PHD2 server configuration";
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(camera) {
        info->description =
            "The name of the camera, default is 'INDI Camera[xxx]'";
    }
    DTO_FIELD(String, camera);

    DTO_FIELD_INFO(telescope) {
        info->description =
            "The name of the telescope, default is 'INDI Mount[xxx]'";
    }
    DTO_FIELD(String, telescope);

    DTO_FIELD_INFO(focalLength) {
        info->description = "The focal length of the telescope, default is 0.0";
    }
    DTO_FIELD(Float64, focalLength);

    DTO_FIELD_INFO(pixelSize) {
        info->description = "The pixel size of the camera, default is 0.0";
    }
    DTO_FIELD(Float64, pixelSize);

    DTO_FIELD_INFO(massChangeThreshold) {
        info->description = "The mass change threshold, default is 0.0";
    }
    DTO_FIELD(Float64, massChangeThreshold);

    DTO_FIELD_INFO(calibrationDistance) {
        info->description = "The calibration distance, default is 0.0";
    }
    DTO_FIELD(Float64, calibrationDistance);

    DTO_FIELD_INFO(calibrationDuration) {
        info->description = "The calibration duration, default is 0.0";
    }
    DTO_FIELD(Float64, calibrationDuration);

    DTO_FIELD_INFO(massChangeFlag) {
        info->description = "The mass change flag";
    }
    DTO_FIELD(Boolean, massChangeFlag);
};

class RequestPHD2ConfigDto : public RequestDto {
    DTO_INIT(RequestPHD2ConfigDto, RequestDto)

    DTO_FIELD_INFO(path) {
        info->description =
            "The path of the PHD2 server configuration file directory";
        info->required = true;
    }
    DTO_FIELD(String, path) = "~/.phd2";
};

class ReturnPHD2ConfigDto : public StatusDto {
    DTO_INIT(ReturnPHD2ConfigDto, StatusDto)

    DTO_FIELD_INFO(configs) {
        info->description = "The PHD2 server configurations";
    }
    DTO_FIELD(List<PHDConfigDto>, configs);
};

class RequestPHD2StartDto : public RequestDto {
    DTO_INIT(RequestPHD2StartDto, RequestDto)

    DTO_FIELD_INFO(name) {
        info->description = "The name of the PHD2 configuration";
        info->required = true;
    }
    DTO_FIELD(String, name);

    DTO_FIELD_INFO(args) {
        info->description = "The arguments of the PHD2 server executable";
    }
    DTO_FIELD(Vector<String>, args);

    DTO_FIELD_INFO(env) {
        info->description = "The environment variables of the PHD2 server";
    }
    DTO_FIELD(UnorderedFields<String>, env);

    DTO_FIELD_INFO(workingDir) {
        info->description = "The working directory of the PHD2 server";
    }
    DTO_FIELD(String, workingDir);    
};

#include OATPP_CODEGEN_END(DTO)  ///< End DTO codegen section

#endif  // PHD2DTO_HPP
