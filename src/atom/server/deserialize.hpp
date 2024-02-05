/*
 * deserialize.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

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

class JsonDeserializer : public DeserializeEngine
{
public:
    std::optional<std::any> deserialize(const std::string &data) const override;
};

namespace Atom::Server
{
    /**
     * @class DeserializationEngine
     * @brief A class responsible for deserializing data using different deserialization engines.
     */
    class DeserializationEngine
    {
    public:
        /**
         * @brief Default constructor.
         */
        DeserializationEngine() = default;

        /**
         * @brief Default destructor.
         */
        ~DeserializationEngine() = default;

        /**
         * @brief Adds a deserialization engine to the engine map.
         * @param name The name of the deserialization engine.
         * @param engine A shared pointer to the deserialization engine.
         */
        void addDeserializeEngine(const std::string &name, const std::shared_ptr<DeserializeEngine> &engine);

        /**
         * @brief Sets the current deserialization engine.
         * @param name The name of the deserialization engine to set as current.
         * @return True if the deserialization engine was set successfully, false otherwise.
         */
        bool setCurrentDeserializeEngine(const std::string &name);

        /**
         * @brief Deserializes the given data using the current deserialization engine.
         * @tparam T The type to deserialize the data into.
         * @param data The serialized data to deserialize.
         * @return An optional containing the deserialized object if successful, or an empty optional otherwise.
         */
        template <typename T>
        std::optional<T> deserialize(const std::string &data) const;

    private:
#if ENABLE_FASTHASH
        emhash8::HashMap<std::string, std::shared_ptr<DeserializeEngine>> deserializationEngines_; /**< Map of deserialization engines. */
#else
        std::unordered_map<std::string, std::shared_ptr<DeserializeEngine>> deserializationEngines_; /**< Map of deserialization engines. */
#endif
        std::string currentDeserializationEngine_; /**< Name of the current deserialization engine. */
        mutable std::mutex mutex_;                 /**< Mutex to ensure thread safety. */
    };

    template <typename T>
    std::optional<T> DeserializationEngine::deserialize(const std::string &data) const
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
}

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
