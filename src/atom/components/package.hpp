#ifndef ATOM_COMPONENTS_PACKAGE_HPP
#define ATOM_COMPONENTS_PACKAGE_HPP

#include <absl/strings/match.h>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string_view>

// Constants
constexpr size_t ALIGNMENT = 64;
constexpr size_t MAX_ELEMENTS = 10;

// Enum for JSON value types
enum class JsonValueType { STRING, OBJECT, ARRAY, NUMBER, BOOLEAN, UNKNOWN };

// Structure for JSON key-value pair
struct alignas(ALIGNMENT) JsonKeyValue {
    std::string_view key;
    std::string_view value;
    JsonValueType type;
};

// Helper function to compare strings
constexpr auto Equals(std::string_view str1, std::string_view str2) -> bool {
    return str1 == str2;
}

// Trim leading and trailing spaces
constexpr auto Trim(std::string_view str) -> std::string_view {
    auto isSpace = [](char character) {
        return character == ' ' || character == '\n' || character == '\t';
    };
    str.remove_prefix(std::ranges::find_if_not(str, isSpace) - str.begin());
    str.remove_suffix(
        std::ranges::find_if_not(str | std::views::reverse, isSpace).base() -
        str.end());
    return str;
}

// Remove ']' from a string
constexpr auto RemoveBrackets(std::string_view str) -> std::string_view {
    size_t position = 0;
    while ((position = str.find(']')) != std::string_view::npos) {
        str.remove_suffix(str.size() - position);
    }
    while ((position = str.find('{')) != std::string_view::npos ||
           (position = str.find('}')) != std::string_view::npos) {
        str.remove_suffix(str.size() - position);
    }
    return str;
}

// Parse key-value pair from a JSON line
constexpr auto ParseKeyValue(std::string_view jsonLine)
    -> std::pair<JsonKeyValue, std::string> {
    auto colonPosition = jsonLine.find(':');
    if (colonPosition == std::string_view::npos) {
        return {JsonKeyValue{}, "Invalid JSON line: no colon found"};
    }

    std::string_view key = Trim(jsonLine.substr(0, colonPosition));
    std::string_view value = Trim(jsonLine.substr(colonPosition + 1));

    // Remove quotes from key
    if (key.front() == '"' && key.back() == '"') {
        key.remove_prefix(1);
        key.remove_suffix(1);
    }

    JsonValueType type = JsonValueType::STRING;

    // Detect value type (OBJECT, ARRAY, STRING, NUMBER, BOOLEAN)
    if (value.front() == '{' && value.back() == '}') {
        type = JsonValueType::OBJECT;
    } else if (value.front() == '[' && value.back() == ']') {
        type = JsonValueType::ARRAY;
    } else if (value.front() == '"' && value.back() == '"') {
        value.remove_prefix(1);
        value.remove_suffix(1);
    } else if (value == "true" || value == "false") {
        type = JsonValueType::BOOLEAN;
    } else if (std::all_of(value.begin(), value.end(), ::isdigit)) {
        type = JsonValueType::NUMBER;
    } else {
        type = JsonValueType::UNKNOWN;
    }

    return {JsonKeyValue{key, value, type}, ""};
}

// Parse a JSON array
constexpr auto ParseArray(std::string_view arrayString)
    -> std::pair<std::array<std::string_view, MAX_ELEMENTS>, std::string> {
    std::array<std::string_view, MAX_ELEMENTS> result = {};
    size_t currentPosition = 0;
    size_t elementIndex = 0;

    // Remove square brackets
    arrayString.remove_prefix(1);
    arrayString.remove_suffix(1);

    while (currentPosition < arrayString.size() &&
           elementIndex < result.size()) {
        size_t nextCommaPosition = arrayString.find(',', currentPosition);
        if (nextCommaPosition == std::string_view::npos) {
            nextCommaPosition = arrayString.size();
        }

        std::string_view element = Trim(arrayString.substr(
            currentPosition, nextCommaPosition - currentPosition));
        if (element.front() == '"' && element.back() == '"') {
            element.remove_prefix(1);
            element.remove_suffix(1);
        }
        result[elementIndex++] = element;

        currentPosition = nextCommaPosition + 1;
    }

    return {result, ""};
}

// Parse a JSON object
constexpr auto ParseObject(std::string_view objectString)
    -> std::pair<std::array<JsonKeyValue, MAX_ELEMENTS>, std::string> {
    std::array<JsonKeyValue, MAX_ELEMENTS> result = {};
    size_t currentPosition = 0;
    size_t lineIndex = 0;

    // Remove curly braces
    objectString.remove_prefix(1);
    objectString.remove_suffix(1);

    while (currentPosition < objectString.size() && lineIndex < result.size()) {
        size_t nextCommaPosition = objectString.find(',', currentPosition);
        if (nextCommaPosition == std::string_view::npos) {
            nextCommaPosition = objectString.size();
        }

        std::string_view line = Trim(objectString.substr(
            currentPosition, nextCommaPosition - currentPosition));
        if (!line.empty() && absl::StrContains(line, ':')) {
            auto [kv, error] = ParseKeyValue(line);
            if (!error.empty()) {
                return {result, error};
            }
            result[lineIndex++] = kv;
        }

        currentPosition = nextCommaPosition + 1;
    }

    return {result, ""};
}

// Parse the entire JSON document
constexpr auto ParseJson(std::string_view json)
    -> std::pair<std::array<JsonKeyValue, MAX_ELEMENTS>, std::string> {
    std::array<JsonKeyValue, MAX_ELEMENTS> result = {};
    size_t currentPosition = 0;
    size_t lineIndex = 0;

    while (currentPosition < json.size() && lineIndex < result.size()) {
        size_t nextLinePosition = json.find('\n', currentPosition);
        if (nextLinePosition == std::string_view::npos) {
            nextLinePosition = json.size();
        }

        std::string_view line = Trim(
            json.substr(currentPosition, nextLinePosition - currentPosition));
        if (!line.empty() && absl::StrContains(line, ':')) {
            auto [kv, error] = ParseKeyValue(line);
            if (!error.empty()) {
                return {result, error};
            }

            // If it's an array, parse it
            if (kv.type == JsonValueType::ARRAY) {
                auto [arrayValues, arrayError] = ParseArray(kv.value);
                if (!arrayError.empty()) {
                    return {result, arrayError};
                }
                // Handle array values if needed
            }
            // If it's an object, parse it
            else if (kv.type == JsonValueType::OBJECT) {
                auto [objectValues, objectError] = ParseObject(kv.value);
                if (!objectError.empty()) {
                    return {result, objectError};
                }
                // Handle object values if needed
            }

            result[lineIndex++] = kv;
        }

        currentPosition = nextLinePosition + 1;
    }

    return {result, ""};
}

// Split array elements and get internal values
constexpr void SplitArrayElements(
    const std::span<const std::string_view>& arrayElements) {
    for (const auto& element : arrayElements) {
        if (!element.empty()) {
            std::cout << "  Array Element: " << element << '\n';
        }
    }
}

#endif  // ATOM_COMPONENTS_PACKAGE_HPP
