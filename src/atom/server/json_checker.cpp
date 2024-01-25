/*
 * json_checker.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-11

Description: Json Checker interface implementation

**************************************************/

#include "json_checker.hpp"

JsonChecker::JsonChecker()
{
    // 初始化默认规则
    addDefaultRule<std::string>("string", [](const json &jsonData)
                                { return jsonData.is_string(); });

    addDefaultRule<int>("integer", [](const json &jsonData)
                        { return jsonData.is_number_integer(); });

    addDefaultRule<double>("number", [](const json &jsonData)
                           { return jsonData.is_number(); });

    addDefaultRule<bool>("boolean", [](const json &jsonData)
                         { return jsonData.is_boolean(); });
}

void JsonChecker::onFailure(const std::string &message)
{
    if (failureCallback_)
    {
        failureCallback_(message);
    }
}

void JsonChecker::setFailureCallback(std::function<void(const std::string &)> callback)
{
    failureCallback_ = std::move(callback);
}

/*

int main()
{
    json templateData = R"(
        {
            "name": "string",
            "age": 0,
            "isStudent": false,
            "email": "string"
        }
    )"_json;

    json jsonData = R"(
        {
            "name": "Alice",
            "age": 25,
            "isStudent": true,
            "email": "alice@example.com"
        }
    )"_json;

    JsonChecker checker;
    checker.setFailureCallback([](const std::string &message)
                               { std::cerr << "JSON 数据与模板不匹配: " << message << std::endl; });

    // 添加自定义规则
    checker.addCustomRule<bool>("boolean", [](const json &jsonData)
                                { return jsonData.is_boolean() || jsonData.is_string(); });

    bool isMatched = true;
    isMatched &= checker.checkType<std::string>(jsonData["name"], templateData["name"]);
    isMatched &= checker.checkType<int>(jsonData["age"], templateData["age"]);
    isMatched &= checker.checkType<bool>(jsonData["isStudent"], templateData["isStudent"]);
    isMatched &= checker.validateFormat<std::string>(jsonData["email"], R"(\w+@\w+\.\w+)");

    if (isMatched)
    {
        std::cout << "JSON 数据与模板匹配！" << std::endl;
    }

    return 0;
}

*/