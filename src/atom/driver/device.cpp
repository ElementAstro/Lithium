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
#include "atom/property/uuid.hpp"

#ifdef __cpp_lib_format
#include <format>
#else
#include <fmt/format.h>
#endif
#include <typeinfo>
#include <typeindex>

Device::Device(const std::string &name) : _name(name)
{
    _uuid = Atom::Property::UUIDGenerator::generateUUIDWithFormat();
}

Device::~Device()
{
}

void Device::init()
{
    setProperty("name", _name);
    setProperty("uuid", _uuid);
}

const std::string Device::getDeviceName()
{
    return _name;
}

void Device::insertProperty(const std::string &name, const std::any &value, const std::string &bind_get_func, const std::string &bind_set_func, const std::any &possible_values, PossibleValueType possible_type, bool need_check)
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
            m_properties[name] = property;
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
            m_properties[name] = property;
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
        try
        {
            if (m_properties[name].type() == typeid(std::shared_ptr<IStringProperty>) ||
                m_properties[name].type() == typeid(std::shared_ptr<INumberProperty>) ||
                m_properties[name].type() == typeid(std::shared_ptr<IBoolProperty>) ||
                m_properties[name].type() == typeid(std::shared_ptr<INumberVector>))
            {
                const auto property = m_properties[name];
                if (!std::any_cast<std::shared_ptr<IPropertyBase>>(property)->set_func.empty())
                {
#ifdef __cpp_lib_format
                    auto res = Dispatch(std::format("set_{}", name), {{name, value}});
#else
                    auto res = Dispatch(fmt::format("set_{}", name), {{name, value}});
#endif
                    if (res.find("error") != res.end())
                    {
                        try
                        {
                            throw DispatchError(std::any_cast<std::string>(res["error"]));
                        }
                        catch (const std::bad_any_cast &e)
                        {
                            throw InvalidReturn(e.what());
                        }
                    }
                }
                if (property.type() == typeid(std::shared_ptr<IStringProperty>))
                {
                    auto pp = std::any_cast<std::shared_ptr<IStringProperty>>(property);
                    pp->value = std::any_cast<std::string>(value);
                    m_properties[name] = pp;
                }
                else if (property.type() == typeid(std::shared_ptr<INumberProperty>))
                {
                    auto pp = std::any_cast<std::shared_ptr<INumberProperty>>(property);
                    pp->value = std::any_cast<double>(value);
                    m_properties[name] = pp;
                }
                else if (property.type() == typeid(std::shared_ptr<IBoolProperty>))
                {
                    auto pp = std::any_cast<std::shared_ptr<IBoolProperty>>(property);
                    pp->value = std::any_cast<bool>(value);
                    m_properties[name] = pp;
                }
                else if (property.type() == typeid(std::shared_ptr<INumberVector>))
                {
                    auto pp = std::any_cast<std::shared_ptr<INumberVector>>(property);
                    pp->value = std::any_cast<std::vector<double>>(value);
                    m_properties[name] = pp;
                }
                else
                {
#ifdef __cpp_lib_format
                    throw InvalidProperty(std::format("Unknown type of property {}", name));
#else

#endif
                }
            }
            else
            {
#ifdef __cpp_lib_format
                throw InvalidProperty(std::format("Unknown type of property {}", name));
#else
                throw InvalidProperty(fmt::format("Unknown type of property {}", name));
#endif
            }
        }
        catch (const std::bad_any_cast &e)
        {
#ifdef __cpp_lib_format
            throw InvalidProperty(std::format("Failed to convert property {} with {}", name, e.what()));
#else
            throw InvalidProperty(fmt::format("Failed to convert property {} with {}", name, e.what()));
#endif
        }
        if (!m_observers.empty())
        {
            for (const auto &observer : m_observers)
            {
                observer(getProperty(name, false));
            }
        }
    }
    else
    {
        insertProperty(name, value, "","", {}, PossibleValueType::None);
    }
}

std::any Device::getProperty(const std::string &name, bool need_refresh)
{
    if (m_properties.find(name) != m_properties.end())
    {
        if (need_refresh)
        {
            std::any property = m_properties[name];
            bool has_func = false;

            try
            {
                if (property.type() == typeid(std::shared_ptr<IStringProperty>) ||
                    property.type() == typeid(std::shared_ptr<INumberProperty>) ||
                    property.type() == typeid(std::shared_ptr<IBoolProperty>) ||
                    property.type() == typeid(std::shared_ptr<INumberVector>))
                {
                    if (!std::any_cast<std::shared_ptr<IPropertyBase>>(property)->get_func.empty())
                    {
                        has_func = true;
                    }
                }
                else
                {
#ifdef __cpp_lib_format
                    throw InvalidProperty(std::format("Unknown type of property {}", name));
#else
                    throw InvalidProperty(fmt::format("Unknown type of property {}", name));
#endif
                }
            }
            catch (const std::bad_any_cast &e)
            {
#ifdef __cpp_lib_format
                throw InvalidProperty(std::format("Failed to convert property {} with {}", name, e.what()));
#else
                throw InvalidProperty(fmt::format("Failed to convert property {} with {}", name, e.what()));
#endif
            }
            if (has_func)
            {
#ifdef __cpp_lib_format
                Dispatch(std::format("get_{}", name), {});
#else
                Dispatch(fmt::format("get_{}", name), {});
#endif
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
                return std::any_cast<std::shared_ptr<INumberProperty>>(property);
            }
            catch (const std::bad_any_cast &e)
            {
                throw InvalidProperty(e.what());
            }
        }
        else
            return nullptr;
    }
    catch (const std::bad_any_cast &e)
    {
        throw InvalidProperty(e.what());
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
                return std::any_cast<std::shared_ptr<IStringProperty>>(property);
            }
            catch (const std::exception &e)
            {
                throw InvalidProperty(e.what());
            }
        }
        else
            return nullptr;
    }
    catch (const std::bad_any_cast &e)
    {
        throw InvalidProperty(e.what());
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
                return std::any_cast<std::shared_ptr<IBoolProperty>>(property);
            }
            catch (const std::exception &e)
            {
                throw InvalidProperty(e.what());
            }
        }
        else
            return nullptr;
    }
    catch (const std::bad_any_cast &e)
    {
        throw InvalidProperty(e.what());
    }
}

void Device::removeProperty(const std::string &name)
{
    if (m_properties.find(name) != m_properties.end())
    {
        m_properties.erase(name);
    }
}

void Device::insertTask(const std::string &name, std::any defaultValue, Args params_template,
                        const std::function<Args(const Args &)> &func,
                        const std::function<Args(const Args &)> &stop_func,
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

std::shared_ptr<Atom::Task::SimpleTask> Device::getTask(const std::string &name, const Args &params)
{
    if (name.empty())
    {
        return nullptr;
    }
    if (task_map.find(name) != task_map.end())
    {
        auto tmp_task = task_map[name];
        tmp_task->setParams(params);
        if (tmp_task->validateJsonValue(params, tmp_task->getParamsTemplate()))
        {
            return tmp_task;
        }
    }
    return nullptr;
}
