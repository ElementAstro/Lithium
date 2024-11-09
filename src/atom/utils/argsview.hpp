#ifndef ATOM_UTILS_ARGUMENT_PARSER_HPP
#define ATOM_UTILS_ARGUMENT_PARSER_HPP

#include <any>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/macro.hpp"

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

    enum class NargsType {
        NONE,
        OPTIONAL,
        ZERO_OR_MORE,
        ONE_OR_MORE,
        CONSTANT
    };

    struct Nargs {
        NargsType type;
        int count;  // Used if type is CONSTANT

        Nargs() : type(NargsType::NONE), count(1) {}
        Nargs(NargsType t, int c = 1) : type(t), count(c) {}
    };

    ArgumentParser() = default;
    explicit ArgumentParser(std::string program_name);

    // 设置描述和结尾
    void setDescription(const std::string& description);
    void setEpilog(const std::string& epilog);

    void addArgument(const std::string& name, ArgType type = ArgType::AUTO,
                     bool required = false, const std::any& default_value = {},
                     const std::string& help = "",
                     const std::vector<std::string>& aliases = {},
                     bool is_positional = false, const Nargs& nargs = Nargs());

    void addFlag(const std::string& name, const std::string& help = "",
                 const std::vector<std::string>& aliases = {});

    void addSubcommand(const std::string& name, const std::string& help = "");

    void addMutuallyExclusiveGroup(const std::vector<std::string>& group_args);

    // 自定义文件解析
    void addArgumentFromFile(const std::string& prefix = "@");
    void setFileDelimiter(char delimiter);

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
        bool required{};
        std::any defaultValue;
        std::optional<std::any> value;
        std::string help;
        std::vector<std::string> aliases;
        bool isMultivalue{};
        bool is_positional{};
        Nargs nargs;

        Argument() = default;

        Argument(ArgType t, bool req, std::any def, std::string hlp,
                 const std::vector<std::string>& als, bool mult = false,
                 bool pos = false, const Nargs& ng = Nargs())
            : type(t),
              required(req),
              defaultValue(std::move(def)),
              help(std::move(hlp)),
              aliases(als),
              isMultivalue(mult),
              is_positional(pos),
              nargs(ng) {}
    } ATOM_ALIGNAS(128);

    struct Flag {
        bool value;
        std::string help;
        std::vector<std::string> aliases;
    } ATOM_ALIGNAS(64);

    struct Subcommand;

    std::unordered_map<std::string, Argument> arguments_;
    std::unordered_map<std::string, Flag> flags_;
    std::unordered_map<std::string, Subcommand> subcommands_;
    std::unordered_map<std::string, std::string> aliases_;
    std::vector<std::string> positionalArguments_;
    std::string description_;
    std::string epilog_;
    std::string programName_;

    std::vector<std::vector<std::string>> mutuallyExclusiveGroups_;

    // 文件解析相关
    bool enableFileParsing_ = false;
    std::string filePrefix_ = "@";
    char fileDelimiter_ = ' ';

    static auto detectType(const std::any& value) -> ArgType;
    static auto parseValue(ArgType type, const std::string& value) -> std::any;
    static auto argTypeToString(ArgType type) -> std::string;
    static auto anyToString(const std::any& value) -> std::string;
    void expandArgumentsFromFile(std::vector<std::string>& argv);
};

struct ArgumentParser::Subcommand {
    std::string help;
    ArgumentParser parser;
} ATOM_ALIGNAS(128);

inline ArgumentParser::ArgumentParser(std::string program_name)
    : programName_(std::move(program_name)) {}

inline void ArgumentParser::setDescription(const std::string& description) {
    description_ = description;
}

inline void ArgumentParser::setEpilog(const std::string& epilog) {
    epilog_ = epilog;
}

inline void ArgumentParser::addArgument(const std::string& name, ArgType type,
                                        bool required,
                                        const std::any& default_value,
                                        const std::string& help,
                                        const std::vector<std::string>& aliases,
                                        bool is_positional,
                                        const Nargs& nargs) {
    if (type == ArgType::AUTO && default_value.has_value()) {
        type = detectType(default_value);
    } else if (type == ArgType::AUTO) {
        type = ArgType::STRING;
    }

    arguments_[name] =
        Argument{type,          required, default_value,
                 help,          aliases,  nargs.type != NargsType::NONE,
                 is_positional, nargs};

    for (const auto& alias : aliases) {
        aliases_[alias] = name;
    }
}

inline void ArgumentParser::addFlag(const std::string& name,
                                    const std::string& help,
                                    const std::vector<std::string>& aliases) {
    flags_[name] = Flag{false, help, aliases};
    for (const auto& alias : aliases) {
        aliases_[alias] = name;
    }
}

inline void ArgumentParser::addSubcommand(const std::string& name,
                                          const std::string& help) {
    subcommands_[name] = Subcommand{help, ArgumentParser(name)};
}

