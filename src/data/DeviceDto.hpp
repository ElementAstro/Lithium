/*
 * DeviceDto.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-11-17

Description: Data Transform Object for Device Controller

**************************************************/

#ifndef DEVICEDTO_HPP
#define DEVICEDTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO) ///< Begin DTO codegen section

class AddDeviceLibraryDTO : public oatpp::DTO
{
    DTO_INIT(AddDeviceLibraryDTO, DTO)

    DTO_FIELD_INFO(library_path)
    {
        info->description = "Path of the device library to add";
        info->required = true;
    }
    DTO_FIELD(String, library_path);

    DTO_FIELD_INFO(library_name)
    {
        info->description = "Name of the device library to add";
        info->required = true;
    }
    DTO_FIELD(String, library_name);
};

class RemoveDeviceLibraryDTO : public oatpp::DTO
{
    DTO_INIT(RemoveDeviceLibraryDTO, DTO)

    DTO_FIELD_INFO(library_name)
    {
        info->description = "Name of the device library to remove";
        info->required = true;
    }
    DTO_FIELD(String, library_name);
};

class AddDeviceDTO : public oatpp::DTO
{
    DTO_INIT(AddDeviceDTO, DTO)

    DTO_FIELD_INFO(library_name)
    {
        info->description = "Name of the device library to add from";
        info->required = true;
    }
    DTO_FIELD(String, library_name);

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to add";
        info->required = true;
    }
    DTO_FIELD(String, device_name);

    DTO_FIELD_INFO(device_type)
    {
        info->description = "Type of device to add";
        info->required = true;
    }
    DTO_FIELD(String, device_type);
};

class RemoveDeviceDTO : public oatpp::DTO
{
    DTO_INIT(RemoveDeviceDTO, DTO)

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to remove";
        info->required = true;
    }
    DTO_FIELD(String, device_name);
};

class GetPropertyDTO : public oatpp::DTO
{
    DTO_INIT(GetPropertyDTO, DTO)

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to get property";
        info->required = true;
    }
    DTO_FIELD(String, device_name);

    DTO_FIELD_INFO(property_name)
    {
        info->description = "Name of the property to get";
        info->required = true;
    }
    DTO_FIELD(String, property_name);

    DTO_FIELD_INFO(need_update)
    {
        info->description = "Whether the property should be updated";
        info->required = false;
    }
    DTO_FIELD(String, need_update);
};

class SetPropertyDTO : public oatpp::DTO
{
    DTO_INIT(SetPropertyDTO, DTO)

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to set property";
        info->required = true;
    }
    DTO_FIELD(String, device_name);

    DTO_FIELD_INFO(property_name)
    {
        info->description = "Name of the property to set";
        info->required = true;
    }
    DTO_FIELD(String, property_name);

    DTO_FIELD_INFO(property_value)
    {
        info->description = "Value of the property to set";
        info->required = true;
    }
    DTO_FIELD(String, property_value);

    DTO_FIELD_INFO(property_type)
    {
        info->description = "Type of the property to set";
        info->required = true;
    }
    DTO_FIELD(String, property_type);
};

class RunDeviceFuncDTO : public oatpp::DTO
{
    DTO_INIT(RunDeviceFuncDTO, DTO)

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to set property";
        info->required = true;
    }
    DTO_FIELD(String, device_name);

    DTO_FIELD_INFO(task_name)
    {
        info->description = "Name of the task to run of the specified device";
        info->required = true;
    }
    DTO_FIELD(String, task_name);

    DTO_FIELD_INFO(task_params)
    {
        info->description = "Parameters for the task to run of the specified device.(in JSON format)";
        info->required = true;
    }
    DTO_FIELD(String, task_params);

    DTO_FIELD_INFO(need_async)
    {
        info->description = "Whether run task in async mode(default)";
        info->required = false;
    }
    DTO_FIELD(Boolean, need_async);
};

class GetDeviceFuncDTO : public oatpp::DTO
{
    DTO_INIT(GetDeviceFuncDTO, DTO)

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to get function infomation";
        info->required = true;
    }
    DTO_FIELD(String, device_name);

    DTO_FIELD_INFO(func_name)
    {
        info->description = "Name of the function to get information";
        info->required = true;
    }
    DTO_FIELD(String, func_name);
};

class ConnectDeviceDTO : public oatpp::DTO
{
    DTO_INIT(ConnectDeviceDTO, DTO)

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to connect";
        info->required = true;
    }
    DTO_FIELD(String, device_name);

    DTO_FIELD_INFO(auto_reconnect)
    {
        info->description = "Whether to automatically reconnect the device (default: true).";
        info->required = false;
    }
    DTO_FIELD(Boolean, auto_reconnect);

    DTO_FIELD_INFO(times_to_connect)
    {
        info->description = "Times to connect the device (default: 3). Automatically try to connect the device 3 times at a time";
        info->required = false;
    }
    DTO_FIELD(Int32, times_to_connect);
};

class DisconnectDeviceDTO : public oatpp::DTO
{
    DTO_INIT(DisconnectDeviceDTO, DTO)

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to disconnect(must match the unique device name)";
        info->required = true;
    }
    DTO_FIELD(String ,device_name);

    DTO_FIELD_INFO(must_success)
    {
        info->description = "If this option is true, the device will be erased from device vector until it is disconnected successfully";
        info->required = false;
    }
    DTO_FIELD(Boolean ,must_success);
};

class ReconnectDeviceDTO : public oatpp::DTO
{
    DTO_INIT(ReconnectDeviceDTO, DTO)

    DTO_FIELD_INFO(device_name)
    {
        info->description = "Name of the device to reconnect";
        info->required = true;
    }
    DTO_FIELD(String, device_name);
};

class ScanDeviceDTO : public oatpp::DTO
{
    DTO_INIT(ScanDeviceDTO, DTO)

    DTO_FIELD_INFO(device_type)
    {
        info->description = "Type of the device to scan, from device mamanger";
        info->required = true;
    }
    DTO_FIELD(String, device_type);
};

#include OATPP_CODEGEN_END(DTO) ///< End DTO codegen section

#endif