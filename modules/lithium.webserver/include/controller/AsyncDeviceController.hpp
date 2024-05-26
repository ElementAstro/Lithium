/*
 * AsyncDeviceController.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-17

Description: Device Routes

**************************************************/

#ifndef LITHIUM_ASYNC_DEVICE_CONTROLLER_HPP
#define LITHIUM_ASYNC_DEVICE_CONTROLLER_HPP

#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include "data/DeviceDto.hpp"
#include "data/IODto.hpp"

enum class DeviceType {
    Camera,
    Telescope,
    Focuser,
    FilterWheel,
    Solver,
    Guider,
    NumDeviceTypes
};

inline const char* DeviceTypeToString(DeviceType type) {
    switch (type) {
        case DeviceType::Camera:
            return "Camera";
            break;
        case DeviceType::Telescope:
            return "Telescope";
            break;
        case DeviceType::Focuser:
            return "Focuser";
            break;
        case DeviceType::FilterWheel:
            return "FilterWheel";
            break;
        case DeviceType::Solver:
            return "Solver";
            break;
        case DeviceType::Guider:
            return "Guider";
            break;
        default:
            return "Unknown";
            break;
    }
    return "Unknown";
}

inline DeviceType StringToDeviceType(const std::string& type) {
    if (type == "Camera")
        return DeviceType::Camera;
    else if (type == "Telescope")
        return DeviceType::Telescope;
    else if (type == "Focuser")
        return DeviceType::Focuser;
    else if (type == "FilterWheel")
        return DeviceType::FilterWheel;
    else if (type == "Solver")
        return DeviceType::Solver;
    else if (type == "Guider")
        return DeviceType::Guider;
    else
        return DeviceType::NumDeviceTypes;
}

#include "magic_enum/magic_enum.hpp"

#include "lithiumapp.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)  //<- Begin Codegen

class DeviceController : public oatpp::web::server::api::ApiController {
public:
    DeviceController(const std::shared_ptr<ObjectMapper>& objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper) {}

public:
    static std::shared_ptr<DeviceController> createShared(
        OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper)) {
        return std::make_shared<DeviceController>(objectMapper);
    }

