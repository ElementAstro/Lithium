/*
 * fnmatch.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-5-2

Description: Enhanced Python-Like fnmatch for C++

**************************************************/

#include "fnmatch.hpp"

#include <algorithm>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#else
#include <fnmatch.h>
#endif

#include "atom/log/loguru.hpp"

namespace atom::algorithm {

#ifdef _WIN32
constexpr int FNM_NOESCAPE = 0x01;
constexpr int FNM_PATHNAME = 0x02;
constexpr int FNM_PERIOD = 0x04;
constexpr int FNM_CASEFOLD = 0x08;
#endif

auto fnmatch(std::string_view pattern, std::string_view string,
             int flags) -> bool {
    LOG_F(INFO, "fnmatch called with pattern: {}, string: {}, flags: {}",
          pattern, string, flags);

    try {
#ifdef _WIN32
        auto p = pattern.begin();
        auto s = string.begin();

        while (p != pattern.end() && s != string.end()) {
            switch (*p) {
                case '?':
                    LOG_F(INFO, "Wildcard '?' encountered.");
                    ++s;
                    break;
                case '*':
                    LOG_F(INFO, "Wildcard '*' encountered.");
                    if (++p == pattern.end()) {
                        LOG_F(INFO,
                              "Trailing '*' matches the rest of the string.");
                        return true;
                    }
                    while (s != string.end()) {
                        if (fnmatch({p, pattern.end()}, {s, string.end()},
                                    flags)) {
                            return true;
                        }
                        ++s;
                    }
                    LOG_F(INFO, "No match found after '*'.");
                    return false;
                case '[': {
                    LOG_F(INFO, "Character class '[' encountered.");
                    if (++p == pattern.end()) {
                        LOG_F(ERROR, "Unclosed '[' in pattern.");
                        throw FmmatchException("Unclosed '[' in pattern.");
                    }
                    bool invert = false;
                    if (*p == '!') {
                        invert = true;
                        LOG_F(INFO, "Inverted character class.");
                        ++p;
                    }
                    bool matched = false;
                    char last_char = 0;
                    while (p != pattern.end() && *p != ']') {
                        if (*p == '-' && last_char != 0 &&
                            p + 1 != pattern.end() && *(p + 1) != ']') {
                            ++p;
                            if (*s >= last_char && *s <= *p) {
                                matched = true;
                                LOG_F(INFO, "Range match: {}-{}", last_char,
                                      *p);
                                break;
                            }
                        } else {
                            if (*s == *p) {
                                matched = true;
                                LOG_F(INFO, "Exact character match: {}", *p);
                                break;
                            }
                            last_char = *p;
                        }
                        ++p;
                    }
                    if (p == pattern.end()) {
                        LOG_F(ERROR, "Unclosed '[' in pattern.");
                        throw FmmatchException("Unclosed '[' in pattern.");
                    }
                    if (invert) {
                        matched = !matched;
                        LOG_F(INFO, "Inversion applied to match result.");
                    }
                    if (!matched) {
                        LOG_F(INFO, "Character class did not match.");
                        return false;
                    }
                    ++s;
                    break;
                }
                case '\\':
                    LOG_F(INFO, "Escape character '\\' encountered.");
                    if (!(flags & FNM_NOESCAPE)) {
                        if (++p == pattern.end()) {
                            LOG_F(ERROR,
                                  "Escape character '\\' at end of pattern.");
                            throw FmmatchException(
                                "Escape character '\\' at end of pattern.");
                        }
                    }
                    [[fallthrough]];
                default:
                    if ((flags & FNM_CASEFOLD)
                            ? (std::tolower(*p) != std::tolower(*s))
                            : (*p != *s)) {
                        LOG_F(INFO,
                              "Literal character mismatch: pattern '{}' vs "
                              "string '{}'",
                              *p, *s);
                        return false;
                    }
                    ++s;
                    break;
            }
            ++p;
        }

        if (p == pattern.end() && s == string.end()) {
            LOG_F(INFO, "Full match achieved.");
            return true;
        }
        if (p != pattern.end() && *p == '*') {
            ++p;
            LOG_F(INFO, "Trailing '*' allows remaining characters to match.");
        }
        bool result = p == pattern.end() && s == string.end();
        LOG_F(INFO, "Match result: {}", result ? "True" : "False");
        return result;
#else
        LOG_F(INFO, "Using system fnmatch.");
        int ret = ::fnmatch(pattern.data(), string.data(), flags);
        bool result = (ret == 0);
        LOG_F(INFO, "System fnmatch result: {}", result ? "True" : "False");
        return result;
#endif
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in fnmatch: {}", e.what());
        throw;  // Rethrow the exception after logging
    }
}

auto filter(const std::vector<std::string>& names, std::string_view pattern,
            int flags) -> bool {
    LOG_F(INFO, "Filter called with pattern: {} and {} names.", pattern,
          names.size());
    try {
        return std::ranges::any_of(names, [&](const std::string& name) {
            bool match = fnmatch(pattern, name, flags);
            LOG_F(INFO, "Checking if \"{}\" matches pattern \"{}\": {}", name,
                  pattern, match ? "Yes" : "No");
            return match;
        });
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in filter: {}", e.what());
        throw;
    }
}

auto filter(const std::vector<std::string>& names,
            const std::vector<std::string>& patterns,
            int flags) -> std::vector<std::string> {
    LOG_F(INFO,
          "Filter called with multiple patterns: {} patterns and {} names.",
          patterns.size(), names.size());
    std::vector<std::string> result;
    try {
        for (const auto& name : names) {
            bool matched =
                std::ranges::any_of(patterns, [&](std::string_view pattern) {
                    bool match = fnmatch(pattern, name, flags);
                    LOG_F(INFO, "Checking if \"{}\" matches pattern \"{}\": {}",
                          name, pattern, match ? "Yes" : "No");
                    return match;
                });
            if (matched) {
                LOG_F(INFO, "Name \"{}\" matches at least one pattern.", name);
                result.push_back(name);
            }
        }
        LOG_F(INFO, "Filter result contains {} matched names.", result.size());
        return result;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in multiple patterns filter: {}", e.what());
        throw;
    }
}

auto translate(std::string_view pattern, std::string& result,
               int flags) -> bool {
    LOG_F(INFO, "Translating pattern: {} with flags: {}", pattern, flags);
    result.clear();
    try {
        for (auto it = pattern.begin(); it != pattern.end(); ++it) {
            switch (*it) {
                case '*':
                    LOG_F(INFO, "Translating '*' to '.*'");
                    result += ".*";
                    break;
                case '?':
                    LOG_F(INFO, "Translating '?' to '.'");
                    result += '.';
                    break;
                case '[': {
                    LOG_F(INFO, "Translating '[' to '['");
                    result += '[';
                    if (++it == pattern.end()) {
                        LOG_F(ERROR,
                              "Unclosed '[' in pattern during translation.");
                        throw FnmatchException(
                            "Unclosed '[' in pattern during translation.");
                    }
                    if (*it == '!') {
                        LOG_F(INFO,
                              "Inverted character class during translation.");
                        result += '^';
                        ++it;
                    }
                    if (it == pattern.end()) {
                        LOG_F(ERROR,
                              "Unclosed '[' in pattern during translation.");
                        throw FnmatchException(
                            "Unclosed '[' in pattern during translation.");
                    }
                    char lastChar = *it;
                    result += *it;
                    while (++it != pattern.end() && *it != ']') {
                        if (*it == '-' && it + 1 != pattern.end() &&
                            *(it + 1) != ']') {
                            LOG_F(INFO,
                                  "Translating range in character class.");
                            result += *it;
                            result += *(++it);
                            lastChar = *it;
                        } else {
                            result += *it;
                            lastChar = *it;
                        }
                    }
                    if (it == pattern.end()) {
                        LOG_F(ERROR,
                              "Unclosed '[' in pattern during translation.");
                        throw FnmatchException(
                            "Unclosed '[' in pattern during translation.");
                    }
                    result += ']';
                    break;
                }
                case '\\':
                    LOG_F(INFO, "Translating escape character '\\' to '\\\\'");
                    if ((flags & FNM_NOESCAPE) == 0) {
                        if (++it == pattern.end()) {
                            LOG_F(ERROR,
                                  "Escape character '\\' at end of pattern "
                                  "during translation.");
                            throw FnmatchException(
                                "Escape character '\\' at end of pattern "
                                "during translation.");
                        }
                    }
                    [[fallthrough]];
                default:
                    if (((flags & FNM_CASEFOLD) != 0) &&
                        (std::isalpha(*it) != 0)) {
                        LOG_F(INFO,
                              "Translating alphabetic character with case "
                              "folding: {}",
                              *it);
                        result += '[';
                        result += static_cast<char>(std::tolower(*it));
                        result += static_cast<char>(std::toupper(*it));
                        result += ']';
                    } else {
                        LOG_F(INFO, "Translating literal character: {}", *it);
                        result += *it;
                    }
                    break;
            }
        }
        LOG_F(INFO, "Translation successful. Resulting regex: {}", result);
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception in translate: {}", e.what());
        throw;
    }
}

}  // namespace atom::algorithm