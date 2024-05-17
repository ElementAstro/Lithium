/*
 * fnmatch.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-2

Description: Python Like fnmatch for C++

**************************************************/

#ifndef ATOM_SYSTEM_FNMATCH_HPP
#define ATOM_SYSTEM_FNMATCH_HPP

#include <string>
#include <string_view>
#include <vector>

namespace atom::algorithm {
/**
 * @brief Matches a string against a specified pattern.
 *
 * This function compares the given `string` against the specified `pattern`
 * using shell-style pattern matching. The `flags` parameter can be used to
 * modify the behavior of the matching process.
 *
 * @param pattern The pattern to match against.
 * @param string The string to match.
 * @param flags Optional flags to modify the matching behavior (default is 0).
 * @return True if the `string` matches the `pattern`, false otherwise.
 */
bool fnmatch(std::string_view pattern, std::string_view string, int flags = 0);

/**
 * @brief Filters a vector of strings based on a specified pattern.
 *
 * This function filters the given vector of `names` based on the specified
 * `pattern` using shell-style pattern matching. The `flags` parameter can be
 * used to modify the filtering behavior.
 *
 * @param names The vector of strings to filter.
 * @param pattern The pattern to filter with.
 * @param flags Optional flags to modify the filtering behavior (default is 0).
 * @return True if any element of `names` matches the `pattern`, false
 * otherwise.
 */
bool filter(const std::vector<std::string>& names, std::string_view pattern,
            int flags = 0);

/**
 * @brief Filters a vector of strings based on multiple patterns.
 *
 * This function filters the given vector of `names` based on the specified
 * `patterns` using shell-style pattern matching. The `flags` parameter can be
 * used to modify the filtering behavior.
 *
 * @param names The vector of strings to filter.
 * @param patterns The vector of patterns to filter with.
 * @param flags Optional flags to modify the filtering behavior (default is 0).
 * @return A vector containing strings from `names` that match any pattern in
 * `patterns`.
 */
std::vector<std::string> filter(const std::vector<std::string>& names,
                                const std::vector<std::string>& patterns,
                                int flags = 0);

/**
 * @brief Translates a pattern into a different representation.
 *
 * This function translates the specified `pattern` into a different
 * representation and stores the result in the `result` parameter. The `flags`
 * parameter can be used to modify the translation behavior.
 *
 * @param pattern The pattern to translate.
 * @param result A reference to a string where the translated pattern will be
 * stored.
 * @param flags Optional flags to modify the translation behavior (default is
 * 0).
 * @return True if the translation was successful, false otherwise.
 */
bool translate(std::string_view pattern, std::string& result, int flags = 0);

}  // namespace atom::algorithm

#endif  // ATOM_SYSTEM_FNMATCH_HPP
