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

namespace atom::utils {

class ArgsView {
public:
    explicit ArgsView(int argc, char** argv);

    // 添加参数定义
    void addArgument(
        std::string_view name, std::string_view help = "",
        bool required = false,
        std::optional<std::string_view> defaultValue = std::nullopt);
    void addPositionalArgument(std::string_view name,
                               std::string_view help = "",
                               bool required = false);
    void addFlag(std::string_view name, std::string_view help = "");

    // 自动生成帮助信息
    std::string help() const;

    std::optional<std::string_view> get(std::string_view key) const;

    template <typename T>
    std::optional<T> get(std::string_view key) const;

    bool has(std::string_view key) const;

    bool hasFlag(std::string_view flag) const;

    std::vector<std::string_view> getFlags() const;

    std::unordered_map<std::string, std::string_view> getArgs() const;

    void addRule(std::string_view prefix,
                 std::function<void(std::string_view)> handler);

private:
    void parseArguments();

    int m_argc;
    char** m_argv;
    std::unordered_map<std::string, std::string_view> m_args;
    std::vector<std::string_view> m_flags;
    std::vector<std::string_view> m_positionals;
    std::vector<std::pair<std::string, std::function<void(std::string_view)>>>
        m_rules;

    struct Argument {
        std::string_view name;
        std::string_view help;
        bool required;
        std::optional<std::string_view> defaultValue;
    };

    std::unordered_map<std::string, Argument> m_argDefinitions;
    std::unordered_map<std::string, Argument> m_positionalDefinitions;
    std::unordered_map<std::string, std::string_view> m_flagDefinitions;
};

}  // namespace atom::utils

#endif