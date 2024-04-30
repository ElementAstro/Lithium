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

namespace atom::server {
JsonChecker::JsonChecker() {
    // 初始化默认规则
    addDefaultRule<std::string>(
        "string", [](const json &jsonData) { return jsonData.is_string(); });

    addDefaultRule<int>("integer", [](const json &jsonData) {
        return jsonData.is_number_integer();
    });

    addDefaultRule<double>(
        "number", [](const json &jsonData) { return jsonData.is_number(); });

    addDefaultRule<bool>(
        "boolean", [](const json &jsonData) { return jsonData.is_boolean(); });
}

void JsonChecker::onFailure(const std::string &message) {
    if (failureCallback_) {
        failureCallback_(message);
    }
}

void JsonChecker::setFailureCallback(
    std::function<void(const std::string &)> callback) {
    failureCallback_ = std::move(callback);
}

}  // namespace atom::server
