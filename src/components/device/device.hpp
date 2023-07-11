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

#include "task/task.hpp"

#include "nlohmann/json.hpp"
#include "property/imessage.hpp"

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

    Device(){};

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
    virtual bool connect(std::string name) = 0;

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
    virtual bool scanForAvailableDevices() = 0;

    /**
     * @brief 获取设备设置
     *
     * @return 获取设备设置是否成功
     */
    virtual bool getSettings() = 0;

    /**
     * @brief 保存设备设置
     *
     * @return 保存设备设置是否成功
     */
    virtual bool saveSettings() = 0;

    /**
     * @brief 获取设备参数
     *
     * @param paramName 参数名称
     * @return 获取设备参数是否成功
     */
    virtual bool getParameter(const std::string &paramName) = 0;

    /**
     * @brief 设置设备参数
     *
     * @param paramName 参数名称
     * @param paramValue 参数值
     * @return 设置设备参数是否成功
     */
    virtual bool setParameter(const std::string &paramName, const std::string &paramValue) = 0;

    /**
     * @brief 获取SimpleTask
     *
     * @param task_name 任务名
     * @param params 参数
     * @return SimpleTask指针
     */
    virtual std::shared_ptr<OpenAPT::SimpleTask> getSimpleTask(const std::string &task_name, const nlohmann::json &params) = 0;

    /**
     * @brief 获取ConditionalTask
     *
     * @param task_name 任务名
     * @param params 参数
     * @return ConditionalTask指针
     */
    virtual std::shared_ptr<OpenAPT::ConditionalTask> getCondtionalTask(const std::string &task_name, const nlohmann::json &params) = 0;

    /**
     * @brief 获取LoopTask
     *
     * @param task_name 任务名
     * @param params 参数
     * @return LoopTask指针
     */
    virtual std::shared_ptr<OpenAPT::LoopTask> getLoopTask(const std::string &task_name, const nlohmann::json &params) = 0;

    /**
     * @brief 获取设备名称
     *
     * @return 设备名称
     */
    virtual std::string getName() const = 0;

    /**
     * @brief 设置设备名称
     *
     * @param name 设备名称
     * @return 设置设备名称是否成功
     */
    virtual void setName(const std::string &name) = 0;

    virtual std::string getDeviceName() = 0;

    virtual void setDeviceName(const std::string &name) = 0;

    const std::string getId() const
    {
        return _uuid;
    }

    /**
     * @brief 设置设备ID
     *
     * @param id 设备ID
     * @return 设置设备ID是否成功
     */
    virtual void setId(int id) = 0;

public:
    auto IAFindMessage(const std::string &identifier);

    void IAInsertMessage(const OpenAPT::Property::IMessage &message,std::shared_ptr<OpenAPT::SimpleTask> task);

    OpenAPT::Property::IMessage IACreateMessage(const std::string &message_name, std::any message_value);

    void IAUpdateMessage(const std::string &identifier, const OpenAPT::Property::IMessage &newMessage);

    void IARemoveMessage(const std::string &identifier);

    OpenAPT::Property::IMessage *IAGetMessage(const std::string &identifier);

    void IANotifyObservers(const OpenAPT::Property::IMessage &newMessage, const OpenAPT::Property::IMessage &oldMessage);

    void IANotifyObservers(const OpenAPT::Property::IMessage &removedMessage);

    struct MessageInfo
    {
        OpenAPT::Property::IMessage message;
        std::shared_ptr<OpenAPT::SimpleTask> task;
    };

    std::vector<MessageInfo> device_messages;

    std::vector<std::function<void(const OpenAPT::Property::IMessage &, const OpenAPT::Property::IMessage &)>> observers;

public:
    std::string _name;                  ///< 设备名称
    std::string _uuid;                  ///< 设备ID
    std::string device_name;            ///< 设备名称
    std::string description;            ///< 设备描述信息
    std::string configPath;             ///< 配置文件路径
    std::string hostname = "127.0.0.1"; ///< 主机名
    int port = 7624;                    ///< 端口号
    bool is_connected;                  ///< 是否已连接
    bool is_debug;                      ///< 调试模式
};