/*
 * device_task.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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