/*
 * deserialize.hpp
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

#pragma once

#include <any>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

class DeserializeEngine
{
public:
    virtual ~DeserializeEngine() = default;
    virtual std::optional<std::any> deserialize(const std::string &data) const = 0;
};

class DeserializationEngine
{
public:
    DeserializationEngine() = default;
    ~DeserializationEngine() = default;

    void addDeserializeEngine(const std::string &name, const std::shared_ptr<DeserializeEngine> &DeserializeEngine);

    bool setCurrentDeserializeEngine(const std::string &name);

    template <typename T>
    std::optional<T> deserialize(const std::string &data) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = deserializationEngines_.find(currentDeserializationEngine_);
        if (it != deserializationEngines_.end())
        {
            try
            {
                auto result = it->second->deserialize(data);
                if (result.has_value())
                {
                    return std::any_cast<T>(result.value());
                }
            }
            catch (const std::bad_any_cast &)
            {
                return std::nullopt;
            }
        }

        return std::nullopt;
    }

private:
    std::unordered_map<std::string, std::shared_ptr<DeserializeEngine>> deserializationEngines_;
    std::string currentDeserializationEngine_;
    mutable std::mutex mutex_;
};

class JsonDeserializer : public DeserializeEngine
{
public:
    std::optional<std::any> deserialize(const std::string &data) const override;
};

/*

class YamlDeserializer : public DeserializeEngine
{
public:
    std::optional<std::any> deserialize(const std::string &data) const override
    {
        try
        {
            YAML::Node yamlData = YAML::Load(data);
            if (yamlData.IsMap())
            {
                std::map<std::string, std::string> result;
                for (auto it = yamlData.begin(); it != yamlData.end(); ++it)
                {
                    result[it->first.as<std::string>()] = it->second.as<std::string>();
                }
                return result;
            }
            else
            {
                return std::nullopt;
            }
        }
        catch (const YAML::ParserException &)
        {
            return std::nullopt;
        }
    }
};


int main()
{
    DeserializationEngine deserializationEngine;
    deserializationEngine.addDeserializeEngine("json", std::make_shared<JsonDeserializer>());
    deserializationEngine.addDeserializeEngine("yaml", std::make_shared<YamlDeserializer>());
    deserializationEngine.setCurrentDeserializeEngine("json");

    std::string jsonData = R"({"name": "John", "age": "30", "city": "New York"})";
    std::string yamlData = R"(name: "John"
age: "30"
city: "New York")";

    // JSON 反序列化
    auto deserializedJsonData = deserializationEngine.deserialize<std::map<std::string, std::string>>(jsonData);
    if (deserializedJsonData.has_value())
    {
        std::cout << "Deserialized JSON data:" << std::endl;
        for (const auto &entry : deserializedJsonData.value())
        {
            std::cout << entry.first << ": " << std::any_cast<std::string>(entry.second) << std::endl;
        }
    }
    else
    {
        std::cout << "Failed to deserialize JSON data." << std::endl;
    }

    // YAML 反序列化
    deserializationEngine.setCurrentDeserializeEngine("yaml");
    auto deserializedYamlData = deserializationEngine.deserialize<std::map<std::string, std::string>>(yamlData);
    if (deserializedYamlData.has_value())
    {
        std::cout << "Deserialized YAML data:" << std::endl;
        for (const auto &entry : deserializedYamlData.value())
        {
            std::cout << entry.first << ": " << std::any_cast<std::string>(entry.second) << std::endl;
        }
    }
    else
    {
        std::cout << "Failed to deserialize YAML data." << std::endl;
    }

    return 0;
}

*/