/*
 * device_task.hpp
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

Date: 2023-7-19

Description: Device Task Definition

**************************************************/

#pragma once

#include "task.hpp"

class DeviceTask : public Atom::Task::SimpleTask
{
public:
    /**
     * @brief DeviceTask类构造函数
     * @param func 要执行的函数
     * @param params_template 参数模板，用于参数验证
     * @param device_name 设备名称
     * @param stop_fn 一个可选的停止函数，默认为nullptr
     * @param can_stop 指示任务是否可以停止，默认为false
     */
    DeviceTask(const std::function<json(const json &)> &func,
               const json &params_template,
               const std::string &device_name,
               const std::string &device_uuid,
               const std::string &device_device_name,
               const std::function<json(const json &)> &stop_fn,
               bool can_stop = false);

    /**
     * @brief 获取设备名称
     * @return 设备名称
     */
    const std::string &getDeviceName() const;

    /**
     * @brief 设置设备名称
     * @param device_name 要设置的设备名称
     */
    void setDeviceName(const std::string &device_name);

    /**
     * @brief 获取设备UUID
     * @return 设备UUID
     */
    const std::string &getDeviceUUID() const;

    /**
     * @brief 获取设备真实名称
     * @return 设备真实名称
     */
    const std::string &getDeviceDeviceName() const;

    /**
     * @brief 设置设备名称
     * @param device_name 要设置的设备名称
     */
    void setDeviceDeviceName(const std::string &device_device_name);

private:
    // 设备名称
    std::string device_name_;
    std::string device_uuid_;
    std::string device_device_name_;
};
