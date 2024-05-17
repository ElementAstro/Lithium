/*
 * argsview.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-19

Description: ArgsView Class for C++

**************************************************/

#ifndef ATOM_UTILS_ARGSVIEW_HPP
#define ATOM_UTILS_ARGSVIEW_HPP

#include <algorithm>
#include <charconv>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace atom::utils {
/**
 * @brief Represents a view of command-line arguments.
 */
class ArgsView {
public:
    /**
     * @brief Constructs an ArgsView object from argc and argv.
     *
     * @param argc The number of command-line arguments.
     * @param argv The array of command-line arguments.
     */
    explicit ArgsView(int argc, char** argv);

    /**
     * @brief Gets the value associated with the specified key.
     *
     * @param key The key to search for.
     * @return An optional containing the value if found, otherwise nullopt.
     * @throws std::runtime_error if the key is not found.
     */
    std::optional<std::string_view> get(std::string_view key) const;

    /**
     * @brief Gets the value associated with the specified key and converts it
     * to the specified type.
     *
     * @tparam T The type to convert the value to.
     * @param key The key to search for.
     * @return An optional containing the converted value if found and
     * successfully converted, otherwise nullopt.
     */
    template <typename T>
    std::optional<T> get(std::string_view key) const;

    /**
     * @brief Checks if the specified key exists.
     *
     * @param key The key to search for.
     * @return true if the key exists, otherwise false.
     */
    bool has(std::string_view key) const;

    /**
     * @brief Checks if the specified flag exists.
     *
     * @param flag The flag to search for.
     * @return true if the flag exists, otherwise false.
     */
    bool hasFlag(std::string_view flag) const;

    /**
     * @brief Gets all the flags.
     *
     * @return A vector containing all the flags.
     */
    std::vector<std::string_view> getFlags() const;

    /**
     * @brief Gets all the key-value pairs.
     *
     * @return An unordered_map containing all the key-value pairs.
     */
    std::unordered_map<std::string_view, std::string_view> getArgs() const;

    /**
     * @brief Adds a custom rule with the specified prefix and handler function.
     *
     * @param prefix The prefix of the rule.
     * @param handler The handler function for the rule.
     */
    void addRule(std::string_view prefix,
                 std::function<void(std::string_view)> handler);

private:
    int m_argc;
    char** m_argv;
    std::unordered_map<std::string_view, std::string_view> m_args;
    std::vector<std::string_view> m_flags;
    std::vector<
        std::pair<std::string_view, std::function<void(std::string_view)>>>
        m_rules;
};

}  // namespace atom::utils

#endif
