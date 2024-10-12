# RegexWrapper Class Documentation

## Overview

The `RegexWrapper` class is a C++ wrapper around Boost.Regex, providing a convenient interface for various regex operations. It is defined in the `atom::extra::boost` namespace and offers methods for matching, searching, replacing, and splitting strings using regular expressions.

## Table of Contents

1. [Constructor](#constructor)
2. [Basic Operations](#basic-operations)
   - [match](#match)
   - [search](#search)
   - [searchAll](#searchall)
   - [replace](#replace)
   - [split](#split)
3. [Advanced Operations](#advanced-operations)
   - [matchGroups](#matchgroups)
   - [forEachMatch](#foreachmatch)
   - [namedCaptures](#namedcaptures)
   - [replaceCallback](#replacecallback)
4. [Utility Methods](#utility-methods)
   - [getPattern](#getpattern)
   - [setPattern](#setpattern)
   - [isValid](#isvalid)
   - [escapeString](#escapestring)
   - [benchmarkMatch](#benchmarkmatch)
   - [isValidRegex](#isvalidregex)

## Constructor

```cpp
explicit RegexWrapper(std::string_view pattern,
                      ::boost::regex_constants::syntax_option_type flags =
                          ::boost::regex_constants::normal)
```

Creates a `RegexWrapper` object with the given regex pattern and optional flags.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
atom::extra::boost::RegexWrapper regex_i("\\w+", boost::regex_constants::icase);
```

## Basic Operations

### match

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto match(const T& str) const -> bool
```

Matches the entire input string against the regex pattern.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
bool result = regex.match("12345");  // true
bool result2 = regex.match("abc");   // false
```

### search

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto search(const T& str) const -> std::optional<std::string>
```

Searches for the first occurrence of the regex pattern in the input string.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
auto result = regex.search("abc123def");  // Optional containing "123"
auto result2 = regex.search("abcdef");    // std::nullopt
```

### searchAll

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto searchAll(const T& str) const -> std::vector<std::string>
```

Searches for all occurrences of the regex pattern in the input string.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
auto results = regex.searchAll("abc123def456ghi");  // Vector containing {"123", "456"}
```

### replace

```cpp
template <typename T, typename U>
    requires std::convertible_to<T, std::string_view> &&
                 std::convertible_to<U, std::string_view>
auto replace(const T& str, const U& replacement) const -> std::string
```

Replaces all occurrences of the regex pattern in the input string with the replacement string.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
std::string result = regex.replace("abc123def456", "X");  // "abcXdefX"
```

### split

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto split(const T& str) const -> std::vector<std::string>
```

Splits the input string by the regex pattern.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex(",");
auto parts = regex.split("a,b,c,d");  // Vector containing {"a", "b", "c", "d"}
```

## Advanced Operations

### matchGroups

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto matchGroups(const T& str) const
    -> std::vector<std::pair<std::string, std::vector<std::string>>>
```

Matches the input string and returns the groups of each match.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("(\\w+)=(\\d+)");
auto groups = regex.matchGroups("key1=123,key2=456");
// Result: {{"key1=123", {"key1", "123"}}, {"key2=456", {"key2", "456"}}}
```

### forEachMatch

```cpp
template <typename T, typename Func>
    requires std::convertible_to<T, std::string_view> &&
             std::invocable<Func, const ::boost::smatch&>
void forEachMatch(const T& str, Func&& func) const
```

Applies a function to each match of the regex pattern in the input string.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
regex.forEachMatch("a1b2c3", [](const boost::smatch& match) {
    std::cout << "Found: " << match.str() << std::endl;
});
// Output:
// Found: 1
// Found: 2
// Found: 3
```

### namedCaptures

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto namedCaptures(const T& str) const
    -> std::map<std::string, std::string>
```

Matches the input string and returns the named captures.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("(?<key>\\w+)=(?<value>\\d+)");
auto captures = regex.namedCaptures("name=John");
// Result: {"1": "name", "2": "John"}
```

### replaceCallback

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto replaceCallback(
    const T& str,
    const std::function<std::string(const ::boost::smatch&)>& callback)
    const -> std::string
```

Replaces all matches of the regex pattern in the input string using a callback function.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
std::string result = regex.replaceCallback("a1b2c3", [](const boost::smatch& match) {
    int value = std::stoi(match.str());
    return std::to_string(value * 2);
});
// Result: "a2b4c6"
```

## Utility Methods

### getPattern

```cpp
[[nodiscard]] auto getPattern() const -> std::string
```

Gets the regex pattern as a string.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
std::string pattern = regex.getPattern();  // "\d+"
```

### setPattern

```cpp
void setPattern(std::string_view pattern,
                ::boost::regex_constants::syntax_option_type flags =
                    ::boost::regex_constants::normal)
```

Sets a new regex pattern with optional flags.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
regex.setPattern("\\w+", boost::regex_constants::icase);
```

### isValid

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto isValid(const T& str) const -> bool
```

Checks if the given string is a valid match for the regex pattern.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d{3}-\\d{2}-\\d{4}");
bool valid1 = regex.isValid("123-45-6789");  // true
bool valid2 = regex.isValid("12-345-6789");  // false
```

### escapeString

```cpp
[[nodiscard]] static auto escapeString(const std::string& str)
    -> std::string
```

Escapes special characters in the given string for use in a regex pattern.

**Example:**

```cpp
std::string escaped = atom::extra::boost::RegexWrapper::escapeString("1+1=2");
// Result: "1\\+1\\=2"
```

### benchmarkMatch

```cpp
template <typename T>
    requires std::convertible_to<T, std::string_view>
auto benchmarkMatch(const T& str, int iterations = 1000) const
    -> std::chrono::nanoseconds
```

Benchmarks the match operation for the given string over a number of iterations.

**Example:**

```cpp
atom::extra::boost::RegexWrapper regex("\\d+");
auto avg_time = regex.benchmarkMatch("12345", 10000);
std::cout << "Average match time: " << avg_time.count() << " ns" << std::endl;
```

### isValidRegex

```cpp
static auto isValidRegex(const std::string& pattern) -> bool
```

Checks if the given regex pattern is valid.

**Example:**

```cpp
bool valid1 = atom::extra::boost::RegexWrapper::isValidRegex("\\d+");    // true
bool valid2 = atom::extra::boost::RegexWrapper::isValidRegex("\\d+[");   // false
```

## Complete Usage Example

Here's a comprehensive example demonstrating various features of the `RegexWrapper` class:

```cpp
#include "atom_extra_boost_regex.hpp"
#include <iostream>
#include <string>

int main() {
    // Create a RegexWrapper object
    atom::extra::boost::RegexWrapper regex("(\\w+)=(\\d+)");

    // Basic operations
    std::string input = "key1=123,key2=456,key3=789";

    // Match
    bool full_match = regex.match("name=John");
    std::cout << "Full match: " << (full_match ? "true" : "false") << std::endl;

    // Search
    auto search_result = regex.search(input);
    if (search_result) {
        std::cout << "First match: " << *search_result << std::endl;
    }

    // SearchAll
    auto all_matches = regex.searchAll(input);
    std::cout << "All matches:" << std::endl;
    for (const auto& match : all_matches) {
        std::cout << "  " << match << std::endl;
    }

    // Replace
    std::string replaced = regex.replace(input, "[$1:$2]");
    std::cout << "Replaced: " << replaced << std::endl;

    // Split
    atom::extra::boost::RegexWrapper split_regex(",");
    auto parts = split_regex.split(input);
    std::cout << "Split parts:" << std::endl;
    for (const auto& part : parts) {
        std::cout << "  " << part << std::endl;
    }

    // Advanced operations
    // MatchGroups
    auto groups = regex.matchGroups(input);
    std::cout << "Match groups:" << std::endl;
    for (const auto& [full_match, group_matches] : groups) {
        std::cout << "  Full match: " << full_match << std::endl;
        std::cout << "  Groups:" << std::endl;
        for (const auto& group : group_matches) {
            std::cout << "    " << group << std::endl;
        }
    }

    // ForEachMatch
    std::cout << "ForEachMatch:" << std::endl;
    regex.forEachMatch(input, [](const boost::smatch& match) {
        std::cout << "  Match: " << match[0] << ", Key: " << match[1] << ", Value: " << match[2] << std::endl;
    });

    // ReplaceCallback
    std::string callback_replaced = regex.replaceCallback(input, [](const boost::smatch& match) {
        int value = std::stoi(match[2]);
        return match[1].str() + "=" + std::to_string(value * 2);
    });
    std::cout << "Callback replaced: " << callback_replaced << std::endl;

    // Utility methods
    std::cout << "Regex pattern: " << regex.getPattern() << std::endl;

    regex.setPattern("\\d+");
    std::cout << "New pattern: " << regex.getPattern() << std::endl;

    std::cout << "Is '123' valid? " << (regex.isValid("123") ? "true" : "false") << std::endl;
    std::cout << "Is 'abc' valid? " << (regex.isValid("abc") ? "true" : "false") << std::endl;

    std::string to_escape = "1+1=2";
    std::cout << "Escaped string: " << atom::extra::boost::RegexWrapper::escapeString(to_escape) << std::endl;

    auto bench_time = regex.benchmarkMatch("12345", 10000);
    std::cout << "Average match time: " << bench_time.count() << " ns" << std::endl;

    std::cout << "Is '\\d+' a valid regex? " << (atom::extra::boost::RegexWrapper::isValidRegex("\\d+") ? "true" : "false") << std::endl;
    std::cout << "Is '\\d+[' a valid regex? " << (atom::extra::boost::RegexWrapper::isValidRegex("\\d+[") ? "true" : "false") << std::endl;

    return 0;
}
```

This example demonstrates how to use all the methods provided by the `RegexWrapper` class. It covers basic operations like matching, searching, and replacing, as well as more advanced features like working with match groups and using callback functions for replacements. It also shows how to use the utility methods for tasks like escaping strings, benchmarking, and validating regex patterns.

## Conclusion

The `RegexWrapper` class provides a powerful and convenient interface for working with regular expressions in C++. It encapsulates the functionality of Boost.Regex while offering additional utility methods and a more modern C++ interface. This wrapper is particularly useful for projects that require extensive use of regular expressions, as it simplifies common operations and provides a consistent API for regex-related tasks.
