/**
 * @file generator.cpp
 * @brief Task Generator
 *
 * This file contains the definition and implementation of a task generator.
 *
 * @date 2023-07-21
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#include "generator.hpp"

#include <algorithm>
#include <numeric>
#include <regex>
#include <string>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"

using json = nlohmann::json;
using namespace std::literals;

namespace lithium {

class TaskGenerator::Impl {
public:
    std::unordered_map<std::string, MacroValue> macros_;

    Impl();
    void addMacro(const std::string& name, MacroValue value);
    void processJson(json& j) const;
    void preprocessJsonMacros(json& j);
    void processJsonWithJsonMacros(json& j);
    auto evaluateMacro(const std::string& name,
                       const std::vector<std::string>& args) const
        -> std::string;
    auto replaceMacros(const std::string& input) const -> std::string;
};

TaskGenerator::TaskGenerator() : impl_(std::make_unique<Impl>()) {}
TaskGenerator::~TaskGenerator() = default;

auto TaskGenerator::createShared() -> std::shared_ptr<TaskGenerator> {
    return std::make_shared<TaskGenerator>();
}

void TaskGenerator::addMacro(const std::string& name, MacroValue value) {
    impl_->addMacro(name, std::move(value));
}

void TaskGenerator::processJson(json& j) const { impl_->processJson(j); }

void TaskGenerator::processJsonWithJsonMacros(json& j) {
    impl_->processJsonWithJsonMacros(j);
}

TaskGenerator::Impl::Impl() {
    addMacro("uppercase", [](const std::vector<std::string>& args) {
        if (args.empty()) {
            THROW_INVALID_ARGUMENT(
                "uppercase macro requires at least 1 argument");
        }
        std::string result = args[0];
        std::ranges::transform(result, result.begin(), ::toupper);
        return result;
    });
    addMacro("concat", [](const std::vector<std::string>& args) -> std::string {
        if (args.empty()) {
            return "";
        }

        std::string result = args[0];
        for (size_t i = 1; i < args.size(); ++i) {
            if (!args[i].empty()) {
                if (std::ispunct(args[i][0]) && args[i][0] != '(' &&
                    args[i][0] != '[') {
                    result += args[i];
                } else {
                    result += " " + args[i];
                }
            }
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
        if (args.empty()) {
            THROW_INVALID_ARGUMENT(
                "tolower macro requires at least 1 argument");
        }
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

void TaskGenerator::Impl::addMacro(const std::string& name, MacroValue value) {
    macros_[name] = std::move(value);
}

void TaskGenerator::Impl::processJson(json& j) const {
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

auto TaskGenerator::Impl::evaluateMacro(
    const std::string& name,
    const std::vector<std::string>& args) const -> std::string {
    if (auto it = macros_.find(name); it != macros_.end()) {
        if (std::holds_alternative<std::string>(it->second)) {
            return std::get<std::string>(it->second);
        }
        if (std::holds_alternative<
                std::function<std::string(const std::vector<std::string>&)>>(
                it->second)) {
            return std::get<
                std::function<std::string(const std::vector<std::string>&)>>(
                it->second)(args);
        }
    }
    THROW_INVALID_ARGUMENT("Undefined macro: " + name);
}

auto TaskGenerator::Impl::replaceMacros(const std::string& input) const
    -> std::string {
    static const std::regex MACRO_PATTERN(
        R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");
    std::string result = input;
    std::smatch match;

    while (std::regex_search(result, match, MACRO_PATTERN)) {
        std::string macroCall = match[1].str();
        auto pos = macroCall.find('(');

        // 如果没有找到括号，表示这是一个简单的宏替换而不是调用
        if (pos == std::string::npos) {
            auto it = macros_.find(macroCall);
            if (it != macros_.end()) {
                // 替换为宏的值
                if (std::holds_alternative<std::string>(it->second)) {
                    result.replace(match.position(0), match.length(0),
                                   std::get<std::string>(it->second));
                } else {
                    THROW_INVALID_ARGUMENT(
                        "Malformed macro or undefined macro: " + macroCall);
                }
            } else {
                THROW_INVALID_ARGUMENT("Undefined macro: " + macroCall);
            }
        } else {
            // 处理带参数的宏调用
            if (macroCall.back() != ')') {
                THROW_INVALID_ARGUMENT("Malformed macro: " + macroCall);
            }
            std::string macroName = macroCall.substr(0, pos);
            std::vector<std::string> args;

            std::string argsStr =
                macroCall.substr(pos + 1, macroCall.length() - pos - 2);
            static const std::regex ARG_PATTERN(R"(([^,]+))");
            std::sregex_token_iterator iter(argsStr.begin(), argsStr.end(),
                                            ARG_PATTERN);
            std::sregex_token_iterator end;
            for (; iter != end; ++iter) {
                args.push_back(atom::utils::trim(
                    replaceMacros(iter->str())));  // 递归处理嵌套宏
            }

            try {
                std::string replacement = evaluateMacro(macroName, args);
                result.replace(match.position(0), match.length(0), replacement);

                // 递归处理可能包含更多宏的结果字符串
                result = replaceMacros(result);
            } catch (const std::exception& e) {
                THROW_INVALID_ARGUMENT("Error processing macro: " + macroName +
                                       " - " + e.what());
            }
        }
    }

    return result;
}

void TaskGenerator::Impl::preprocessJsonMacros(json& j) {
    for (auto& [key, value] : j.items()) {
        if (value.is_string()) {
            std::string strValue = value.get<std::string>();
            std::smatch match;
            static const std::regex MACRO_PATTERN(
                R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");

            // Check if this is a macro definition
            if (std::regex_search(strValue, match, MACRO_PATTERN)) {
                std::string macroName = key;
                std::string macroBody = match[1].str();

                // Add to macros_ for later use
                macros_[macroName] = macroBody;

                // Optionally output for debugging
                LOG_F(INFO, "Preprocessed macro: {} -> {}", macroName, macroBody);
            }
        } else if (value.is_object() || value.is_array()) {
            preprocessJsonMacros(
                value);  // Recursive call for nested objects/arrays
        }
    }
}

void TaskGenerator::Impl::processJsonWithJsonMacros(json& j) {
    preprocessJsonMacros(j);  // First, preprocess macros to fill `macros_`
    processJson(j);           // Then, process JSON for macro replacements

    static const std::regex MACRO_PATTERN(
        R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})");
    for (auto& [key, value] : j.items()) {
        if (value.is_string()) {
            std::string strValue = value.get<std::string>();
            std::smatch match;
            if (std::regex_search(strValue, match, MACRO_PATTERN)) {
                std::string macroCall = match[1].str();
                auto pos = macroCall.find('(');
                if (pos == std::string::npos || macroCall.back() != ')') {
                    THROW_INVALID_ARGUMENT("Malformed macro: " + macroCall);
                }
                std::string macroName = macroCall.substr(0, pos);
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

                try {
                    json replacement = evaluateMacro(macroName, args);
                    value =
                        replacement;  // Replace the macro with evaluated result
                } catch (const std::exception& e) {
                    THROW_INVALID_ARGUMENT("Error in macro processing: " +
                                           std::string(e.what()));
                }
            }
        } else if (value.is_object() || value.is_array()) {
            processJsonWithJsonMacros(
                value);  // Recursively process nested objects/arrays
        }
    }
}

}  // namespace lithium
