#ifndef ATOM_COMPONENTS_PACKAGE_HPP
#define ATOM_COMPONENTS_PACKAGE_HPP

#include <array>
#include <stdexcept>
#include <string_view>
#include <optional>
#include <span>
#include <ranges>
#include <expected>

// Enum for JSON value types
enum class JsonValueType { STRING, OBJECT, ARRAY, NUMBER, BOOLEAN, UNKNOWN };

// Structure for JSON key-value pair
struct JsonKeyValue {
    std::string_view key;
    std::string_view value;
    JsonValueType type;
};

// Helper function to compare strings
constexpr bool equals(std::string_view str1, std::string_view str2) {
    return str1 == str2;
}

// Trim leading and trailing spaces
constexpr std::string_view trim(std::string_view str) {
    auto is_space = [](char c) { return c == ' ' || c == '\n' || c == '\t'; };
    str.remove_prefix(std::ranges::find_if_not(str, is_space) - str.begin());
    str.remove_suffix(std::ranges::find_if_not(str | std::views::reverse, is_space).base() - str.end());
    return str;
}

// Remove ']' from a string
constexpr std::string_view remove_brackets(std::string_view str) {
    size_t pos = 0;
    while ((pos = str.find(']')) != std::string_view::npos) {
        str.remove_suffix(str.size() - pos);
    }
    while ((pos = str.find('{')) != std::string_view::npos ||
           (pos = str.find('}')) != std::string_view::npos) {
        str.remove_suffix(str.size() - pos);
    }
    return str;
}

// Parse key-value pair from a JSON line
constexpr std::expected<JsonKeyValue, std::string> parse_key_value(std::string_view json_line) {
    auto colon_pos = json_line.find(':');
    if (colon_pos == std::string_view::npos) {
        return std::unexpected("Invalid JSON line: no colon found");
    }

    std::string_view key = trim(json_line.substr(0, colon_pos));
    std::string_view value = trim(json_line.substr(colon_pos + 1));

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
    } else if (std::ranges::all_of(value, ::isdigit)) {
        type = JsonValueType::NUMBER;
    } else {
        type = JsonValueType::UNKNOWN;
    }

    return JsonKeyValue{key, value, type};
}

// Parse a JSON array
constexpr std::expected<std::array<std::string_view, 10>, std::string> parse_array(std::string_view array_str) {
    std::array<std::string_view, 10> result = {};
    size_t current_pos = 0;
    size_t element_index = 0;

    // Remove square brackets
    array_str.remove_prefix(1);
    array_str.remove_suffix(1);

    while (current_pos < array_str.size() && element_index < result.size()) {
        size_t next_comma_pos = array_str.find(',', current_pos);
        if (next_comma_pos == std::string_view::npos) {
            next_comma_pos = array_str.size();
        }

        std::string_view element = trim(array_str.substr(current_pos, next_comma_pos - current_pos));
        if (element.front() == '"' && element.back() == '"') {
            element.remove_prefix(1);
            element.remove_suffix(1);
        }
        result[element_index++] = element;

        current_pos = next_comma_pos + 1;
    }

    return result;
}

// Parse a JSON object
constexpr std::expected<std::array<JsonKeyValue, 10>, std::string> parse_object(std::string_view object_str) {
    std::array<JsonKeyValue, 10> result = {};
    size_t current_pos = 0;
    size_t line_index = 0;

    // Remove curly braces
    object_str.remove_prefix(1);
    object_str.remove_suffix(1);

    while (current_pos < object_str.size() && line_index < result.size()) {
        size_t next_comma_pos = object_str.find(',', current_pos);
        if (next_comma_pos == std::string_view::npos) {
            next_comma_pos = object_str.size();
        }

        std::string_view line = trim(object_str.substr(current_pos, next_comma_pos - current_pos));
        if (!line.empty() && line.find(':') != std::string_view::npos) {
            auto kv = parse_key_value(line);
            if (!kv) {
                return std::unexpected(kv.error());
            }
            result[line_index++] = kv.value();
        }

        current_pos = next_comma_pos + 1;
    }

    return result;
}

// Parse the entire JSON document
constexpr std::expected<std::array<JsonKeyValue, 10>, std::string> parse_json(std::string_view json) {
    std::array<JsonKeyValue, 10> result = {};
    size_t current_pos = 0;
    size_t line_index = 0;

    while (current_pos < json.size() && line_index < result.size()) {
        size_t next_line_pos = json.find('\n', current_pos);
        if (next_line_pos == std::string_view::npos) {
            next_line_pos = json.size();
        }

        std::string_view line = trim(json.substr(current_pos, next_line_pos - current_pos));
        if (!line.empty() && line.find(':') != std::string_view::npos) {
            auto kv = parse_key_value(line);
            if (!kv) {
                return std::unexpected(kv.error());
            }

            // If it's an array, parse it
            if (kv->type == JsonValueType::ARRAY) {
                auto array_values = parse_array(kv->value);
                if (!array_values) {
                    return std::unexpected(array_values.error());
                }
                // Handle array values if needed
            }
            // If it's an object, parse it
            else if (kv->type == JsonValueType::OBJECT) {
                auto object_values = parse_object(kv->value);
                if (!object_values) {
                    return std::unexpected(object_values.error());
                }
                // Handle object values if needed
            }

            result[line_index++] = kv.value();
        }

        current_pos = next_line_pos + 1;
    }

    return result;
}

// Split array elements and get internal values
constexpr void split_array_elements(const std::span<const std::string_view>& array_elements) {
    for (const auto& elem : array_elements) {
        if (!elem.empty()) {
            std::cout << "  Array Element: " << elem << '\n';
        }
    }
}

#endif // ATOM_COMPONENTS_PACKAGE_HPP
