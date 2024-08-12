/*
 * generator.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-21

Description: Task Generator

**************************************************/

#include "generator.hpp"
#include "task.hpp"

#include <algorithm>
#include <regex>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"

#include "atom/type/json.hpp"

using json = nlohmann::json;
using namespace std::literals;

namespace lithium {
TaskGenerator::TaskGenerator() {
    addMacro("uppercase", [](const std::vector<std::string>& args) {
        std::string result = args[0];
        std::ranges::transform(result, result.begin(), ::toupper);
        return result;
    });
    addMacro("concat", [](const std::vector<std::string>& args) {
        return std::accumulate(args.begin(), args.end(), std::string{});
    });
    addMacro("if", [](const std::vector<std::string>& args) {
        if (args.size() < 3) {
            THROW_INVALID_ARGUMENT("if macro requires 3 arguments");
        }
        return args[0] == "true" ? args[1] : args[2];
    });
    addMacro("length", [](const std::vector<std::string>& args) {
        if (args.size() != 1) {
            THROW_INVALID_ARGUMENT("length macro requires 1 argument");
        }
        return std::to_string(args[0].length());
    });
    addMacro("equals", [](const std::vector<std::string>& args) {
        if (args.size() != 2) {
            THROW_INVALID_ARGUMENT("equals macro requires 2 arguments");
        }
        return args[0] == args[1] ? "true" : "false";
    });
    addMacro("tolower", [](const std::vector<std::string>& args) {
        std::string result = args[0];
        std::ranges::transform(result, result.begin(), ::tolower);
        return result;
    });
    addMacro("repeat", [](const std::vector<std::string>& args) {
        if (args.size() != 2) {
            THROW_INVALID_ARGUMENT("repeat macro requires 2 arguments");
        }
        std::string result;
        int times = std::stoi(args[1]);
        for (int i = 0; i < times; ++i) {
            result += args[0];
        }
        return result;
    });
}

auto TaskGenerator::createShared() -> std::shared_ptr<TaskGenerator> {
    return std::make_shared<TaskGenerator>();
}

void TaskGenerator::addMacro(const std::string& name, MacroValue value) {
    macros_[name] = std::move(value);
}

void TaskGenerator::processJson(json& j) const {
    for (const auto& [key, value] : j.items()) {
        if (value.is_string()) {
            std::string newValue = replaceMacros(value.get<std::string>());
            while (newValue != value.get<std::string>()) {
                value = newValue;
                newValue = replaceMacros(value);
            }
            value = newValue;
        } else if (value.is_object() || value.is_array()) {
            processJson(value);
        }
    }
}

auto TaskGenerator::evaluateMacro(const std::string& name,
                                  const std::vector<std::string>& args) const
    -> std::string {
    if (auto it = macros_.find(name); it != macros_.end()) {
        if (std::holds_alternative<std::string>(it->second)) {
            return std::get<std::string>(it->second);
        }
        if (std::holds_alternative<json>(it->second)) {
            return std::get<json>(it->second).dump();
        }
        if (std::holds_alternative<
                std::function<json(const std::vector<std::string>&)>>(
                it->second)) {
            return std::get<
                       std::function<json(const std::vector<std::string>&)>>(
                       it->second)(args)
                .dump();
        }
    }
    THROW_INVALID_ARGUMENT("Undefined macro: " + name);
}

auto TaskGenerator::replaceMacros(const std::string& input) const
    -> std::string {
    static const std::regex MACRO_PATTERN(
        R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");
    std::string result = input;
    std::smatch match;

    while (std::regex_search(result, match, MACRO_PATTERN)) {
        std::string macroCall = match[1].str();
        auto pos = macroCall.find('(');
        std::string macroName =
            (pos == std::string::npos) ? macroCall : macroCall.substr(0, pos);
        std::vector<std::string> args;

        if (pos != std::string::npos) {
            std::string argsStr =
                macroCall.substr(pos + 1, macroCall.length() - pos - 2);
            static const std::regex ARG_PATTERN(R"(([^,]+))");
            std::sregex_token_iterator iter(argsStr.begin(), argsStr.end(),
                                            ARG_PATTERN);
            std::sregex_token_iterator end;
            for (; iter != end; ++iter) {
                args.push_back(atom::utils::trim(replaceMacros(iter->str())));
            }
        }

        std::string replacement = evaluateMacro(macroName, args);
        result.replace(match.position(0), match.length(0), replacement);
    }

    return result;
}

void TaskGenerator::processJsonWithJsonMacros(json& j) {
    for (const auto& [key, value] : j.items()) {
        if (value.is_string()) {
            std::string newValue = replaceMacros(value.get<std::string>());
            while (newValue != value.get<std::string>()) {
                value = newValue;
                newValue = replaceMacros(value);
            }
            value = newValue;
        } else if (value.is_object() || value.is_array()) {
            processJsonWithJsonMacros(value);
        }
    }

    static const std::regex MACRO_PATTERN(
        R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");
    for (const auto& [key, value] : j.items()) {
        if (value.is_string()) {
            std::string strValue = value.get<std::string>();
            std::smatch match;
            if (std::regex_search(strValue, match, MACRO_PATTERN)) {
                std::string macroCall = match[1].str();
                auto pos = macroCall.find('(');
                std::string macroName = (pos == std::string::npos)
                                            ? macroCall
                                            : macroCall.substr(0, pos);
                std::vector<std::string> args;

                if (pos != std::string::npos) {
                    std::string argsStr =
                        macroCall.substr(pos + 1, macroCall.length() - pos - 2);
                    static const std::regex ARG_PATTERN(R"(([^,]+))");
                    std::sregex_token_iterator iter(argsStr.begin(),
                                                    argsStr.end(), ARG_PATTERN);
                    std::sregex_token_iterator end;
                    for (; iter != end; ++iter) {
                        args.push_back(
                            atom::utils::trim(replaceMacros(iter->str())));
                    }
                }

                json replacement = evaluateMacro(macroName, args);
                value = replacement;
            }
        } else if (value.is_object() || value.is_array()) {
            processJsonWithJsonMacros(value);
        }
    }
}
}  // namespace lithium
