/*
 * device.hpp
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

Date: 2023-6-1

Description: Basic Device Defination

*************************************************/

#pragma once

#include <string>
#include <vector>
#include <functional>

#include "modules/task/task.hpp"
#include "modules/property/imessage.hpp"

#include "nlohmann/json.hpp"

enum class DeviceType
{
    Camera,
    Telescope,
    Focuser,
    FilterWheel,
    Solver,
    Guider,
    NumDeviceTypes
};

static constexpr int DeviceTypeCount = 6;

/**
 * @brief 设备基类
 */
class Device
{
public:
    /**
     * @brief 构造函数
     *
     * @param name 设备名称
     */
    explicit Device(const std::string &name);

    /**
     * @brief 析构函数
     */
    virtual ~Device(){};

    /**
     * @brief 连接设备
     *
     * @param name 设备名称
     * @return 连接设备是否成功
     */
    virtual bool connect(const std::string &name) = 0;

    /**
     * @brief 断开设备连接
     *
     * @return 断开设备连接是否成功
     */
    virtual bool disconnect() = 0;

    /**
     * @brief 重新连接设备
     *
     * @return 重新连接设备是否成功
     */
    virtual bool reconnect() = 0;

    /**
     * @brief 扫描可用设备
     *
     * @return 扫描可用设备是否成功
     */
    virtual std::vector<std::string> scanForAvailableDevices() = 0;

    /**
     * @brief 获取SimpleTask
     *
     * @param task_name 任务名
     * @param params 参数
     * @return SimpleTask指针
     */
    virtual std::shared_ptr<Lithium::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) = 0;

public:
    auto IAFindMessage(const std::string &identifier);

    void IASetProperty(const std::string &name, const std::string &value);

    std::string IAGetProperty(const std::string &name);

    void IAInsertMessage(const Lithium::Property::IMessage &message, std::shared_ptr<Lithium::SimpleTask> task);

    Lithium::Property::IMessage IACreateMessage(const std::string &message_name, std::any message_value);

    void IAUpdateMessage(const std::string &identifier, const Lithium::Property::IMessage &newMessage);

    void IARemoveMessage(const std::string &identifier);

    Lithium::Property::IMessage *IAGetMessage(const std::string &identifier);

    void IANotifyObservers(const Lithium::Property::IMessage &newMessage, const Lithium::Property::IMessage &oldMessage);

    void IANotifyObservers(const Lithium::Property::IMessage &removedMessage);

    struct MessageInfo
    {
        Lithium::Property::IMessage message;
        std::shared_ptr<Lithium::SimpleTask> task;
    };

    std::vector<MessageInfo> device_messages;

    std::vector<std::function<void(const Lithium::Property::IMessage &, const Lithium::Property::IMessage &)>> observers;

private:
    nlohmann::json device_info;
};