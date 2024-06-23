/*
 * argsview.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-19

Description: ArgsView Class for C++

**************************************************/

#include "argsview.hpp"

#include <algorithm>
#include <charconv>
#include <stdexcept>

namespace atom::utils {

ArgsView::ArgsView(int argc, char** argv) : m_argc(argc), m_argv(argv) {
    parseArguments();
}

void ArgsView::addArgument(std::string_view name, std::string_view help,
                           bool required,
                           std::optional<std::string_view> defaultValue) {
    m_argDefinitions.emplace(name,
                             Argument{name, help, required, defaultValue});
}

void ArgsView::addPositionalArgument(std::string_view name,
                                     std::string_view help, bool required) {
    m_positionalDefinitions.emplace(
        name, Argument{name, help, required, std::nullopt});
}

void ArgsView::addFlag(std::string_view name, std::string_view help) {
    m_flagDefinitions.emplace(name, help);
}

std::string ArgsView::help() const {
    std::string helpMessage = "Usage: program [options] ";
    for (const auto& [name, arg] : m_positionalDefinitions) {
        helpMessage += "<" + std::string(name) + "> ";
    }
    helpMessage += "\n\nOptions:\n";
    for (const auto& [name, arg] : m_argDefinitions) {
        helpMessage += "--" + std::string(name) + ": " + std::string(arg.help) +
                       (arg.required ? " (required)" : "");
        if (arg.defaultValue) {
            helpMessage += " (default: " + std::string(*arg.defaultValue) + ")";
        }
        helpMessage += '\n';
    }
    for (const auto& [name, help] : m_flagDefinitions) {
        helpMessage +=
            "--" + std::string(name) + ": " + std::string(help) + '\n';
    }
    return helpMessage;
}

void ArgsView::parseArguments() {
    int positionalIndex = 0;

    for (int i = 1; i < m_argc; ++i) {
        std::string_view arg(m_argv[i]);
        if (arg.substr(0, 2) == "--") {
            auto pos = arg.find('=');
            if (pos != std::string_view::npos) {
                auto key = arg.substr(2, pos - 2);
                m_args[std::string(key)] = arg.substr(pos + 1);
            } else {
                auto flag = arg.substr(2);
                m_flags.emplace_back(flag);
            }
        } else if (arg[0] == '-') {
            for (size_t j = 1; j < arg.size(); ++j) {
                auto flag = arg.substr(j, 1);
                m_flags.emplace_back(flag);
            }
        } else {
            if (positionalIndex < m_positionalDefinitions.size()) {
                auto positionalName =
                    m_positionalDefinitions
                        .at(std::string(m_positionals[positionalIndex++]))
                        .name;
                m_args[std::string(positionalName)] = arg;
            }
        }
    }

    for (const auto& [key, arg] : m_argDefinitions) {
        if (arg.required && !has(key)) {
            throw std::runtime_error("Missing required argument: " +
                                     std::string(key));
        }
        if (!has(key) && arg.defaultValue) {
            m_args[key] = *arg.defaultValue;
        }
    }

    for (const auto& [key, arg] : m_positionalDefinitions) {
        if (arg.required && !has(key)) {
            throw std::runtime_error("Missing required positional argument: " +
                                     std::string(key));
        }
    }
}

std::optional<std::string_view> ArgsView::get(std::string_view key) const {
    if (auto it = m_args.find(std::string(key)); it != m_args.end()) {
        return it->second;
    }
    return std::nullopt;
}

template <typename T>
std::optional<T> ArgsView::get(std::string_view key) const {
    if (auto it = m_args.find(std::string(key)); it != m_args.end()) {
        T value;
        auto [ptr, ec] = std::from_chars(
            it->second.data(), it->second.data() + it->second.size(), value);
        if (ec == std::errc()) {
            return value;
        }
    }
    return std::nullopt;
}

bool ArgsView::has(std::string_view key) const {
    return m_args.count(std::string(key)) > 0;
}

bool ArgsView::hasFlag(std::string_view flag) const {
    return std::find(m_flags.begin(), m_flags.end(), flag) != m_flags.end();
}

std::vector<std::string_view> ArgsView::getFlags() const { return m_flags; }

std::unordered_map<std::string, std::string_view> ArgsView::getArgs() const {
    return m_args;
}

void ArgsView::addRule(std::string_view prefix,
                       std::function<void(std::string_view)> handler) {
    m_rules.emplace_back(prefix, handler);
}

}  // namespace atom::utils