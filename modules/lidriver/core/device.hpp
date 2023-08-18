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
#include <functional>
#include <memory>
#include <string_view>
#include <map>

#include <nlohmann/json.hpp>
#include "emhash/hash_table8.hpp"

#include "liproperty/iproperty.hpp"
#include "liproperty/task/device_task.hpp"

#define REGISTER_COMMAND_MEMBER(commandName, memberFunction) \
    registerCommand(commandName, [this]() { memberFunction(); })

class Device
{
public:
    explicit Device(const std::string &name);
    virtual ~Device();

public:
    virtual bool connect(const std::string &name) { return true; };

    virtual bool disconnect() { return true; };

    virtual bool reconnect() { return true; };

    virtual void init();

    void insertNumberProperty(const std::string &name, const double &value, std::vector<double> possible_values, PossibleValueType possible_type, bool need_check = false);

    void insertNumberBindProperty(const std::string &name, const std::string &bind_get_func, const std::string &bind_set_func, const double &value, std::vector<double> possible_values, PossibleValueType possible_type, bool need_check = false);

    void setNumberProperty(const std::string &name, const double &value);

    std::shared_ptr<INumberProperty> getNumberProperty(const std::string &name);

    void insertStringProperty(const std::string &name, const std::string &value, std::vector<std::string> possible_values, PossibleValueType possible_type, bool need_check = false);

    void insertStringBindProperty(const std::string &name, const std::string &bind_get_func, const std::string &bind_set_func, const std::string &value, std::vector<std::string> possible_values, PossibleValueType possible_type, bool need_check = false);

    void setStringProperty(const std::string &name, const std::string &value);

    std::shared_ptr<IStringProperty> getStringProperty(const std::string &name);

    void insertBoolProperty(const std::string &name, const bool &value, std::vector<bool> possible_values, PossibleValueType possible_type, bool need_check = false);

    void insertBoolBindProperty(const std::string &name, const std::string &bind_get_func, const std::string &bind_set_func, const bool &value, std::vector<bool> possible_values, PossibleValueType possible_type, bool need_check = false);

    void setBoolProperty(const std::string &name, const bool &value);

    std::shared_ptr<IBoolProperty> getBoolProperty(const std::string &name);

    void removeStringProperty(const std::string &name);

    void removeNumberProperty(const std::string &name);

    void removeBoolProperty(const std::string &name);

    void insertTask(const std::string &name, std::any defaultValue, nlohmann::json params_template,
                    const std::function<nlohmann::json(const nlohmann::json &)> &func,
                    const std::function<nlohmann::json(const nlohmann::json &)> &stop_func,
                    bool isBlock = false);

    bool removeTask(const std::string &name);

    std::shared_ptr<Lithium::SimpleTask> getTask(const std::string &name, const nlohmann::json &params);

    void addStringObserver(const std::function<void(const std::shared_ptr<IStringProperty> &message)> &observer);

    void removeStringObserver(const std::function<void(const std::shared_ptr<IStringProperty> &message)> &observer);

    void addNumberObserver(const std::function<void(const std::shared_ptr<INumberProperty> &message)> &observer);

    void removeNumberObserver(const std::function<void(const std::shared_ptr<INumberProperty> &message)> &observer);

    void addBoolObserver(const std::function<void(const std::shared_ptr<IBoolProperty> &message)> &observer);

    void removeBoolObserver(const std::function<void(const std::shared_ptr<IBoolProperty> &message)> &observer);

    const nlohmann::json exportDeviceInfoToJson();

    template <typename Function>
    void registerCommand(const std::string &commandName, Function &&handler)
    {
        commandMap[commandName] = [handler]()
        {
            try
            {
                handler();
            }
            catch (const std::exception &e)
            {
            }
        };
    }

    template <typename Function, typename... Args>
    void registerCommand(const std::string &commandName, Function &&handler, Args &&...args)
    {
        auto boundHandler = std::bind(std::forward<Function>(handler), std::forward<Args>(args)...);
        commandMap[commandName] = [boundHandler]()
        {
            try
            {
                boundHandler();
            }
            catch (const std::exception &e)
            {
            }
        };
    }

    template <typename... Args>
    void invokeCommand(const std::string &commandName, Args &&...args)
    {
        auto it = commandMap.find(commandName);
        if (it != commandMap.end())
        {
            try
            {
                it->second();
            }
            catch (const std::exception &e)
            {
            }
        }
    }

private:
    // Why we use emhash8 : because it is so fast!
    // Different types of properties
    emhash8::HashMap<std::string, std::shared_ptr<INumberProperty>> number_properties;
    emhash8::HashMap<std::string, std::shared_ptr<IStringProperty>> string_properties;
    emhash8::HashMap<std::string, std::shared_ptr<IBoolProperty>> bool_properties;
    emhash8::HashMap<std::string, std::shared_ptr<INumberVector>> number_vector_properties;

    emhash8::HashMap<std::string, std::function<void()>> commandMap;
    emhash8::HashMap<std::string, std::function<void(const std::string &)>> stringCommandMap;
    emhash8::HashMap<std::string, std::function<void(const double &)>> numberCommandMap;
    emhash8::HashMap<std::string, std::function<void(const bool &)>> boolCommandMap;

    // Observers of different properties
    std::vector<std::function<void(std::shared_ptr<INumberProperty>)>> number_observers;
    std::vector<std::function<void(std::shared_ptr<IStringProperty>)>> string_observers;
    std::vector<std::function<void(std::shared_ptr<IBoolProperty>)>> bool_observers;
    std::vector<std::function<void(std::shared_ptr<INumberVector>)>> number_vector_observers;
    // Basic Device name and UUID
    std::string _name;
    std::string _uuid;
    // Map of the task
    emhash8::HashMap<std::string, std::shared_ptr<DeviceTask>> task_map;

public:
    // insertNumberProperty
    typedef void (*INP)(const std::string &, const double &, std::vector<double>, PossibleValueType, bool);

    // setNumberProperty
    typedef void (*SNP)(const std::string &, const double &);

    // getNumberProperty
    typedef std::shared_ptr<INumberProperty> (*GNP)(const std::string &);

    // insertStringProperty
    typedef void (*ISP)(const std::string &, const std::string &, std::vector<std::string>, PossibleValueType, bool);

    // setStringProperty
    typedef void (*SSP)(const std::string &, const std::string &);

    // getStringProperty
    typedef std::shared_ptr<IStringProperty> (*GSP)(const std::string &);

    // insertBoolProperty
    typedef void (*IBP)(const std::string &, const bool &, std::vector<bool>, PossibleValueType, bool);

    // setBoolProperty
    typedef void (*SBP)(const std::string &, const bool &);

    // getBoolProperty
    typedef std::shared_ptr<IBoolProperty> (*GBP)(const std::string &);

    // removeStringProperty
    typedef void (*RSP)(const std::string &);

    // removeNumberProperty
    typedef void (*RNP)(const std::string &);

    // removeBoolProperty
    typedef void (*RBP)(const std::string &);
};

#endif // DEVICE_H
