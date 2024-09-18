#ifndef ATOM_UTILS_ARGUMENT_PARSER_HPP
#define ATOM_UTILS_ARGUMENT_PARSER_HPP

#include <any>
#include <filesystem>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "exception.hpp"
#include "macro.hpp"

namespace atom::utils {

class ArgumentParser {
public:
    enum class ArgType { STRING, INTEGER, FLOAT, BOOLEAN, FILEPATH, AUTO };

    ArgumentParser() = default;

    explicit ArgumentParser(const std::string& program_name);

    void addArgument(const std::string& name, ArgType type = ArgType::AUTO,
                     bool required = false, const std::any& default_value = {},
                     const std::string& help = "",
                     const std::vector<std::string>& aliases = {});

    void addDescription(const std::string& description) {
        description_ = description;
    }

    void addEpilog(const std::string& epilog) { epilog_ = epilog; }

    void addFlag(const std::string& name, const std::string& help = "",
                 const std::vector<std::string>& aliases = {});

    void addMultivalueArgument(const std::string& name,
                               ArgType type = ArgType::AUTO,
                               bool required = false,
                               const std::string& help = "",
                               const std::vector<std::string>& aliases = {});

    void parse(int argc, char* argv[]);

    template <typename T>
    std::optional<T> get(const std::string& name) const;

    template <typename T>
    auto getMultivalue(const std::string& name) const
        -> std::optional<std::vector<T>>;

    auto getFlag(const std::string& name) const -> bool;

    void printHelp() const;

private:
    struct Argument {
        ArgType type;
        bool required;
        std::any defaultValue;
        std::optional<std::any> value;
        std::string help;
        std::vector<std::string> aliases;
        bool isMultivalue;
    } ATOM_ALIGNAS(128);

    struct Flag {
        bool value;
        std::string help;
        std::vector<std::string> aliases;
    } ATOM_ALIGNAS(64);

    std::unordered_map<std::string, Argument> arguments_;
    std::unordered_map<std::string, Flag> flags_;
    std::unordered_map<std::string, std::string> aliases_;
    std::vector<std::string> positional_arguments_;
    std::string description_;
    std::string epilog_;

    static auto detectType(const std::any& value) -> ArgType;

    static auto parseValue(ArgType type, const std::string& value) -> std::any;

    static auto argTypeToString(ArgType type) -> std::string;

    static auto anyToString(const std::any& value) -> std::string;
};

ATOM_INLINE ArgumentParser::ArgumentParser(const std::string& program_name) {
    addArgument("help", ArgType::BOOLEAN, false, false, "Print this help");
    addArgument("program_name", ArgType::STRING, false, program_name,
                "Program name");
}

ATOM_INLINE void ArgumentParser::addArgument(
    const std::string& name, ArgType type, bool required,
    const std::any& default_value, const std::string& help,
    const std::vector<std::string>& aliases) {
    ArgType detectedType = type;
    if (type == ArgType::AUTO) {
        detectedType = detectType(default_value);
    }
    arguments_[name] =
        Argument{detectedType, required, default_value, std::nullopt,
                 help,         aliases,  false};
    for (const auto& alias : aliases) {
        aliases_[alias] = name;
    }
}

ATOM_INLINE void ArgumentParser::addFlag(
    const std::string& name, const std::string& help,
    const std::vector<std::string>& aliases) {
    flags_[name] = Flag{false, help, aliases};
    for (const auto& alias : aliases) {
        aliases_[alias] = name;
    }
}

ATOM_INLINE void ArgumentParser::addMultivalueArgument(
    const std::string& name, ArgType type, bool required,
    const std::string& help, const std::vector<std::string>& aliases) {
    arguments_[name] =
        Argument{type, required, {}, std::nullopt, help, aliases, true};
    for (const auto& alias : aliases) {
        aliases_[alias] = name;
    }
}

ATOM_INLINE void ArgumentParser::parse(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printHelp();
            exit(0);
        } else if (arg.starts_with("--") || arg.starts_with("-")) {
            arg = arg.starts_with("--") ? arg.substr(2) : arg.substr(1);
            if (aliases_.find(arg) != aliases_.end()) {
                arg = aliases_[arg];
            }
            if (flags_.find(arg) != flags_.end()) {
                flags_[arg].value = true;
            } else if (arguments_.find(arg) != arguments_.end()) {
                if (arguments_[arg].isMultivalue) {
                    std::vector<std::string> values;
                    while (i + 1 < argc &&
                           !std::string(argv[i + 1]).starts_with("--")) {
                        values.emplace_back(argv[++i]);
                    }
                    arguments_[arg].value = values;
                } else {
                    if (i + 1 < argc) {
                        arguments_[arg].value =
                            parseValue(arguments_[arg].type, argv[++i]);
                    } else {
                        THROW_INVALID_ARGUMENT("Value for argument " + arg +
                                               " not provided");
                    }
                }
            } else {
                THROW_INVALID_ARGUMENT("Unknown argument: " + arg);
            }
        } else {
            positional_arguments_.push_back(arg);
        }
    }

    for (const auto& [name, argument] : arguments_) {
        if (argument.required && !argument.value.has_value() &&
            !argument.defaultValue.has_value()) {
            THROW_INVALID_ARGUMENT("Required argument " + name +
                                   " not provided");
        }
    }
}