inline void ArgumentParser::addMutuallyExclusiveGroup(
    const std::vector<std::string>& group_args) {
    mutuallyExclusiveGroups_.emplace_back(group_args);
}

inline void ArgumentParser::addArgumentFromFile(const std::string& prefix) {
    enableFileParsing_ = true;
    filePrefix_ = prefix;
}

inline void ArgumentParser::setFileDelimiter(char delimiter) {
    fileDelimiter_ = delimiter;
}

inline void ArgumentParser::parse(int argc, std::vector<std::string> argv) {
    if (argc < 1)
        return;

    // 扩展来自文件的参数
    if (enableFileParsing_) {
        expandArgumentsFromFile(argv);
    }

    std::string currentSubcommand;
    std::vector<std::string> subcommandArgs;

    // Track which mutually exclusive groups have been used
    std::vector<bool> groupUsed(mutuallyExclusiveGroups_.size(), false);

    for (size_t i = 0; i < argv.size(); ++i) {
        std::string arg = argv[i];

        // Check for subcommand
        if (subcommands_.find(arg) != subcommands_.end()) {
            currentSubcommand = arg;
            subcommandArgs.push_back(argv[0]);  // Program name
            continue;
        }

        // If inside a subcommand, pass arguments to subcommand parser
        if (!currentSubcommand.empty()) {
            subcommandArgs.push_back(argv[i]);
            continue;
        }

        // Handle help flag
        if (arg == "--help" || arg == "-h") {
            printHelp();
            std::exit(0);
        }

        // Handle optional arguments and flags
        if (arg.starts_with("--") || arg.starts_with("-")) {
            std::string argName;
            bool isFlag = false;

            if (arg.starts_with("--")) {
                argName = arg.substr(2);
            } else {
                argName = arg.substr(1);
            }

            // Resolve aliases
            if (aliases_.find(argName) != aliases_.end()) {
                argName = aliases_[argName];
            }

            // Check if it's a flag
            if (flags_.find(argName) != flags_.end()) {
                flags_[argName].value = true;
                continue;
            }

            // Check if it's an argument
            if (arguments_.find(argName) != arguments_.end()) {
                Argument& argument = arguments_[argName];
                std::vector<std::string> values;

                // Handle nargs
                int expected = 1;
                bool is_constant = false;
                if (argument.nargs.type == NargsType::ONE_OR_MORE) {
                    expected = -1;  // Indicate multiple
                } else if (argument.nargs.type == NargsType::ZERO_OR_MORE) {
                    expected = -1;
                } else if (argument.nargs.type == NargsType::OPTIONAL) {
                    expected = 1;
                } else if (argument.nargs.type == NargsType::CONSTANT) {
                    expected = argument.nargs.count;
                    is_constant = true;
                }

                // Collect values based on nargs
                for (int j = 0; j < expected || expected == -1; ++j) {
                    if (i + 1 < static_cast<int>(argv.size()) &&
                        !argv[i + 1].starts_with("-")) {
                        values.emplace_back(argv[++i]);
                    } else {
                        break;
                    }
                }

                if (is_constant &&
                    static_cast<int>(values.size()) != argument.nargs.count) {
                    THROW_INVALID_ARGUMENT(
                        "Argument " + argName + " expects " +
                        std::to_string(argument.nargs.count) + " value(s).");
                }

                if (values.empty() &&
                    argument.nargs.type == NargsType::OPTIONAL) {
                    // Optional argument without a value
                    if (argument.defaultValue.has_value()) {
                        argument.value = argument.defaultValue;
                    }
                } else if (!values.empty()) {
                    if (expected == -1) {  // Multiple values
                        // Store as vector<string>
                        argument.value = std::any(values);
                    } else {  // Single value
                        argument.value = parseValue(argument.type, values[0]);
                    }
                }

                continue;
            }

            THROW_INVALID_ARGUMENT("Unknown argument: " + arg);
        }

        // Handle positional arguments
        positionalArguments_.push_back(arg);
    }

    if (!currentSubcommand.empty() && !subcommandArgs.empty()) {
        subcommands_[currentSubcommand].parser.parse(
            static_cast<int>(subcommandArgs.size()), subcommandArgs);
    }

    // Validate mutually exclusive groups
    for (size_t g = 0; g < mutuallyExclusiveGroups_.size(); ++g) {
        int count = 0;
        for (const auto& arg : mutuallyExclusiveGroups_[g]) {
            if (flags_.find(arg) != flags_.end() && flags_[arg].value) {
                count++;
            }
            if (arguments_.find(arg) != arguments_.end() &&
                arguments_[arg].value.has_value()) {
                count++;
            }
        }
        if (count > 1) {
            THROW_INVALID_ARGUMENT("Arguments in mutually exclusive group " +
                                   std::to_string(g + 1) +
                                   " cannot be used together.");
        }
    }

    // Check required arguments
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
            try {
                return std::any_cast<T>(arg.value.value());
            } catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
        if (arg.defaultValue.has_value()) {
            try {
                return std::any_cast<T>(arg.defaultValue);
            } catch (const std::bad_any_cast&) {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

inline auto ArgumentParser::getFlag(const std::string& name) const -> bool {
    if (flags_.find(name) != flags_.end()) {
        return flags_.at(name).value;
    }
    return false;
}

inline auto ArgumentParser::getSubcommandParser(const std::string& name) const
    -> std::optional<std::reference_wrapper<const ArgumentParser>> {
    if (subcommands_.find(name) != subcommands_.end()) {
        return subcommands_.at(name).parser;
    }
    return std::nullopt;
}

inline void ArgumentParser::printHelp() const {
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
        if (argument.is_positional)
            continue;
        std::cout << "  --" << name;
        for (const auto& alias : argument.aliases) {
            std::cout << ", -" << alias;
        }
        std::cout << " : " << argument.help;
        if (argument.defaultValue.has_value()) {
            std::cout << " (default: " << anyToString(argument.defaultValue)
                      << ")";
        }
        if (argument.nargs.type != NargsType::NONE) {
            std::cout << " [nargs: ";
            switch (argument.nargs.type) {
                case NargsType::OPTIONAL:
                    std::cout << "?";
                    break;
                case NargsType::ZERO_OR_MORE:
                    std::cout << "*";
                    break;
                case NargsType::ONE_OR_MORE:
                    std::cout << "+";
                    break;
                case NargsType::CONSTANT:
                    std::cout << std::to_string(argument.nargs.count);
                    break;
                default:
                    std::cout << "1";
            }
            std::cout << "]";
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

    // Positional arguments
    std::vector<std::string> positional;
    for (const auto& [name, argument] : arguments_) {
        if (argument.is_positional) {
            positional.push_back(name);
        }
    }
    if (!positional.empty()) {
        std::cout << "\nPositional Arguments:\n";
        for (const auto& name : positional) {
            const auto& argument = arguments_.at(name);
            std::cout << "  " << name;
            std::cout << " : " << argument.help;
            if (argument.defaultValue.has_value()) {
                std::cout << " (default: " << anyToString(argument.defaultValue)
                          << ")";
            }
            if (argument.nargs.type != NargsType::NONE) {
                std::cout << " [nargs: ";
                switch (argument.nargs.type) {
                    case NargsType::OPTIONAL:
                        std::cout << "?";
                        break;
                    case NargsType::ZERO_OR_MORE:
                        std::cout << "*";
                        break;
                    case NargsType::ONE_OR_MORE:
                        std::cout << "+";
                        break;
                    case NargsType::CONSTANT:
                        std::cout << std::to_string(argument.nargs.count);
                        break;
                    default:
                        std::cout << "1";
                }
                std::cout << "]";
            }
            std::cout << "\n";
        }
    }

    if (!mutuallyExclusiveGroups_.empty()) {
        std::cout << "\nMutually Exclusive Groups:\n";
        for (size_t g = 0; g < mutuallyExclusiveGroups_.size(); ++g) {
            std::cout << "  Group " << g + 1 << ": ";
            for (size_t i = 0; i < mutuallyExclusiveGroups_[g].size(); ++i) {
                std::cout << "--" << mutuallyExclusiveGroups_[g][i];
                if (i != mutuallyExclusiveGroups_[g].size() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << "\n";
        }
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

inline auto ArgumentParser::detectType(const std::any& value) -> ArgType {
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

inline auto ArgumentParser::parseValue(ArgType type,
                                       const std::string& value) -> std::any {
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

inline auto ArgumentParser::argTypeToString(ArgType type) -> std::string {
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

inline auto ArgumentParser::anyToString(const std::any& value) -> std::string {
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

// 自定义文件解析实现
inline void ArgumentParser::expandArgumentsFromFile(
    std::vector<std::string>& argv) {
    std::vector<std::string> expandedArgs;
    for (const auto& arg : argv) {
        if (arg.starts_with(filePrefix_)) {
            std::string filename = arg.substr(filePrefix_.length());
            std::ifstream infile(filename);
            if (!infile.is_open()) {
                THROW_INVALID_ARGUMENT("Unable to open argument file: " +
                                       filename);
            }
            std::string line;
            while (std::getline(infile, line)) {
                std::istringstream iss(line);
                std::string token;
                while (std::getline(iss, token, fileDelimiter_)) {
                    if (!token.empty()) {
                        expandedArgs.emplace_back(token);
                    }
                }
            }
        } else {
            expandedArgs.emplace_back(arg);
        }
    }
    argv = expandedArgs;
}

}  // namespace atom::utils

#endif  // ATOM_UTILS_ARGUMENT_PARSER_HPP
