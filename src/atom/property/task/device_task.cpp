/*
 * device_task.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-19

Description: Device Task Definition

**************************************************/

#include "device_task.hpp"

DeviceTask::DeviceTask(const std::function<json(const json &)> &func,
           const json &params_template,
           const std::string &device_name,
           const std::string &device_uuid,
           const std::string &device_device_name,
           const std::function<json(const json &)> &stop_fn,
           bool can_stop)
    : SimpleTask(func, params_template, stop_fn, can_stop), device_name_(device_name), device_uuid_(device_uuid), device_device_name_(device_device_name)
{
}

const std::string &DeviceTask::getDeviceName() const
{
    return device_name_;
}

void DeviceTask::setDeviceName(const std::string &device_name)
{
    device_name_ = device_name;
}

const std::string &DeviceTask::getDeviceUUID() const
{
    return device_uuid_;
}

const std::string &DeviceTask::getDeviceDeviceName() const
{
    return device_device_name_;
}

void DeviceTask::setDeviceDeviceName(const std::string &device_device_name)
{
    device_device_name_ = device_device_name;
}