/**
 * @file generator.cpp
 * @brief Task Generator
 *
 * This file contains the definition and implementation of a task generator.
 *
 * @date 2023-07-21
 * @modified 2024-04-27
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#include "generator.hpp"

#include <algorithm>
#include <cctype>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#ifdef LITHIUM_USE_BOOST_REGEX
#include <boost/regex.hpp>
#else
#include <regex>
#endif

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"

using json = nlohmann::json;
using namespace std::literals;

namespace lithium {

#ifdef LITHIUM_USE_BOOST_REGEX
using Regex = boost::regex;
using Match = boost::smatch;
#else
using Regex = std::regex;
using Match = std::smatch;
#endif

class TaskGenerator::Impl {
public:
    Impl();
    void addMacro(const std::string& name, MacroValue value);
    void removeMacro(const std::string& name);
    auto listMacros() const -> std::vector<std::string>;
    void processJson(json& json_obj) const;
    void processJsonWithJsonMacros(json& json_obj);

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, MacroValue> macros_;

    // Cache for macro replacements
    mutable std::unordered_map<std::string, std::string> macro_cache_;
    mutable std::shared_mutex cache_mutex_;

    // Precompiled regex patterns
    static const Regex MACRO_PATTERN;
    static const Regex ARG_PATTERN;

    // Helper methods
    auto replaceMacros(const std::string& input) const -> std::string;
    auto evaluateMacro(const std::string& name,
                       const std::vector<std::string>& args) const
        -> std::string;
    void preprocessJsonMacros(json& json_obj);
};

const Regex TaskGenerator::Impl::MACRO_PATTERN(
    R"(\$\{([^\{\}]+(?:\([^\{\}]*\))*)\})", Regex::optimize);
const Regex TaskGenerator::Impl::ARG_PATTERN(R"(([^,]+))", Regex::optimize);

TaskGenerator::Impl::Impl() {
    // Initialize default macros
    addMacro("uppercase",
             [](const std::vector<std::string>& args) -> std::string {
                 if (args.empty()) {
                     throw TaskGeneratorException(
                         "uppercase macro requires at least 1 argument");
                 }
                 std::string result = args[0];
                 std::transform(result.begin(), result.end(), result.begin(),
                                ::toupper);
                 return result;
             });
    addMacro("concat", [](const std::vector<std::string>& args) -> std::string {
        if (args.empty()) {
            return "";
        }

        std::ostringstream oss;
        oss << args[0];
        for (size_t i = 1; i < args.size(); ++i) {
            if (!args[i].empty()) {
                if ((std::ispunct(args[i][0]) != 0) && args[i][0] != '(' &&
                    args[i][0] != '[') {
                    oss << args[i];
                } else {
                    oss << " " << args[i];
                }
            }
        }
        return oss.str();
    });

    addMacro("if", [](const std::vector<std::string>& args) -> std::string {
        if (args.size() < 3) {
            throw TaskGeneratorException("if macro requires 3 arguments");
        }
        return args[0] == "true" ? args[1] : args[2];
    });
    addMacro("length", [](const std::vector<std::string>& args) -> std::string {
        if (args.size() != 1) {
            throw TaskGeneratorException("length macro requires 1 argument");
        }
        return std::to_string(args[0].length());
    });
    addMacro("equals", [](const std::vector<std::string>& args) -> std::string {
        if (args.size() != 2) {
            throw TaskGeneratorException("equals macro requires 2 arguments");
        }
        return args[0] == args[1] ? "true" : "false";
    });
    addMacro("tolower",
             [](const std::vector<std::string>& args) -> std::string {
                 if (args.empty()) {
                     throw TaskGeneratorException(
                         "tolower macro requires at least 1 argument");
                 }
                 std::string result = args[0];
                 std::transform(result.begin(), result.end(), result.begin(),
                                ::tolower);
                 return result;
             });
    addMacro("repeat", [](const std::vector<std::string>& args) -> std::string {
        if (args.size() != 2) {
            throw TaskGeneratorException("repeat macro requires 2 arguments");
        }
        std::string result;
        try {
            int times = std::stoi(args[1]);
            if (times < 0) {
                throw std::invalid_argument("Negative repeat count");
            }
            result.reserve(args[0].size() * times);
            for (int i = 0; i < times; ++i) {
                result += args[0];
            }
        } catch (const std::exception& e) {
            throw TaskGeneratorException(std::string("Invalid repeat count: ") +
                                         e.what());
        }
        return result;
    });
}

void TaskGenerator::Impl::addMacro(const std::string& name, MacroValue value) {
    std::unique_lock lock(mutex_);
    macros_[name] = std::move(value);
    // Invalidate cache as macros have changed
    std::unique_lock cacheLock(cache_mutex_);
    macro_cache_.clear();
}

void TaskGenerator::Impl::removeMacro(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = macros_.find(name);
    if (it != macros_.end()) {
        macros_.erase(it);
        // Invalidate cache as macros have changed
        std::unique_lock cacheLock(cache_mutex_);
        macro_cache_.clear();
    } else {
        throw TaskGeneratorException("Attempted to remove undefined macro: " +
                                     name);
    }
}

auto TaskGenerator::Impl::listMacros() const -> std::vector<std::string> {
    std::shared_lock lock(mutex_);
    std::vector<std::string> keys;
    keys.reserve(macros_.size());
    for (const auto& [key, _] : macros_) {
        keys.emplace_back(key);
    }
    return keys;
}

void TaskGenerator::Impl::processJson(json& json_obj) const {
    try {
        for (const auto& [key, value] : json_obj.items()) {
            if (value.is_string()) {
                value = replaceMacros(value.get<std::string>());
            } else if (value.is_object() || value.is_array()) {
                processJson(value);
            }
        }
    } catch (const TaskGeneratorException& e) {
        LOG_F(ERROR, "Error processing JSON: {}", e.what());
        throw;
    }
}

void TaskGenerator::Impl::processJsonWithJsonMacros(json& json_obj) {
    try {
        preprocessJsonMacros(json_obj);  // Preprocess macros
        processJson(json_obj);           // Replace macros in JSON
    } catch (const TaskGeneratorException& e) {
        LOG_F(ERROR, "Error processing JSON with macros: {}", e.what());
        throw;
    }
}

auto TaskGenerator::Impl::replaceMacros(const std::string& input) const
    -> std::string {
    std::string result = input;
    Match match;

    while (std::regex_search(result, match, MACRO_PATTERN)) {
        std::string fullMatch = match[0];
        std::string macroContent = match[1].str();

        // Check cache first
        {
            std::shared_lock cacheLock(cache_mutex_);
            auto cacheIt = macro_cache_.find(macroContent);
            if (cacheIt != macro_cache_.end()) {
                result.replace(match.position(0), match.length(0),
                               cacheIt->second);
                continue;
            }
        }

        std::string replacement;
        try {
            auto pos = macroContent.find('(');
            if (pos == std::string::npos) {
                // Simple macro replacement
                std::string macroName = macroContent;
                replacement = evaluateMacro(macroName, {});
            } else {
                // Macro with arguments
                if (macroContent.back() != ')') {
                    throw TaskGeneratorException("Malformed macro: " +
                                                 macroContent);
                }
                std::string macroName = macroContent.substr(0, pos);
                std::string argsStr = macroContent.substr(
                    pos + 1, macroContent.length() - pos - 2);

                std::vector<std::string> args;
                std::sregex_token_iterator iter(argsStr.begin(), argsStr.end(),
                                                ARG_PATTERN);
                std::sregex_token_iterator end;
                for (; iter != end; ++iter) {
                    std::string arg = atom::utils::trim(iter->str());
                    arg = replaceMacros(arg);  // Recursive replacement
                    args.emplace_back(std::move(arg));
                }

                replacement = evaluateMacro(macroName, args);
            }

            // Update cache
            {
                std::unique_lock cacheLock(cache_mutex_);
                macro_cache_[macroContent] = replacement;
            }

            result.replace(match.position(0), match.length(0), replacement);
        } catch (const TaskGeneratorException& e) {
            LOG_F(ERROR, "Error replacing macro '{}': {}", macroContent,
                  e.what());
            throw;
        }
    }

    return result;
}

auto TaskGenerator::Impl::evaluateMacro(
    const std::string& name,
    const std::vector<std::string>& args) const -> std::string {
    std::shared_lock lock(mutex_);
    auto it = macros_.find(name);
    if (it == macros_.end()) {
        throw TaskGeneratorException("Undefined macro: " + name);
    }

    if (std::holds_alternative<std::string>(it->second)) {
        return std::get<std::string>(it->second);
    }
    if (std::holds_alternative<
            std::function<std::string(const std::vector<std::string>&)>>(
            it->second)) {
        try {
            return std::get<
                std::function<std::string(const std::vector<std::string>&)>>(
                it->second)(args);
        } catch (const std::exception& e) {
            throw TaskGeneratorException("Error evaluating macro '" + name +
                                         "': " + e.what());
        }
    } else {
        throw TaskGeneratorException("Invalid macro type for: " + name);
    }
}

void TaskGenerator::Impl::preprocessJsonMacros(json& json_obj) {
    try {
        for (const auto& [key, value] : json_obj.items()) {
            if (value.is_string()) {
                std::string strValue = value.get<std::string>();
                Match match;
                if (std::regex_match(strValue, match, MACRO_PATTERN)) {
                    std::string macroContent = match[1].str();
                    std::string macroName;
                    std::vector<std::string> args;

                    auto pos = macroContent.find('(');
                    if (pos == std::string::npos) {
                        macroName = macroContent;
                    } else {
                        if (macroContent.back() != ')') {
                            throw TaskGeneratorException(
                                "Malformed macro definition: " + macroContent);
                        }
                        macroName = macroContent.substr(0, pos);
                        std::string argsStr = macroContent.substr(
                            pos + 1, macroContent.length() - pos - 2);

                        std::sregex_token_iterator iter(
                            argsStr.begin(), argsStr.end(), ARG_PATTERN);
                        std::sregex_token_iterator end;
                        for (; iter != end; ++iter) {
                            args.emplace_back(atom::utils::trim(iter->str()));
                        }
                    }

                    // Define macro if not already present
                    {
                        std::unique_lock lock(mutex_);
                        if (macros_.find(key) == macros_.end()) {
                            if (args.empty()) {
                                macros_[key] = macroContent;
                            } else {
                                // For simplicity, store as a concatenated
                                // string
                                macros_[key] = "macro_defined_in_json";
                            }
                        }
                    }

                    LOG_F(INFO, "Preprocessed macro: {} -> {}", key,
                          macroContent);
                }
            } else if (value.is_object() || value.is_array()) {
                preprocessJsonMacros(value);
            }
        }
    } catch (const TaskGeneratorException& e) {
        LOG_F(ERROR, "Error preprocessing JSON macros: {}", e.what());
        throw;
    }
}

TaskGenerator::TaskGenerator() : impl_(std::make_unique<Impl>()) {}

TaskGenerator::~TaskGenerator() = default;

auto TaskGenerator::createShared() -> std::shared_ptr<TaskGenerator> {
    return std::make_shared<TaskGenerator>();
}

void TaskGenerator::addMacro(const std::string& name, MacroValue value) {
    impl_->addMacro(name, std::move(value));
}

void TaskGenerator::removeMacro(const std::string& name) {
    impl_->removeMacro(name);
}

auto TaskGenerator::listMacros() const -> std::vector<std::string> {
    return impl_->listMacros();
}

void TaskGenerator::processJson(json& json_obj) const {
    impl_->processJson(json_obj);
}

void TaskGenerator::processJsonWithJsonMacros(json& json_obj) {
    impl_->processJsonWithJsonMacros(json_obj);
}

}  // namespace lithium