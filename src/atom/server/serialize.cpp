/*
 * serialize.cpp
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

Date: 2023-11-3

Description: This file contains the declaration of the SerializationEngine class

**************************************************/

#include "serialize.hpp"

#include <sstream>
#include <ostream>
#include <iomanip>
#ifdef __cpp_lib_format
#include <format>
#else
#include <fmt/format.h>
#endif

#include "property/iproperty.hpp"
#include "atom/type/iparams.hpp"
#include "atom/log/loguru.hpp"

std::string JsonSerializationEngine::serialize(const std::any &data, bool format) const
{
    std::unordered_map<std::string, std::string> _data;
    if (data.type() == typeid(std::unordered_map<std::string, std::string>))
    {
        try
        {
            _data = std::any_cast<std::unordered_map<std::string, std::string>>(data);
        }
        catch (const std::exception &e)
        {
            LOG_F(ERROR, "Failed to serialize message: {}", e.what());
        }
    }
    else
    {
        if (data.type() == typeid(std::shared_ptr<IBoolProperty>))
        {
            try
            {
                std::shared_ptr<IBoolProperty> prop = std::any_cast<std::shared_ptr<IBoolProperty>>(data);
                _data["device_name"] = prop->device_name;
                _data["device_uuid"] = prop->device_uuid;
                _data["message_uuid"] = prop->message_uuid;
                _data["name"] = prop->name;
                _data["need_check"] = prop->need_check ? "true" : "false";
                _data["get_func"] = prop->get_func;
                _data["set_func"] = prop->set_func;
                _data["value"] = prop->value ? "true" : "false";
            }
            catch (const std::exception &e)
            {
                LOG_F(ERROR, "Failed to serialize bool property message: {}", e.what());
            }
        }
        else if (data.type() == typeid(std::shared_ptr<IStringProperty>))
        {
            try
            {
                std::shared_ptr<IStringProperty> prop = std::any_cast<std::shared_ptr<IStringProperty>>(data);
                _data["device_name"] = prop->device_name;
                _data["device_uuid"] = prop->device_uuid;
                _data["message_uuid"] = prop->message_uuid;
                _data["name"] = prop->name;
                _data["need_check"] = prop->need_check ? "true" : "false";
                _data["get_func"] = prop->get_func;
                _data["set_func"] = prop->set_func;
                _data["value"] = prop->value;
            }
            catch (const std::exception &e)
            {
                LOG_F(ERROR, "Failed to serialize bool property message: {}", e.what());
            }
        }
        else if (data.type() == typeid(std::shared_ptr<INumberProperty>))
        {
            try
            {
                std::shared_ptr<INumberProperty> prop = std::any_cast<std::shared_ptr<INumberProperty>>(data);
                _data["device_name"] = prop->device_name;
                _data["device_uuid"] = prop->device_uuid;
                _data["message_uuid"] = prop->message_uuid;
                _data["name"] = prop->name;
                _data["need_check"] = prop->need_check ? "true" : "false";
                _data["get_func"] = prop->get_func;
                _data["set_func"] = prop->set_func;
                _data["value"] = std::to_string(prop->value);
            }
            catch (const std::exception &e)
            {
                LOG_F(ERROR, "Failed to serialize bool property message: {}", e.what());
            }
        }
        else if (data.type() == typeid(std::shared_ptr<IParams>))
        {
            try
            {
                return std::any_cast<std::shared_ptr<IParams>>(data)->toJson();
            }
            catch (const std::bad_any_cast &e)
            {
                LOG_F(ERROR, "Failed to serialize bool property message: {}", e.what());
            }
        }
        else
        {
            LOG_F(ERROR, "Unknown type of message!");
            return "";
        }
    }
    std::ostringstream oss;
    if (format)
    {
        oss << "{\n";
        for (const auto &pair : _data)
        {
            oss << "  \"" << pair.first << "\": \"" << pair.second << "\",\n";
        }
        oss.seekp(-2, std::ios_base::end);
        oss << "\n}";
    }
    else
    {
        oss << "{";
        for (const auto &pair : _data)
        {
            oss << "\"" << pair.first << "\": \"" << pair.second << "\", ";
        }
        oss.seekp(-2, std::ios_base::end);
        oss << "}";
    }
    return oss.str();
}

