/*
 * serialize.cpp
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

#include "loguru/loguru.hpp"

std::string JsonRenderEngine::render(const std::any &data, bool format) const
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

std::string XmlRenderEngine::render(const std::any &data, bool format) const
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

std::string YamlRenderEngine::render(const std::any &data, bool format) const
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

std::string IniRenderEngine::render(const std::any &data, bool format) const
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
std::string TomlRenderEngine::render(const std::any &data, bool format) const
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

    std::string result;

    if (format)
    {
        for (const auto &pair : _data)
        {
            result += fmt::format("{} = \"{}\"\n", pair.first, pair.second);
        }
    }
    else
    {
        for (const auto &pair : _data)
        {
            result += fmt::format("{}=\"{}\"\n", pair.first, pair.second);
        }
    }

    return result;
}

SerializationEngine::SerializationEngine()
{
    // 添加默认的渲染引擎
    auto jsonRenderEngine = std::make_shared<JsonRenderEngine>();
    m_renderEngines["json"] = jsonRenderEngine;
    m_currentRenderEngine = "json"; // 默认选中 JsonRenderEngine
}

// 添加渲染引擎
void SerializationEngine::addRenderEngine(const std::string &name, const std::shared_ptr<RenderEngine> &renderEngine)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_renderEngines[name] = renderEngine;
}

// 设置当前选中的渲染引擎
bool SerializationEngine::setCurrentRenderEngine(const std::string &name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_renderEngines.find(name);
    if (it != m_renderEngines.end())
    {
        m_currentRenderEngine = name;
        return true;
    }
    return false;
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

    std::optional<std::string> result = engine.serialize(data, true);

    if (result)
    {
        std::cout << "Serialization Result: " << *result << std::endl;
    }
    else
    {
        std::cout << "Serialization Failed." << std::endl;
    }

    engine.addRenderEngine("xml", std::make_shared<YamlRenderEngine>());
    engine.setCurrentRenderEngine("xml");
    result = engine.serialize(data, true);

    if (result)
    {
        std::cout << "Serialization Result: " << *result << std::endl;
    }
    else
    {
        std::cout << "Serialization Failed." << std::endl;
    }

    return 0;
}

*/