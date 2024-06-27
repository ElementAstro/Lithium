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

ArgsView::ArgsView(int argc, char** argv) : m_argc_(argc), m_argv_(argv) {
    parseArguments();
}

void ArgsView::addArgument(std::string_view name, std::string_view help,
                           bool required,
                           std::optional<std::string_view> defaultValue) {
    m_argDefinitions_.emplace(name,
                              Argument{name, help, required, defaultValue});
}

void ArgsView::addPositionalArgument(std::string_view name,
                                     std::string_view help, bool required) {
    m_positionalDefinitions_.emplace(
        name, Argument{name, help, required, std::nullopt});
}

void ArgsView::addFlag(std::string_view name, std::string_view help) {
    m_flagDefinitions_.emplace(name, help);
}

std::string ArgsView::help() const {
    std::string helpMessage = "Usage: program [options] ";
    for (const auto& [name, arg] : m_positionalDefinitions_) {
        helpMessage += "<" + std::string(name) + "> ";
    }
    helpMessage += "\n\nOptions:\n";
    for (const auto& [name, arg] : m_argDefinitions_) {
        helpMessage += "--" + std::string(name) + ": " + std::string(arg.help) +
                       (arg.required ? " (required)" : "");
        if (arg.defaultValue) {
            helpMessage += " (default: " + std::string(*arg.defaultValue) + ")";
        }
        helpMessage += '\n';
    }
    for (const auto& [name, help] : m_flagDefinitions_) {
        helpMessage +=
            "--" + std::string(name) + ": " + std::string(help) + '\n';
    }
    return helpMessage;
}

void ArgsView::parseArguments() {
    int positionalIndex = 0;

    for (int i = 1; i < m_argc_; ++i) {
        std::string_view arg(m_argv_[i]);
        if (arg.substr(0, 2) == "--") {
            auto pos = arg.find('=');
            if (pos != std::string_view::npos) {
                auto key = arg.substr(2, pos - 2);
                m_args_[std::string(key)] = arg.substr(pos + 1);
            } else {
                auto flag = arg.substr(2);
                m_flags_.emplace_back(flag);
            }
        } else if (arg[0] == '-') {
            for (size_t j = 1; j < arg.size(); ++j) {
                auto flag = arg.substr(j, 1);
                m_flags_.emplace_back(flag);
            }
        } else {
            if (positionalIndex < m_positionalDefinitions_.size()) {
                auto positionalName =
                    m_positionalDefinitions_
                        .at(std::string(m_positionals_[positionalIndex++]))
                        .name;
                m_args_[std::string(positionalName)] = arg;
            }
        }
    }

    for (const auto& [key, arg] : m_argDefinitions_) {
        if (arg.required && !has(key)) {
            throw std::runtime_error("Missing required argument: " +
                                     std::string(key));
        }
        if (!has(key) && arg.defaultValue) {
            m_args_[key] = *arg.defaultValue;
        }
    }

    for (const auto& [key, arg] : m_positionalDefinitions_) {
        if (arg.required && !has(key)) {
            throw std::runtime_error("Missing required positional argument: " +
                                     std::string(key));
        }
    }
}

std::optional<std::string_view> ArgsView::get(std::string_view key) const {
    if (auto it = m_args_.find(std::string(key)); it != m_args_.end()) {
        return it->second;
    }
    return std::nullopt;
}

template <typename T>
std::optional<T> ArgsView::get(std::string_view key) const {
    if (auto it = m_args_.find(std::string(key)); it != m_args_.end()) {
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
    return m_args_.contains(std::string(key));
}

bool ArgsView::hasFlag(std::string_view flag) const {
    return std::find(m_flags_.begin(), m_flags_.end(), flag) != m_flags_.end();
}

std::vector<std::string_view> ArgsView::getFlags() const { return m_flags_; }

std::unordered_map<std::string, std::string_view> ArgsView::getArgs() const {
    return m_args_;
}

void ArgsView::addRule(std::string_view prefix,
                       const std::function<void(std::string_view)>& handler) {
    m_rules_.emplace_back(prefix, handler);
}

}  // namespace atom::utils