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

#include "atom/type/iparams.hpp"
#include "atom/type/json.hpp"
#include "atom/log/loguru.hpp"

using json = nlohmann::json;

namespace Atom::Server
{
    std::optional<std::any> JsonDeserializer::deserialize(const std::string &data) const
    {
        DLOG_F(INFO, "JsonDeserializer::deserialize called with {}", data);
        try
        {
            json jsonData = json::parse(data);
            DLOG_F(INFO, "JsonDeserializer::deserialize: Successfully deserialized json data.");
            return jsonData.get<std::map<std::string, std::string>>();
        }
        catch (const json::parse_error &)
        {
            LOG_F(ERROR, "JsonDeserializer::deserialize: Failed to deserialize json data.");
            return std::nullopt;
        }
    }

    std::optional<std::any> JsonParamsDeserializer::deserialize(const std::string &data) const
    {
        DLOG_F(INFO, "JsonParamsDeserializer::deserialize called with {}", data);
        try
        {
            std::shared_ptr<IParams> params;
            if (!params->fromJson(data))
            {
                LOG_F(ERROR, "JsonParamsDeserializer::deserialize: Failed to deserialize json data.");
                return std::nullopt;
            }
            DLOG_F(INFO, "JsonParamsDeserializer::deserialize: Successfully deserialized json data.");
            return params;
        }
        catch (const std::exception &)
        {
            LOG_F(ERROR, "JsonParamsDeserializer::deserialize: Failed to deserialize json data.");
            return std::nullopt;
        }
    }

    void DeserializationEngine::addDeserializeEngine(const std::string &name, const std::shared_ptr<DeserializeEngine> &DeserializeEngine)
    {
        DLOG_F(INFO, "DeserializationEngine::addDeserializeEngine called with {}", name);
        if (deserializationEngines_.find(name)!= deserializationEngines_.end())
        {
            LOG_F(ERROR, "DeserializationEngine::addDeserializeEngine: Deserialize engine already exists: {}", name);
            return;
        }
        deserializationEngines_[name] = DeserializeEngine;
        LOG_F(INFO, "DeserializationEngine::addDeserializeEngine: Add deserialize engine: {}", name);
    }

    bool DeserializationEngine::setCurrentDeserializeEngine(const std::string &name)
    {
        DLOG_F(INFO, "DeserializationEngine::setCurrentDeserializeEngine called with {}", name);
        auto it = deserializationEngines_.find(name);
        if (it != deserializationEngines_.end())
        {
            currentDeserializationEngine_ = name;
            LOG_F(INFO, "DeserializationEngine::setCurrentDeserializeEngine: Set current deserialize engine to: {}", name);
            return true;
        }
        LOG_F(ERROR, "DeserializationEngine::setCurrentDeserializeEngine: No such deserialize engine: {}", name);
        return false;
    }
}