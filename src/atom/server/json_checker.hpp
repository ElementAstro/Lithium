/*
 * json_checker.hpp
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

Description: Json Checker interface implementation

**************************************************/

#include <regex>
#include <unordered_map>
#include "atom/type/json.hpp"

using json = nlohmann::json;

class JsonChecker
{
public:
    /**
     * @brief Default constructor for JsonChecker.
     */
    JsonChecker();

    /**
     * @brief Adds a default rule for a specific type.
     * @tparam T The type of the JSON data.
     * @param typeName The name of the type.
     * @param validator The validation function for the type.
     */
    template <typename T>
    void addDefaultRule(const std::string &typeName, std::function<bool(const json &)> validator)
    {
        defaultRules_[typeName] = std::move(validator);
    }

    /**
     * @brief Adds a custom rule for a specific type.
     * @tparam T The type of the JSON data.
     * @param typeName The name of the type.
     * @param validator The validation function for the type.
     */
    template <typename T>
    void addCustomRule(const std::string &typeName, std::function<bool(const json &)> validator)
    {
        customRules_[typeName] = std::move(validator);
    }

    /**
     * @brief Checks if the JSON data matches the specified type.
     * @tparam T The type of the JSON data.
     * @param jsonData The JSON data to be checked.
     * @param typeName The name of the type to be checked.
     * @return True if the JSON data matches the specified type, false otherwise.
     */
    template <typename T>
    bool checkType(const json &jsonData, const std::string &typeName)
    {
        if (!checkTypeImpl<T>(jsonData, typeName))
        {
            onFailure("Type mismatch");
            return false;
        }
        return true;
    }

    /**
     * @brief Checks if the JSON data matches the expected value.
     * @tparam T The type of the JSON data.
     * @param jsonData The JSON data to be checked.
     * @param expectedValue The expected value.
     * @return True if the JSON data matches the expected value, false otherwise.
     */
    template <typename T>
    bool checkValue(const json &jsonData, const T &expectedValue)
    {
        if (!checkValueImpl<T>(jsonData, expectedValue))
        {
            onFailure("Value mismatch");
            return false;
        }
        return true;
    }

    /**
     * @brief Validates the format of the JSON string data using a regular expression.
     * @param jsonData The JSON data to be validated.
     * @param format The regular expression format to be matched.
     * @return True if the JSON data format is valid, false otherwise.
     */
    template <typename T>
    bool validateFormat(const json &jsonData, const std::string &format)
    {
        if (!jsonData.is_string())
        {
            onFailure("JSON data is not a string");
            return false;
        }

        // 执行格式校验
        std::string jsonString = jsonData.get<std::string>();
        if (!std::regex_match(jsonString, std::regex(format)))
        {
            onFailure("Format validation failed");
            return false;
        }

        return true;
    }

    void onFailure(const std::string &message);

    /**
     * @brief Sets the failure callback function.
     * @param callback The failure callback function to be set.
     */
    void setFailureCallback(std::function<void(const std::string &)> callback);

private:
    /**
     * @brief Internal implementation to check if the JSON data matches the specified type.
     * @tparam T The type of the JSON data.
     * @param jsonData The JSON data to be checked.
     * @param typeName The name of the type to be checked.
     * @return True if the JSON data matches the specified type, false otherwise.
     */
    template <typename T>
    bool checkTypeImpl(const json &jsonData, const std::string &typeName)
    {
        auto it = customRules_.find(typeName);
        if (it != customRules_.end())
        {
            return it->second(jsonData);
        }

        it = defaultRules_.find(typeName);
        if (it != defaultRules_.end())
        {
            return it->second(jsonData);
        }

        onFailure("Unknown type: " + typeName);
        return false;
    }
    
    /**
     * @brief Internal implementation to check if the JSON data matches the expected value.
     * @tparam T The type of the JSON data.
     * @param jsonData The JSON data to be checked.
     * @param expectedValue The expected value.
     * @return True if the JSON data matches the expected value, false otherwise.
     */
    template <typename T>
    bool checkValueImpl(const json &jsonData, const T &expectedValue)
    {
        if (jsonData != expectedValue)
        {
            onFailure("Value mismatch");
            return false;
        }
        return true;
    }

private:
    std::unordered_map<std::string, std::function<bool(const json &)>> defaultRules_;
    std::unordered_map<std::string, std::function<bool(const json &)>> customRules_;
    std::function<void(const std::string &)> failureCallback_;
};
