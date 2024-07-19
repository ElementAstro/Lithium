/*
 * argsview.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-6-18

Description: Enhanced ArgsView Class for C++ using C++17/20 features

**************************************************/

#ifndef ATOM_UTILS_ARGSVIEW_HPP
#define ATOM_UTILS_ARGSVIEW_HPP

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "macro.hpp"

namespace atom::utils {

class ArgsView {
public:
    explicit ArgsView(int argc, char** argv);

    void addArgument(
        std::string_view name, std::string_view help = "",
        bool required = false,
        std::optional<std::string_view> defaultValue = std::nullopt);
    void addPositionalArgument(std::string_view name,
                               std::string_view help = "",
                               bool required = false);
    void addFlag(std::string_view name, std::string_view help = "");

    std::string help() const;

    std::optional<std::string_view> get(std::string_view key) const;

    template <typename T>
    std::optional<T> get(std::string_view key) const;

    bool has(std::string_view key) const;

    bool hasFlag(std::string_view flag) const;

    std::vector<std::string_view> getFlags() const;

    std::unordered_map<std::string, std::string_view> getArgs() const;

    void addRule(std::string_view prefix,
                 const std::function<void(std::string_view)>& handler);

private:
    void parseArguments();

    int m_argc_;
    char** m_argv_;
    std::unordered_map<std::string, std::string_view> m_args_;
    std::vector<std::string_view> m_flags_;
    std::vector<std::string> m_positionals_;
    std::vector<std::pair<std::string, std::function<void(std::string_view)>>>
        m_rules_;

    struct Argument {
        std::string_view name;
        std::string_view help;
        bool required;
        std::optional<std::string_view> defaultValue;
    } ATOM_ALIGNAS(64);

    std::unordered_map<std::string, Argument> m_argDefinitions_;
    std::unordered_map<std::string, Argument> m_positionalDefinitions_;
    std::unordered_map<std::string, std::string_view> m_flagDefinitions_;
};

}  // namespace atom::utils

#endif
