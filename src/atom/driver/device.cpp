/*
 * device.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-1

Description: Basic Device Defination

*************************************************/

#include "device.hpp"
#include "atom/utils/uuid.hpp"

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
                    auto res = m_commander->Dispatch(std::format("set_{}", name), std::any_cast<std::string>(value));
#else
                    auto res = m_commander->Dispatch(fmt::format("set_{}", name), {{name, value}});
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
                m_commander->Dispatch(std::format("get_{}", name), {});
#else
                m_commander->Dispatch(fmt::format("get_{}", name), {});
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

void Device::insertTask(const std::string &name, std::any defaultValue, json params_template,
                        const std::function<json(const json &)> &func,
                        const std::function<json(const json &)> &stop_func,
                        bool isBlock)
{
    if (name.empty() || !defaultValue.has_value())
    {
        return;
    }
}

bool Device::removeTask(const std::string &name)
{
    if (name.empty())
    {
        return false;
    }
    return true;
}

std::shared_ptr<DeviceTask> Device::getTask(const std::string &name, const json &params)
{
    if (name.empty())
    {
        return nullptr;
    }
    return nullptr;
}
