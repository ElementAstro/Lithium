#ifndef ATOM_UTILS_ARGUMENT_PARSER_HPP
#define ATOM_UTILS_ARGUMENT_PARSER_HPP

#include <any>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "atom/macro.hpp"
#include "exception.hpp"

namespace atom::utils {

class ArgumentParser {
public:
    enum class ArgType {
        STRING,
        INTEGER,
        UNSIGNED_INTEGER,
        LONG,
        UNSIGNED_LONG,
        FLOAT,
        DOUBLE,
        BOOLEAN,
        FILEPATH,
        AUTO
    };

    ArgumentParser() = default;
    explicit ArgumentParser(std::string program_name);

    void addArgument(const std::string& name, ArgType type = ArgType::AUTO,
                     bool required = false, const std::any& default_value = {},
                     const std::string& help = "",
                     const std::vector<std::string>& aliases = {});

    void addFlag(const std::string& name, const std::string& help = "",
                 const std::vector<std::string>& aliases = {});

    void addSubcommand(const std::string& name, const std::string& help = "");

    void parse(int argc, std::vector<std::string> argv);

    template <typename T>
    auto get(const std::string& name) const -> std::optional<T>;

    auto getFlag(const std::string& name) const -> bool;

    auto getSubcommandParser(const std::string& name) const
        -> std::optional<std::reference_wrapper<const ArgumentParser>>;

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
    };

    struct Flag {
        bool value;
        std::string help;
        std::vector<std::string> aliases;
    };

    struct Subcommand;

    std::unordered_map<std::string, Argument> arguments_;
    std::unordered_map<std::string, Flag> flags_;
    std::unordered_map<std::string, Subcommand> subcommands_;
    std::unordered_map<std::string, std::string> aliases_;
    std::vector<std::string> positionalArguments_;
    std::string description_;
    std::string epilog_;
    std::string programName_;

    static auto detectType(const std::any& value) -> ArgType;
    static auto parseValue(ArgType type, const std::string& value) -> std::any;
    static auto argTypeToString(ArgType type) -> std::string;
    static auto anyToString(const std::any& value) -> std::string;
};

struct ArgumentParser::Subcommand {
    std::string help;
    ArgumentParser parser;
};

ATOM_INLINE ArgumentParser::ArgumentParser(std::string program_name)
    : programName_(std::move(program_name)) {}

ATOM_INLINE void ArgumentParser::addArgument(
    const std::string& name, ArgType type, bool required,
    const std::any& default_value, const std::string& help,
    const std::vector<std::string>& aliases) {
    if (type == ArgType::AUTO && default_value.has_value()) {
        type = detectType(default_value);
    } else if (type == ArgType::AUTO) {
        type = ArgType::STRING;
    }

    arguments_[name] = Argument{type, required, default_value, std::nullopt,
                                help, aliases,  false};

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

ATOM_INLINE void ArgumentParser::addSubcommand(const std::string& name,
                                               const std::string& help) {
    subcommands_[name] = Subcommand{help, ArgumentParser(name)};
}

ATOM_INLINE void ArgumentParser::parse(int argc,
                                       std::vector<std::string> argv) {
    if (argc < 1)
        return;

    std::string currentSubcommand;
    std::vector<std::string> subcommandArgs;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (subcommands_.find(arg) != subcommands_.end()) {
            currentSubcommand = arg;
            subcommandArgs.push_back(argv[0]);  // Program name
            continue;
        }

        if (!currentSubcommand.empty()) {
            subcommandArgs.push_back(argv[i]);
            continue;
        }

        if (arg == "--help" || arg == "-h") {
            printHelp();
            std::exit(0);
        } else if (arg.starts_with("--") || arg.starts_with("-")) {
            arg = arg.starts_with("--") ? arg.substr(2) : arg.substr(1);
            if (aliases_.find(arg) != aliases_.end()) {
                arg = aliases_[arg];
            }
            if (flags_.find(arg) != flags_.end()) {
                flags_[arg].value = true;
            } else if (arguments_.find(arg) != arguments_.end()) {
                if (i + 1 < argc) {
                    arguments_[arg].value =
                        parseValue(arguments_[arg].type, argv[++i]);
                } else {
                    THROW_INVALID_ARGUMENT("Value for argument " + arg +
                                           " not provided");
                }
            } else {
                THROW_INVALID_ARGUMENT("Unknown argument: " + arg);
            }
        } else {
            positionalArguments_.push_back(arg);
        }
    }

    if (!currentSubcommand.empty() && !subcommandArgs.empty()) {
        subcommands_[currentSubcommand].parser.parse(
            static_cast<int>(subcommandArgs.size()), subcommandArgs);
    }

    for (const auto& [name, argument] : arguments_) {
        if (argument.required && !argument.value.has_value() &&
            !argument.defaultValue.has_value()) {
            THROW_INVALID_ARGUMENT("Argument required: " + name);
        }
    }
}

