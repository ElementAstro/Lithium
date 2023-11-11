/*
 * deserialize.cpp
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

Date: 2023-11-11

Description: This file contains the declaration of the DeserializationEngine class

**************************************************/

#include "deserialize.hpp"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

std::optional<std::any> JsonDeserializer::deserialize(const std::string &data) const override
{
    try
    {
        nlohmann::json jsonData = nlohmann::json::parse(data);
        return jsonData.get<std::map<std::string, std::string>>();
    }
    catch (const nlohmann::json::parse_error &)
    {
        return std::nullopt;
    }
}

void DeserializationEngine::addDeserializeEngine(const std::string &name, const std::shared_ptr<DeserializeEngine> &DeserializeEngine)
{
    deserializationEngines_[name] = DeserializeEngine;
}

bool DeserializationEngine::setCurrentDeserializeEngine(const std::string &name)
{
    auto it = deserializationEngines_.find(name);
    if (it != deserializationEngines_.end())
    {
        currentDeserializationEngine_ = name;
        return true;
    }
    return false;
}