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

#include <fstream>
#include <future>
#include <regex>

#include "atom/error/exception.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"
#include "atom/utils/to_string.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;
using namespace std::literals;

namespace lithium {
TaskGenerator::TaskGenerator() {
    add_macro("uppercase", [](const std::vector<std::string>& args) {
        std::string result = args[0];
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    });
    add_macro("concat", [](const std::vector<std::string>& args) {
        std::string result;
        for (const auto& arg : args) {
            result += arg;
        }
        return result;
    });
    add_macro("if", [](const std::vector<std::string>& args) {
        if (args.size() < 3)
            THROW_INVALID_ARGUMENT("if macro requires 3 arguments");
        return args[0] == "true" ? args[1] : args[2];
    });
    add_macro("length", [](const std::vector<std::string>& args) {
        if (args.size() != 1)
            throw std::runtime_error("length macro requires 1 argument");
        return std::to_string(args[0].length());
    });
    add_macro("equals", [](const std::vector<std::string>& args) {
        if (args.size() != 2)
            throw std::runtime_error("equals macro requires 2 arguments");
        return args[0] == args[1] ? "true" : "false";
    });
    add_macro("tolower", [](const std::vector<std::string>& args) {
        std::string result = args[0];
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    });
    add_macro("repeat", [](const std::vector<std::string>& args) {
        if (args.size() != 2)
            throw std::runtime_error("repeat macro requires 2 arguments");
        std::string result;
        int times = std::stoi(args[1]);
        for (int i = 0; i < times; ++i) {
            result += args[0];
        }
        return result;
    });
}

// Add a new macro
void TaskGenerator::add_macro(const std::string& name,
                              const MacroValue& value) {
    macros[name] = value;
}

// Process a JSON object, replacing macros in all string fields
void TaskGenerator::process_json(json& j) const {
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it->is_string()) {
            std::string value = it->get<std::string>();
            std::string new_value = replace_macros(value);
            while (new_value != value) {
                value = new_value;
                new_value = replace_macros(value);
            }
            it.value() = new_value;
        } else if (it->is_object() || it->is_array()) {
            process_json(*it);
        } else if (it->is_string()) {
            it.value() = replace_macros(it.value().get<std::string>());
        }
    }
}

// Evaluate a macro
std::string TaskGenerator::evaluate_macro(
    const std::string& name, const std::vector<std::string>& args) const {
    auto it = macros.find(name);
    if (it != macros.end()) {
        if (std::holds_alternative<std::string>(it->second)) {
            return std::get<std::string>(it->second);
        } else if (std::holds_alternative<json>(it->second)) {
            return std::get<json>(it->second);
        } else if (std::holds_alternative<
                       std::function<json(const std::vector<std::string>&)>>(
                       it->second)) {
            return std::get<
                std::function<json(const std::vector<std::string>&)>>(
                it->second)(args);
        }
    }
    throw std::runtime_error("Undefined macro: " + name);
}

// Replace macros in a string
std::string TaskGenerator::replace_macros(const std::string& input) const {
    std::regex macro_pattern(R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");
    std::string result = input;
    std::smatch match;

    while (std::regex_search(result, match, macro_pattern)) {
        std::string macro_call = match[1].str();
        auto pos = macro_call.find('(');
        std::string macro_name =
            pos == std::string::npos ? macro_call : macro_call.substr(0, pos);
        std::vector<std::string> args;

        if (pos != std::string::npos) {
            std::string args_str =
                macro_call.substr(pos + 1, macro_call.length() - pos - 2);
            std::regex arg_pattern(R"(([^,]+))");
            std::sregex_token_iterator iter(args_str.begin(), args_str.end(),
                                            arg_pattern);
            std::sregex_token_iterator end;
            for (; iter != end; ++iter) {
                args.push_back(atom::utils::trim(replace_macros(
                    iter->str())));  // Recursively replace macros in arguments
            }
        }

        json replacement = evaluate_macro(macro_name, args);
        if (replacement.is_string()) {
            result.replace(match.position(0), match.length(0),
                           replacement.get<std::string>());
        } else if (replacement.is_object()) {
            result.replace(match.position(0), match.length(0),
                           replacement.dump(4));
        } else {
            throw std::runtime_error(
                "Macro replacement must be a string within a string context");
        }
    }

    return result;
}

void TaskGenerator::process_json_with_json_macros(json& j) {
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it->is_string()) {
            std::string value = it->get<std::string>();
            std::string new_value = replace_macros(value);
            while (new_value != value) {
                value = new_value;
                new_value = replace_macros(value);
            }
            it.value() = new_value;
        } else if (it->is_object() || it->is_array()) {
            process_json_with_json_macros(*it);
        } else if (it->is_string()) {
            it.value() = replace_macros(it.value().get<std::string>());
        }
    }

    // Handle JSON macros
    std::regex macro_pattern(R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");
    for (auto it = j.begin(); it != j.end(); ++it) {
        if (it->is_string()) {
            std::string value = it->get<std::string>();
            std::smatch match;
            if (std::regex_search(value, match, macro_pattern)) {
                std::string macro_call = match[1].str();
                auto pos = macro_call.find('(');
                std::string macro_name = pos == std::string::npos
                                             ? macro_call
                                             : macro_call.substr(0, pos);
                std::vector<std::string> args;

                if (pos != std::string::npos) {
                    std::string args_str = macro_call.substr(
                        pos + 1, macro_call.length() - pos - 2);
                    std::regex arg_pattern(R"(([^,]+))");
                    std::sregex_token_iterator iter(
                        args_str.begin(), args_str.end(), arg_pattern);
                    std::sregex_token_iterator end;
                    for (; iter != end; ++iter) {
                        args.push_back(
                            atom::utils::trim(replace_macros(iter->str())));
                    }
                }

                json replacement = evaluate_macro(macro_name, args);
                *it = replacement;
            }
        } else if (it->is_object() || it->is_array()) {
            process_json_with_json_macros(*it);
        }
    }
}
}  // namespace lithium
