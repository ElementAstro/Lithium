/*
 * json_checker.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-11

Description: Json Checker interface implementation

**************************************************/

#include <regex>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include "atom/type/json.hpp"

using json = nlohmann::json;

/**
 * @brief Json Checker class.
 * @details This class is used to check if the JSON data matches the specified type.
 * @note The default rules are added by the constructor.
 */
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
    void addDefaultRule(const std::string &typeName, std::function<bool(const json &)> validator);

    /**
     * @brief Adds a custom rule for a specific type.
     * @tparam T The type of the JSON data.
     * @param typeName The name of the type.
     * @param validator The validation function for the type.
     */
    template <typename T>
    void addCustomRule(const std::string &typeName, std::function<bool(const json &)> validator);

    /**
     * @brief Checks if the JSON data matches the specified type.
     * @tparam T The type of the JSON data.
     * @param jsonData The JSON data to be checked.
     * @param typeName The name of the type to be checked.
     * @return True if the JSON data matches the specified type, false otherwise.
     */
    template <typename T>
    bool checkType(const json &jsonData, const std::string &typeName);

    /**
     * @brief Checks if the JSON data matches the expected value.
     * @tparam T The type of the JSON data.
     * @param jsonData The JSON data to be checked.
     * @param expectedValue The expected value.
     * @return True if the JSON data matches the expected value, false otherwise.
     */
    template <typename T>
    bool checkValue(const json &jsonData, const T &expectedValue);

    /**
     * @brief Validates the format of the JSON string data using a regular expression.
     * @param jsonData The JSON data to be validated.
     * @param format The regular expression format to be matched.
     * @return True if the JSON data format is valid, false otherwise.
     */
    template <typename T>
    bool validateFormat(const json &jsonData, const std::string &format);

    /**
     * @brief Sets the failure message.
     * @param message The failure message to be set.
     */
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
    bool checkTypeImpl(const json &jsonData, const std::string &typeName);

    /**
     * @brief Internal implementation to check if the JSON data matches the expected value.
     * @tparam T The type of the JSON data.
     * @param jsonData The JSON data to be checked.
     * @param expectedValue The expected value.
     * @return True if the JSON data matches the expected value, false otherwise.
     */
    template <typename T>
    bool checkValueImpl(const json &jsonData, const T &expectedValue);

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::function<bool(const json &)>> defaultRules_;
    emhash8::HashMap<std::string, std::function<bool(const json &)>> customRules_;
#else
    std::unordered_map<std::string, std::function<bool(const json &)>> defaultRules_;
    std::unordered_map<std::string, std::function<bool(const json &)>> customRules_;
#endif
    std::function<void(const std::string &)> failureCallback_;
};

template <typename T>
void JsonChecker::addDefaultRule(const std::string &typeName, std::function<bool(const json &)> validator)
{
    defaultRules_[typeName] = std::move(validator);
}

template <typename T>
void JsonChecker::addCustomRule(const std::string &typeName, std::function<bool(const json &)> validator)
{
    customRules_[typeName] = std::move(validator);
}

template <typename T>
bool JsonChecker::checkType(const json &jsonData, const std::string &typeName)
{
    if (!checkTypeImpl<T>(jsonData, typeName))
    {
        onFailure("Type mismatch");
        return false;
    }
    return true;
}

template <typename T>
bool JsonChecker::checkValue(const json &jsonData, const T &expectedValue)
{
    if (!checkValueImpl<T>(jsonData, expectedValue))
    {
        onFailure("Value mismatch");
        return false;
    }
    return true;
}

template <typename T>
bool JsonChecker::validateFormat(const json &jsonData, const std::string &format)
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

template <typename T>
bool JsonChecker::checkTypeImpl(const json &jsonData, const std::string &typeName)
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
