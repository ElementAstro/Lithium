/*
 * device.cpp
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

#include "device.hpp"
#include "liproperty/uuid.hpp"

#include "lidriver/util/utils.hpp"

#include <format>
#include <typeinfo>
#include <typeindex>

Device::Device(const std::string &name) : _name(name)
{
    UUID::UUIDGenerator generator;
    _uuid = generator.generateUUIDWithFormat();
}

Device::~Device()
{
    if (deviceIOServer->is_running())
    {
        deviceIOServer->stop();
    }
    loopThread.request_stop();
}

void Device::init()
{
    setStringProperty("name", _name);
    setStringProperty("uuid", _uuid);

    loopThread = ([&eventLoop]()
                  { eventLoop.start(); });
    deviceIOServer = std::make_shared<SocketServer>(eventLoop, generateRandomNumber(10000, 60000));
    deviceIOServer->start();
}

void Device::insertProperty(const std::string &name, const std::any &value, const std::string &bind_get_func, const std::string &bind_set_func, const std::any &possible_values, PossibleValueType possible_type, bool need_check = false)
{
    if (name.empty() || !value.has_value())
        throw InvalidParameters("Property name and value are required.");
    try
    {
        // IStringProperty
        if (value.type() == typeid(std::string) || value.type() == typeid(const char *))
        {
            std::shared_ptr<IStringProperty> property = std::make_shared<IStringProperty>();
            property->device_name = _name;
            property->name = name;
            property->device_uuid = _uuid;
            property->value = std::any_cast<std::string>(value);
            property->need_check = need_check;
            property->pv_type = possible_type;
            if (possible_values.has_value() && possible_type != PossibleValueType::None)
                property->possible_values = std::any_cast<std::vector<std::string>>(possible_values);
            m_properties[name] = property;
            if (!m_observers.empty())
                for (const auto &observer : m_observers)
                {
                    observer(property);
                }
        }
        // INumberProperty
        else if (value.type() == typeid(int) || value.type() == typeid(float) || value.type() == typeid(double))
        {
            std::shared_ptr<INumberProperty> property = std::make_shared<INumberProperty>();
            property->device_name = _name;
            property->name = name;
            property->device_uuid = _uuid;
            property->value = std::any_cast<double>(value);
            property->need_check = need_check;
            property->pv_type = possible_type;
            if (possible_values.has_value() && possible_type != PossibleValueType::None)
                property->possible_values = std::any_cast<std::vector<double>>(possible_values);
            property->get_func = bind_get_func;
            property->set_func = bind_set_func;
            number_properties[name] = property;
            if (!m_observers.empty())
                for (const auto &observer : m_observers)
                {
                    observer(property);
                }
        }
        // IBoolProperty
        else if (value.type() == typeid(bool))
        {
            std::shared_ptr<IBoolProperty> property = std::make_shared<IBoolProperty>();
            property->device_name = _name;
            property->name = name;
            property->device_uuid = _uuid;
            property->value = std::any_cast<bool>(value);
            property->need_check = need_check;
            property->pv_type = possible_type;
            if (possible_values.has_value() && possible_type != PossibleValueType::None)
                property->possible_values = std::any_cast<std::vector<bool>>(possible_values);
            bool_properties[name] = property;
            if (!m_observers.empty())
                for (const auto &observer : m_observers)
                {
                    observer(property);
                }
        }
    }
    catch (const std::bad_any_cast &e)
    {
        throw InvalidProperty(e.what());
    }
    catch (const std::exception &e)
    {
    }
}

void Device::setProperty(const std::string &name, const std::any &value)
{
    if (m_properties.find(name) != m_properties.end())
    {
        m_properties[name]->value = value;
        if (!m_properties[name]->set_func.empty())
        {
            auto res = Dispatch(std::format("set_{}", name), {{name, value}});
            if (res.find("error") != res.end())
            {
                try
                {
                    throw DispatchError(std::any_cast<std::string>(res["error"]))
                }
                catch (const std::bad_any_cast &e)
                {
                    throw InvalidReturn(e.what());
                }
            }
        }
        if (!m_observers.empty())
        {
            for (const auto &observer : m_observers)
            {
                observer(getProperty(name), false);
            }
        }
    }
}

std::any Device::getProperty(const std::string &name, bool need_refresh)
{
    if (m_properties.find(name) != m_properties.end())
    {
        if (need_refresh)
        {
            if (!m_properties[name]->get_func.empty())
            {
                Dispatch(std::format("get_{}", name), {});
            }
        }
        return m_properties[name];
    }
    return std::any();
}

std::shared_ptr<INumberProperty> Device::getNumberProperty(const std::string &name)
{
    try
    {
        auto property = getProperty("name");
        if (property.has_value())
        {
            try
            {
                return std::any_cast<std::shared_ptr<INumberProperty>>();
            }
            catch (const std::exception &e)
            {
                throw InvalidPproperty(e.what());
            }
        }
        else
            return nullptr;
    }
    catch (const std::bad_any_cast &e)
    {
        throw InvalidPproperty(e.what());
    }
}

std::shared_ptr<IStringProperty> Device::getStringProperty(const std::string &name)
{
    try
    {
        auto property = getProperty("name");
        if (property.has_value())
        {
            try
            {
                return std::any_cast<std::shared_ptr<IStringProperty>>();
            }
            catch (const std::exception &e)
            {
                throw InvalidPproperty(e.what());
            }
        }
        else
            return nullptr;
    }
    catch (const std::bad_any_cast &e)
    {
        throw InvalidPproperty(e.what());
    }
}

std::shared_ptr<IBoolProperty> Device::getBoolProperty(const std::string &name)
{
    try
    {
        auto property = getProperty("name");
        if (property.has_value())
        {
            try
            {
                return std::any_cast<std::shared_ptr<IBoolProperty>>();
            }
            catch (const std::exception &e)
            {
                throw InvalidPproperty(e.what());
            }
        }
        else
            return nullptr;
    }
    catch (const std::bad_any_cast &e)
    {
        throw InvalidPproperty(e.what());
    }
}

void Device::removeProperty(const std::string &name)
{
    if (m_properties.find(name) != m_properties.end())
    {
        m_properties.erase(name);
    }
}

void Device::insertTask(const std::string &name, std::any defaultValue, nlohmann::json params_template,
                        const std::function<nlohmann::json(const nlohmann::json &)> &func,
                        const std::function<nlohmann::json(const nlohmann::json &)> &stop_func,
                        bool isBlock)
{
    if (name.empty() || !defaultValue.has_value())
    {
        return;
    }
    if (!stop_func)
    {
        task_map[name] = std::make_shared<DeviceTask>(func, params_template, _name, _uuid, _name, stop_func, false);
    }
    else
    {
        task_map[name] = std::make_shared<DeviceTask>(func, params_template, _name, _uuid, _name, stop_func, true);
    }
}

bool Device::removeTask(const std::string &name)
{
    if (name.empty())
    {
        return false;
    }
    if (task_map.find(name) != task_map.end())
    {
        task_map.erase(name);
    }
    return true;
}

std::shared_ptr<Lithium::SimpleTask> Device::getTask(const std::string &name, const nlohmann::json &params)
{
    if (name.empty())
    {
        return nullptr;
    }
    if (task_map.find(name) != task_map.end())
    {
        auto tmp_task = task_map[name];
        tmp_task->SetParams(params);
        if (tmp_task->validateJsonValue(params, tmp_task->GetParamsTemplate()))
        {
            return tmp_task;
        }
    }
    return nullptr;
}

void Device::addObserver(const std::function<void(const std::any &message)> &observer)
{
    m_observers.push_back(observer);
}

void Device::removeObserver(const std::function<void(const std::any &message)> &observer)
{
    m_observers.erase(std::remove_if(m_observers.begin(), m_observers.end(),
                                     [&observer](const std::function<void(const std::any &message)> &o)
                                     {
                                         return o.target<std::function<void(const std::any &message)>>() == observer.target<std::function<void(const std::any &message)>>();
                                     }),
                      m_observers.end());
}

const nlohmann::json Device::exportDeviceInfoToJson()
{
    nlohmann::json jsonInfo;
    for (const auto &kv : string_properties)
    {
        jsonInfo[kv.first] = kv.second->value;
    }
    for (const auto &kv : number_properties)
    {
        jsonInfo[kv.first] = kv.second->value;
    }
    for (const auto &kv : bool_properties)
    {
        jsonInfo[kv.first] = kv.second->value;
    }
    std::cout << jsonInfo.dump(4) << std::endl;
    return jsonInfo;
}

bool Device::HasHandler(const std::string &name)
{
    return command_handlers_.find(Djb2Hash(name.c_str())) != command_handlers_.end();
}

IReturns Device::Dispatch(const std::string &name, const IParams &data)
{
    auto it = command_handlers_.find(Djb2Hash(name.c_str()));
    if (it != command_handlers_.end())
    {
        return it->second(data);
    }
    IReturns res;
    res["error"] = "Function not found";
    return res;
}

std::size_t Device::Djb2Hash(const char *str)
{
    std::size_t hash = 5381;
    char c;
    while ((c = *str++) != '\0')
    {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    return hash;
}