std::string XmlSerializationEngine::serialize(const std::any &data, bool format) const
{
    std::unordered_map<std::string, std::string> _data;
    try
    {
        _data = std::any_cast<std::unordered_map<std::string, std::string>>(data);
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to serialize message: {}", e.what());
    }
    std::ostringstream oss;
    if (format)
    {
        oss << "<root>\n";
        for (const auto &pair : _data)
        {
            oss << "  <" << pair.first << ">" << pair.second << "</" << pair.first << ">\n";
        }
        oss << "</root>";
    }
    else
    {
        oss << "<root>";
        for (const auto &pair : _data)
        {
            oss << "<" << pair.first << ">" << pair.second << "</" << pair.first << ">";
        }
        oss << "</root>";
    }
    return oss.str();
}

std::string YamlSerializationEngine::serialize(const std::any &data, bool format) const
{
    std::unordered_map<std::string, std::string> _data;
    try
    {
        _data = std::any_cast<std::unordered_map<std::string, std::string>>(data);
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to serialize message: {}", e.what());
    }
    std::ostringstream oss;
    if (format)
    {
        for (const auto &pair : _data)
        {
            oss << std::setw(2) << pair.first << ": " << pair.second << std::endl;
        }
    }
    else
    {
        for (const auto &pair : _data)
        {
            oss << pair.first << ": " << pair.second << std::endl;
        }
    }
    return oss.str();
}

std::string IniSerializationEngine::serialize(const std::any &data, bool format) const
{
    std::unordered_map<std::string, std::string> _data;
    try
    {
        _data = std::any_cast<std::unordered_map<std::string, std::string>>(data);
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Failed to serialize message: {}", e.what());
    }

    std::ostringstream oss;
    for (const auto &pair : _data)
    {
        oss << pair.first << " = " << pair.second << std::endl;
    }
    return oss.str();
}

namespace Atom::Server
{
    SerializationEngine::SerializationEngine()
    {
        // 添加默认的渲染引擎
        auto jsonSerializationEngine = std::make_shared<JsonSerializationEngine>();
        m_serializeEngines["json"] = jsonSerializationEngine;
        m_currentSerializationEngine = "json"; // 默认选中 JsonSerializationEngine
    }

    // 添加渲染引擎
    void SerializationEngine::addSerializationEngine(const std::string &name, const std::shared_ptr<Serialization> &renderEngine)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_serializeEngines.find(name)!= m_serializeEngines.end())
        {
            LOG_F(ERROR, "SerializationEngine::addSerializationEngine: Render engine {} already exists!", name);
            return;
        }
        m_serializeEngines[name] = renderEngine;
    }

    // 设置当前选中的渲染引擎
    bool SerializationEngine::setCurrentSerializationEngine(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_serializeEngines.find(name);
        if (it != m_serializeEngines.end())
        {
            m_currentSerializationEngine = name;
            DLOG_F(INFO, "SerializationEngine::setCurrentSerializationEngine: Set current render engine: {}", name);
            return true;
        }
        LOG_F(ERROR, "SerializationEngine::setCurrentSerializationEngine: No such render engine: {}", name);
        return false;
    }
}

/*

int main()
{
    SerializationEngine engine;

    IPropertyBase property;
    property.device_name = "Device1";
    property.device_uuid = "12345678";
    property.message_uuid = "abcdefgh";
    property.name = "Property1";
    property.need_check = true;
    property.get_func = "get_property";
    property.set_func = "set_property";

    std::unordered_map<std::string, std::string> data;
    data["device_name"] = property.device_name;
    data["device_uuid"] = property.device_uuid;
    data["message_uuid"] = property.message_uuid;
    data["name"] = property.name;
    data["need_check"] = property.need_check ? "true" : "false";
    data["get_func"] = property.get_func;
    data["set_func"] = property.set_func;

    std::optional<std::string> _data = engine.serialize(data, true);

    if (_data)
    {
        std::cout << "Serialization _data: " << *_data << std::endl;
    }
    else
    {
        std::cout << "Serialization Failed." << std::endl;
    }

    engine.addSerializationEngine("xml", std::make_shared<YamlSerializationEngine>());
    engine.setCurrentSerializationEngine("xml");
    _data = engine.serialize(data, true);

    if (_data)
    {
        std::cout << "Serialization _data: " << *_data << std::endl;
    }
    else
    {
        std::cout << "Serialization Failed." << std::endl;
    }

    return 0;
}

*/