template <typename T>
auto ArgumentParser::get(const std::string& name) const -> std::optional<T> {
    if (arguments_.find(name) != arguments_.end()) {
        if (arguments_.at(name).value.has_value()) {
            std::cout << typeid(arguments_.at(name).value.value()).name()
                      << "\n";
            if (arguments_.at(name).value.has_value()) {
                return std::any_cast<T>(arguments_.at(name).value.value());
            }
            THROW_INVALID_ARGUMENT("Invalid value for argument " + name);
        }
        if (arguments_.at(name).defaultValue.has_value()) {
            return std::any_cast<T>(arguments_.at(name).defaultValue);
        }
    }
    return std::nullopt;
}

template <typename T>
auto ArgumentParser::getMultivalue(const std::string& name) const
    -> std::optional<std::vector<T>> {
    if (arguments_.find(name) != arguments_.end()) {
        if (arguments_.at(name).value.has_value()) {
            std::cout << typeid(arguments_.at(name).value.value()).name()
                      << "\n";
            return std::any_cast<std::vector<T>>(
                arguments_.at(name).value.value());
        }
    }
    return std::nullopt;
}

ATOM_INLINE auto ArgumentParser::getFlag(const std::string& name) const
    -> bool {
    if (flags_.find(name) != flags_.end()) {
        return flags_.at(name).value;
    }
    return false;
}

ATOM_INLINE void ArgumentParser::printHelp() const {
    std::cout << "Usage:\n";
    std::cout << description_ << "\n\n";
    for (const auto& [name, argument] : arguments_) {
        std::cout << "  --" << name;
        for (const auto& alias : argument.aliases) {
            std::cout << ", -" << alias;
        }
        std::cout << " : " << argument.help;
        if (argument.defaultValue.has_value()) {
            std::cout << " (default: " << anyToString(argument.defaultValue)
                      << ")";
        }
        std::cout << " [" << argTypeToString(argument.type) << "]";
        if (argument.required) {
            std::cout << " (required)";
        }
        std::cout << "\n";
    }
    for (const auto& [name, flag] : flags_) {
        std::cout << "  --" << name;
        for (const auto& alias : flag.aliases) {
            std::cout << ", -" << alias;
        }
        std::cout << " : " << flag.help << "\n";
    }
    std::cout << "\n" << epilog_ << std::endl;
}

ATOM_INLINE auto ArgumentParser::detectType(const std::any& value) -> ArgType {
    if (value.type() == typeid(int)) {
        return ArgType::INTEGER;
    }
    if (value.type() == typeid(float)) {
        return ArgType::FLOAT;
    }
    if (value.type() == typeid(bool)) {
        return ArgType::BOOLEAN;
    }
    if (value.type() == typeid(std::string)) {
        return ArgType::STRING;
    }
    if (value.type() == typeid(std::filesystem::path)) {
        return ArgType::FILEPATH;
    }
    return ArgType::STRING;  // Default to string if undetectable
}

ATOM_INLINE auto ArgumentParser::parseValue(
    ArgType type, const std::string& value) -> std::any {
    switch (type) {
        case ArgType::STRING:
            return value;
        case ArgType::INTEGER: {
            int intValue;
            std::istringstream(value) >> intValue;
            return intValue;
        }
        case ArgType::FLOAT: {
            float floatValue;
            std::istringstream(value) >> floatValue;
            return floatValue;
        }
        case ArgType::BOOLEAN:
            return value == "true";
        case ArgType::FILEPATH:
            return std::filesystem::path(value);
        case ArgType::AUTO: {
            if (value == "true" || value == "false") {
                return value == "true";
            }
            if (value.find('.') != std::string::npos) {
                float floatValue;
                std::istringstream(value) >> floatValue;
                return floatValue;
            }
            int intValue;
            std::istringstream(value) >> intValue;
            if (!std::istringstream(value).fail()) {
                return intValue;
            }
            return value;
        }
        default:
            THROW_INVALID_ARGUMENT("Unknown argument type");
    }
}

ATOM_INLINE auto ArgumentParser::argTypeToString(ArgType type) -> std::string {
    switch (type) {
        case ArgType::STRING:
            return "string";
        case ArgType::INTEGER:
            return "int";
        case ArgType::FLOAT:
            return "float";
        case ArgType::BOOLEAN:
            return "bool";
        case ArgType::FILEPATH:
            return "filepath";
        case ArgType::AUTO:
            return "auto";
        default:
            return "unknown";
    }
}

ATOM_INLINE auto ArgumentParser::anyToString(const std::any& value)
    -> std::string {
    try {
        if (value.type() == typeid(std::string)) {
            return std::any_cast<std::string>(value);
        }
        if (value.type() == typeid(int)) {
            return std::to_string(std::any_cast<int>(value));
        }
        if (value.type() == typeid(float)) {
            return std::to_string(std::any_cast<float>(value));
        }
        if (value.type() == typeid(bool)) {
            return std::any_cast<bool>(value) ? "true" : "false";
        }
        if (value.type() == typeid(std::vector<std::string>)) {
            const auto& vec = std::any_cast<std::vector<std::string>>(value);
            std::string result;
            for (const auto& v : vec) {
                if (!result.empty()) {
                    result += ", ";
                }
                result += v;
            }
            return result;
        }
        if (value.type() == typeid(std::filesystem::path)) {
            return std::any_cast<std::filesystem::path>(value).string();
        }
    } catch (const std::bad_any_cast&) {
        return "unknown";
    }
    return "unknown";
}

}  // namespace atom::utils

#endif