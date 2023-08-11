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

#ifndef DEVICE_H
#define DEVICE_H

#include <iostream>
#include <any>
#include <nlohmann/json.hpp>
#include <functional>
#include <memory>
#include <string_view>
#include <map>

#include "property/imessage.hpp"
#include "task/device_task.hpp"

class Device
{
public:
    explicit Device(const std::string &name);
    virtual ~Device();

public:
    virtual bool connect(const std::string &name) = 0;

    virtual bool disconnect() = 0;

    virtual bool reconnect() = 0;

    virtual void init();

    void setProperty(const std::string &name, const std::string &value);

    std::string getProperty(const std::string &name);

    void insertTask(const std::string &name, std::any defaultValue, nlohmann::json params_template,
                    const std::function<nlohmann::json(const nlohmann::json &)> &func,
                    const std::function<nlohmann::json(const nlohmann::json &)> &stop_func,
                    bool isBlock = false, std::shared_ptr<Lithium::SimpleTask> task = nullptr);

    bool removeTask(const std::string &name);

    std::shared_ptr<Lithium::SimpleTask> getTask(const std::string &name, const nlohmann::json &params);

    void insertMessage(const std::string &name, std::any value);

    void updateMessage(const std::string &name, const std::string &identifier, std::any newValue);

    void removeMessage(const std::string &name, const std::string &identifier);

    std::any getMessageValue(const std::string &name, const std::string &identifier);

    void addObserver(const std::function<void(const Lithium::IProperty &message)> &observer);

    void removeObserver(const std::function<void(const Lithium::IProperty &message)> &observer);

    void exportDeviceInfoToJson();

    Device &operator<<(const std::pair<std::string, std::string> &property);

    friend std::ostream &operator<<(std::ostream &os, const Device &device);

private:
    class DeviceInfo
    {
    public:
        std::map<std::string, std::string> properties;
        std::map<std::string, Lithium::IProperty> messages;
    };

    std::string _name;
    std::string _uuid;
    DeviceInfo device_info;
    std::unordered_map<std::string,std::shared_ptr<DeviceTask>> task_map;
    std::vector<std::function<void(Lithium::IProperty)>> observers;
};

#endif // DEVICE_H