public:
    // ----------------------------------------------------------------
    // Device Library Http Handler
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUIAddDeviceLibrary) {
        info->summary =
            "Add device library with information into DeviceManager";
        info->addConsumes<Object<AddDeviceLibraryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/add_device_library",
                   getUIAddDeviceLibrary) {
        ENDPOINT_ASYNC_INIT(getUIAddDeviceLibrary);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<AddDeviceLibraryDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIAddDeviceLibrary::returnResponse);
        }

        Action returnResponse(const oatpp::Object<AddDeviceLibraryDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->library_path.getValue("") == "" ||
                body->library_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device library path and name is required";
            } else {
                auto library_path = body->library_path.getValue("");
                auto library_name = body->library_name.getValue("");
                if (!lithium::MyApp->addDeviceLibrary(
                        {{"lib_path", library_path},
                         {"lib_name", library_name}})) {
                    res->error = "DeviceError";
                    res->message = "Failed to add device library";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRemoveDeviceLibrary) {
        info->summary =
            "Remove device library with information from DeviceManager";
        info->addConsumes<Object<RemoveDeviceLibraryDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/remove_device_library",
                   getUIRemoveDeviceLibrary) {
        ENDPOINT_ASYNC_INIT(getUIRemoveDeviceLibrary);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RemoveDeviceLibraryDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIRemoveDeviceLibrary::returnResponse);
        }

        Action returnResponse(
            const oatpp::Object<RemoveDeviceLibraryDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->library_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device library path and name is required";
            } else {
                auto library_name = body->library_name.getValue("");
                if (!lithium::MyApp->removeDeviceLibrary(library_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to add device library";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    // ----------------------------------------------------------------
    // Device Http Handler
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUIAddDevice) {
        info->summary = "Add device from device library into DeviceManager";
        info->addConsumes<Object<AddDeviceDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/add_device", getUIAddDevice) {
        ENDPOINT_ASYNC_INIT(getUIAddDevice);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<AddDeviceDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIAddDevice::returnResponse);
        }

        Action returnResponse(const oatpp::Object<AddDeviceDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "" ||
                body->device_type.getValue("") == "" ||
                body->library_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message =
                    "Device library path and device name and type are required";
            } else {
                auto device_name = body->device_name.getValue("");
                auto device_type = body->device_type.getValue("");
                auto library_name = body->library_name.getValue("");

                DeviceType device_type_ = StringToDeviceType(device_type);
                if (device_type_ == DeviceType::NumDeviceTypes) {
                    res->error = "Invalid Parameters";
                    res->message = "Unsupported device type";
                } else {
                    if (!lithium::MyApp->addDevice(
                            {{"type", device_type},
                             {"device_name", device_name},
                             {"library_name", library_name}}))
                        ;
                    {
                        res->error = "DeviceError";
                        res->message = "Failed to add device";
                    }
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIRemoveDevice) {
        info->summary = "Remove device with information from DeviceManager";
        info->addConsumes<Object<RemoveDeviceDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/remove_device", getUIRemoveDevice) {
        ENDPOINT_ASYNC_INIT(getUIRemoveDevice);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RemoveDeviceDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIRemoveDevice::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RemoveDeviceDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_name.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    // ----------------------------------------------------------------
    // Device Property Functions
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUIGetProperty) {
        info->summary = "Get a specific property from the specified device";
        info->addConsumes<Object<GetPropertyDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/get_property", getUIGetProperty) {
        ENDPOINT_ASYNC_INIT(getUIGetProperty);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<GetPropertyDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIGetProperty::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetPropertyDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_name.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUISetProperty) {
        info->summary =
            "Set a specific property from the specified device with new "
            "property "
            "value";
        info->addConsumes<Object<SetPropertyDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/set_property", getUISetProperty) {
        ENDPOINT_ASYNC_INIT(getUISetProperty);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<SetPropertyDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUISetProperty::returnResponse);
        }

        Action returnResponse(const oatpp::Object<SetPropertyDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_name.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    // ----------------------------------------------------------------
    // Device Task Functions
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUIRunDeviceFunc) {
        info->summary = "Run a specific task from device";
        info->addConsumes<Object<RunDeviceFuncDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/run_device_func", getUIRunDeviceFunc) {
        ENDPOINT_ASYNC_INIT(getUIRunDeviceFunc);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<RunDeviceFuncDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIRunDeviceFunc::returnResponse);
        }

        Action returnResponse(const oatpp::Object<RunDeviceFuncDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_name.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIGetDeviceFunc) {
        info->summary = "Get a specific task's infomation from device";
        info->addConsumes<Object<GetDeviceFuncDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/get_device_func", getUIGetDeviceFunc) {
        ENDPOINT_ASYNC_INIT(getUIGetDeviceFunc);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<GetDeviceFuncDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIGetDeviceFunc::returnResponse);
        }

        Action returnResponse(const oatpp::Object<GetDeviceFuncDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_name.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    // ----------------------------------------------------------------
    // Device Common Interface
    // ----------------------------------------------------------------

    ENDPOINT_INFO(getUIConnectDevice) {
        info->summary =
            "Connect to a specific device with name (must be unique).";
        info->addConsumes<Object<GetDeviceFuncDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/connect", getUIConnectDevice) {
        ENDPOINT_ASYNC_INIT(getUIConnectDevice);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<ConnectDeviceDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIConnectDevice::returnResponse);
        }

        Action returnResponse(const oatpp::Object<ConnectDeviceDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_name.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIDisconnectDevice) {
        info->summary =
            "Disconnect from a specific device with name (must be unique).";
        info->addConsumes<Object<DisconnectDeviceDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/disconnect", getUIDisconnectDevice) {
        ENDPOINT_ASYNC_INIT(getUIDisconnectDevice);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<DisconnectDeviceDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIDisconnectDevice::returnResponse);
        }

        Action returnResponse(const oatpp::Object<DisconnectDeviceDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_name.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIReconnectDevice) {
        info->summary =
            "Reconnect to a specific device with name (must be unique and the "
            "device must already connect successfully).";
        info->addConsumes<Object<ReconnectDeviceDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/reconnect", getUIReconnectDevice) {
        ENDPOINT_ASYNC_INIT(getUIReconnectDevice);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<ReconnectDeviceDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIReconnectDevice::returnResponse);
        }

        Action returnResponse(const oatpp::Object<ReconnectDeviceDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_name.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_name.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };

    ENDPOINT_INFO(getUIScanDevice) {
        info->summary =
            "Scan a specific type of the devices and return a list of "
            "available "
            "devices.";
        info->addConsumes<Object<ScanDeviceDTO>>("application/json");
        info->addResponse<Object<StatusDto>>(Status::CODE_200,
                                             "application/json");
    }
    ENDPOINT_ASYNC("GET", "/api/device/scan", getUIScanDevice) {
        ENDPOINT_ASYNC_INIT(getUIScanDevice);
        Action act() override {
            return request
                ->readBodyToDtoAsync<oatpp::Object<ScanDeviceDTO>>(
                    controller->getDefaultObjectMapper())
                .callbackTo(&getUIScanDevice::returnResponse);
        }

        Action returnResponse(const oatpp::Object<ScanDeviceDTO>& body) {
            auto res = StatusDto::createShared();
            if (body->device_type.getValue("") == "") {
                res->error = "Invalid Parameters";
                res->message = "Device name is required";
            } else {
                auto device_name = body->device_type.getValue("");
                if (!lithium::MyApp->removeDeviceByName(device_name)) {
                    res->error = "DeviceError";
                    res->message = "Failed to remove device";
                }
            }
            return _return(
                controller->createDtoResponse(Status::CODE_200, res));
        }
    };
};

#include OATPP_CODEGEN_END(ApiController)  //<- End Codegen

#endif  // LITHIUM_ASYNC_DEVICE_CONTROLLER_HPP