template <typename T>
auto ArgumentParser::get(const std::string& name) const -> std::optional<T> {
    if (arguments_.find(name) != arguments_.end()) {
        const auto& arg = arguments_.at(name);
        if (arg.value.has_value()) {
            return std::any_cast<T>(arg.value.value());
        }
        if (arg.defaultValue.has_value()) {
            return std::any_cast<T>(arg.defaultValue);
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

ATOM_INLINE auto ArgumentParser::getSubcommandParser(const std::string& name)
    const -> std::optional<std::reference_wrapper<const ArgumentParser>> {
    if (subcommands_.find(name) != subcommands_.end()) {
        return subcommands_.at(name).parser;
    }
    return std::nullopt;
}

ATOM_INLINE void ArgumentParser::printHelp() const {
    std::cout << "Usage:\n  " << programName_ << " [options] ";
    if (!subcommands_.empty()) {
        std::cout << "<subcommand> [subcommand options]";
    }
    std::cout << "\n\n";

    if (!description_.empty()) {
        std::cout << description_ << "\n\n";
    }

    std::cout << "Options:\n";
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
        std::cout << "\n";
    }
    for (const auto& [name, flag] : flags_) {
        std::cout << "  --" << name;
        for (const auto& alias : flag.aliases) {
            std::cout << ", -" << alias;
        }
        std::cout << " : " << flag.help << "\n";
    }

    if (!subcommands_.empty()) {
        std::cout << "\nSubcommands:\n";
        for (const auto& [name, subcommand] : subcommands_) {
            std::cout << "  " << name << " : " << subcommand.help << "\n";
        }
    }

    if (!epilog_.empty()) {
        std::cout << "\n" << epilog_ << "\n";
    }
}

ATOM_INLINE auto ArgumentParser::detectType(const std::any& value) -> ArgType {
    if (value.type() == typeid(int)) {
        return ArgType::INTEGER;
    }
    if (value.type() == typeid(unsigned int)) {
        return ArgType::UNSIGNED_INTEGER;
    }
    if (value.type() == typeid(long)) {
        return ArgType::LONG;
    }
    if (value.type() == typeid(unsigned long)) {
        return ArgType::UNSIGNED_LONG;
    }
    if (value.type() == typeid(float)) {
        return ArgType::FLOAT;
    }
    if (value.type() == typeid(double)) {
        return ArgType::DOUBLE;
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
    return ArgType::STRING;
}

ATOM_INLINE auto ArgumentParser::parseValue(
    ArgType type, const std::string& value) -> std::any {
    try {
        switch (type) {
            case ArgType::STRING:
                return value;
            case ArgType::INTEGER:
                return std::stoi(value);
            case ArgType::UNSIGNED_INTEGER:
                return static_cast<unsigned int>(std::stoul(value));
            case ArgType::LONG:
                return std::stol(value);
            case ArgType::UNSIGNED_LONG:
                return std::stoul(value);
            case ArgType::FLOAT:
                return std::stof(value);
            case ArgType::DOUBLE:
                return std::stod(value);
            case ArgType::BOOLEAN:
                return (value == "true" || value == "1");
            case ArgType::FILEPATH:
                return std::filesystem::path(value);
            default:
                return value;
        }
    } catch (...) {
        THROW_INVALID_ARGUMENT("Unable to parse argument value: " + value);
    }
}

ATOM_INLINE auto ArgumentParser::argTypeToString(ArgType type) -> std::string {
    switch (type) {
        case ArgType::STRING:
            return "string";
        case ArgType::INTEGER:
            return "integer";
        case ArgType::UNSIGNED_INTEGER:
            return "unsigned integer";
        case ArgType::LONG:
            return "long";
        case ArgType::UNSIGNED_LONG:
            return "unsigned long";
        case ArgType::FLOAT:
            return "float";
        case ArgType::DOUBLE:
            return "double";
        case ArgType::BOOLEAN:
            return "boolean";
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
    if (value.type() == typeid(std::string)) {
        return std::any_cast<std::string>(value);
    }
    if (value.type() == typeid(int)) {
        return std::to_string(std::any_cast<int>(value));
    }
    if (value.type() == typeid(unsigned int)) {
        return std::to_string(std::any_cast<unsigned int>(value));
    }
    if (value.type() == typeid(long)) {
        return std::to_string(std::any_cast<long>(value));
    }
    if (value.type() == typeid(unsigned long)) {
        return std::to_string(std::any_cast<unsigned long>(value));
    }
    if (value.type() == typeid(float)) {
        return std::to_string(std::any_cast<float>(value));
    }
    if (value.type() == typeid(double)) {
        return std::to_string(std::any_cast<double>(value));
    }
    if (value.type() == typeid(bool)) {
        return std::any_cast<bool>(value) ? "true" : "false";
    }
    if (value.type() == typeid(std::filesystem::path)) {
        return std::any_cast<std::filesystem::path>(value).string();
    }
    return "unknown type";
}

}  // namespace atom::utils

#endif  // ATOM_UTILS_ARGUMENT_PARSER_HPP