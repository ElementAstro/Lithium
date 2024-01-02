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

#include <any>
#include <functional>
#include <memory>
#include <thread>

#ifdef ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/property/task/device_task.hpp"
#include "atom/type/args.hpp"
#include "atom/server/commander.hpp"
#include "atom/server/variables.hpp"

#include "device_exception.hpp"


class Device
{
public:
    explicit Device(const std::string &name);
    virtual ~Device();

public:
    virtual bool connect(const Args &params) { return true; };

    virtual bool disconnect(const Args &params) { return true; };

    virtual bool reconnect(const Args &params) { return true; };

    virtual bool isConnected() { return true; }

    virtual void init();

    const std::string getDeviceName();

    void insertProperty(const std::string &name, const std::any &value, const std::string &bind_get_func, const std::string &bind_set_func, const std::any &possible_values, PossibleValueType possible_type, bool need_check = false);

    void setProperty(const std::string &name, const std::any &value);

    std::any getProperty(const std::string &name, bool need_refresh = true);

    void removeProperty(const std::string &name);

    std::shared_ptr<INumberProperty> getNumberProperty(const std::string &name);

    std::shared_ptr<IStringProperty> getStringProperty(const std::string &name);

    std::shared_ptr<IBoolProperty> getBoolProperty(const std::string &name);

    void insertTask(const std::string &name, std::any defaultValue, Args params_template,
                    const std::function<Args(const Args &)> &func,
                    const std::function<Args(const Args &)> &stop_func,
                    bool isBlock = false);

    bool removeTask(const std::string &name);

    std::shared_ptr<Atom::Task::DeviceTask> getTask(const std::string &name, const Args &params);

private:
    // Why we use emhash8 : because it is so fast!
    // Map of the properties
#ifdef ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::any> m_properties;
    emhash8::HashMap<std::string, std::shared_ptr<DeviceTask>> m_task_map;
#else
    std::unordered_map<std::string, std::any> m_properties;
    std::unordered_map<std::string, std::shared_ptr<DeviceTask>> m_task_map;
#endif

    std::unique_ptr<CommandDispatcher<Args(Args)>> m_commander;

    // Basic Device name and UUID
    std::string _name;
    std::string _uuid;

public:
    typedef bool (*ConnectFunc)(const Args &params);
    typedef bool (*DisconnectFunc)(const Args &params);
    typedef bool (*ReconnectFunc)(const Args &params);
    typedef void (*InitFunc)();
    typedef void (*InsertPropertyFunc)(const std::string &name, const std::any &value, const std::string &bind_get_func, const std::string &bind_set_func, const std::any &possible_values, PossibleValueType possible_type, bool need_check);
    typedef void (*SetPropertyFunc)(const std::string &name, const std::any &value);
    typedef std::any (*GetPropertyFunc)(const std::string &name);
    typedef void (*RemovePropertyFunc)(const std::string &name);
    typedef void (*InsertTaskFunc)(const std::string &name, std::any defaultValue, Args params_template, const std::function<nlohmann::Args(const Args &)> &func, const std::function<nlohmann::Args(const Args &)> &stop_func, bool isBlock);
    typedef bool (*RemoveTaskFunc)(const std::string &name);
    typedef std::shared_ptr<Lithium::SimpleTask> (*GetTaskFunc)(const std::string &name, const Args &params);
    typedef void (*AddObserverFunc)(const std::function<void(const std::any &message)> &observer);
    typedef void (*RemoveObserverFunc)(const std::function<void(const std::any &message)> &observer);
    typedef const Args (*ExportDeviceInfoToArgsFunc)();
};

#endif // DEVICE_H
