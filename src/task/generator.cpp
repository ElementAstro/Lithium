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

#include <regex>

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
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    });
    addMacro("concat", [](const std::vector<std::string>& args) {
        std::string result;
        for (const auto& arg : args) {
            result += arg;
        }
        return result;
    });
    addMacro("if", [](const std::vector<std::string>& args) {
        if (args.size() < 3) {
            THROW_INVALID_ARGUMENT("if macro requires 3 arguments");
        }
        return args[0] == "true" ? args[1] : args[2];
    });
    addMacro("length", [](const std::vector<std::string>& args) {
        if (args.size() != 1) {
            THROW_MISSING_ARGUMENT("length macro requires 1 argument");
        }
        return std::to_string(args[0].length());
    });
    addMacro("equals", [](const std::vector<std::string>& args) {
        if (args.size() != 2) {
            THROW_MISSING_ARGUMENT("equals macro requires 2 arguments");
        }
        return args[0] == args[1] ? "true" : "false";
    });
    addMacro("tolower", [](const std::vector<std::string>& args) {
        std::string result = args[0];
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    });
    addMacro("repeat", [](const std::vector<std::string>& args) {
        if (args.size() != 2) {
            THROW_MISSING_ARGUMENT("repeat macro requires 2 arguments");
        }
        std::string result;
        int times = std::stoi(args[1]);
        for (int i = 0; i < times; ++i) {
            result += args[0];
        }
        return result;
    });
}

// Add a new macro
void TaskGenerator::addMacro(const std::string& name, const MacroValue& value) {
    macros_[name] = value;
}

// Process a JSON object, replacing macros in all string fields
void TaskGenerator::processJson(json& j) const {
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it->is_string()) {
            std::string value = it->get<std::string>();
            std::string newValue = replaceMacros(value);
            while (newValue != value) {
                value = newValue;
                newValue = replaceMacros(value);
            }
            it.value() = newValue;
        } else if (it->is_object() || it->is_array()) {
            processJson(*it);
        } else if (it->is_string()) {
            it.value() = replaceMacros(it.value().get<std::string>());
        }
    }
}

// Evaluate a macro
auto TaskGenerator::evaluateMacro(const std::string& name,
                                  const std::vector<std::string>& args) const
    -> std::string {
    auto it = macros_.find(name);
    if (it != macros_.end()) {
        if (std::holds_alternative<std::string>(it->second)) {
            return std::get<std::string>(it->second);
        }
        if (std::holds_alternative<json>(it->second)) {
            return std::get<json>(it->second);
        }
        if (std::holds_alternative<
                std::function<json(const std::vector<std::string>&)>>(
                it->second)) {
            return std::get<
                std::function<json(const std::vector<std::string>&)>>(
                it->second)(args);
        }
    }
    THROW_WRONG_ARGUMENT("Undefined macro: " + name);
}

// Replace macros in a string
auto TaskGenerator::replaceMacros(const std::string& input) const
    -> std::string {
    std::regex macroPattern(R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");
    std::string result = input;
    std::smatch match;

    while (std::regex_search(result, match, macroPattern)) {
        std::string macroCall = match[1].str();
        auto pos = macroCall.find('(');
        std::string macroName =
            pos == std::string::npos ? macroCall : macroCall.substr(0, pos);
        std::vector<std::string> args;

        if (pos != std::string::npos) {
            std::string argsStr =
                macroCall.substr(pos + 1, macroCall.length() - pos - 2);
            std::regex argPattern(R"(([^,]+))");
            std::sregex_token_iterator iter(argsStr.begin(), argsStr.end(),
                                            argPattern);
            std::sregex_token_iterator end;
            for (; iter != end; ++iter) {
                args.push_back(atom::utils::trim(replaceMacros(
                    iter->str())));  // Recursively replace macros in arguments
            }
        }

        json replacement = evaluateMacro(macroName, args);
        if (replacement.is_string()) {
            result.replace(match.position(0), match.length(0),
                           replacement.get<std::string>());
        } else if (replacement.is_object()) {
            result.replace(match.position(0), match.length(0),
                           replacement.dump(4));
        } else {
            THROW_INVALID_ARGUMENT(
                "Macro replacement must be a string within a string context");
        }
    }

    return result;
}

void TaskGenerator::processJsonWithJsonMacros(json& j) {
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it->is_string()) {
            std::string value = it->get<std::string>();
            std::string newValue = replaceMacros(value);
            while (newValue != value) {
                value = newValue;
                newValue = replaceMacros(value);
            }
            it.value() = newValue;
        } else if (it->is_object() || it->is_array()) {
            processJsonWithJsonMacros(*it);
        } else if (it->is_string()) {
            it.value() = replaceMacros(it.value().get<std::string>());
        }
    }

    // Handle JSON macros
    std::regex macroPattern(R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it->is_string()) {
            std::string value = it->get<std::string>();
            std::smatch match;
            if (std::regex_search(value, match, macroPattern)) {
                std::string macroCall = match[1].str();
                auto pos = macroCall.find('(');
                std::string macroName = pos == std::string::npos
                                            ? macroCall
                                            : macroCall.substr(0, pos);
                std::vector<std::string> args;

                if (pos != std::string::npos) {
                    std::string argsStr =
                        macroCall.substr(pos + 1, macroCall.length() - pos - 2);
                    std::regex argPattern(R"(([^,]+))");
                    std::sregex_token_iterator iter(argsStr.begin(),
                                                    argsStr.end(), argPattern);
                    std::sregex_token_iterator end;
                    for (; iter != end; ++iter) {
                        args.push_back(
                            atom::utils::trim(replaceMacros(iter->str())));
                    }
                }

                json replacement = evaluateMacro(macroName, args);
                *it = replacement;
            }
        } else if (it->is_object() || it->is_array()) {
            processJsonWithJsonMacros(*it);
        }
    }
}
}  // namespace lithium
