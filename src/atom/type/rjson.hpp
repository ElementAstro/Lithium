#ifndef ATOM_TYPE_RJSON_HPP
#define ATOM_TYPE_RJSON_HPP

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace atom::type {

/**
 * @brief Forward declaration of the JsonValue class.
 */
class JsonValue;

/**
 * @brief Alias for a map of strings to JsonValue representing a JSON object.
 */
using JsonObject = std::unordered_map<std::string, JsonValue>;

/**
 * @brief Alias for a vector of JsonValue representing a JSON array.
 */
using JsonArray = std::vector<JsonValue>;

/**
 * @brief Represents a value in a JSON document.
 *
 * This class can represent various JSON value types including null, string,
 * number, boolean, object (map), and array. It provides methods to access the
 * value in its appropriate type and convert it to a string representation.
 */
class JsonValue {
public:
    /**
     * @brief Enumeration of JSON value types.
     */
    enum class Type { Null, String, Number, Bool, Object, Array };

    /**
     * @brief Default constructor initializing a null value.
     */
    JsonValue();

    /**
     * @brief Constructs a JsonValue from a string.
     * @param value The string value.
     */
    explicit JsonValue(const std::string& value);

    /**
     * @brief Constructs a JsonValue from a number.
     * @param value The numeric value.
     */
    explicit JsonValue(double value);

    /**
     * @brief Constructs a JsonValue from a boolean.
     * @param value The boolean value.
     */
    explicit JsonValue(bool value);

    /**
     * @brief Constructs a JsonValue from a JSON object.
     * @param value The JSON object (map).
     */
    explicit JsonValue(const JsonObject& value);

    /**
     * @brief Constructs a JsonValue from a JSON array.
     * @param value The JSON array (vector).
     */
    explicit JsonValue(const JsonArray& value);

    /**
     * @brief Gets the type of the JSON value.
     * @return The type of the JSON value.
     */
    [[nodiscard]] auto type() const -> Type;

    /**
     * @brief Gets the value as a string.
     * @return The string value.
     * @throws std::invalid_argument if the value is not a string.
     */
    [[nodiscard]] auto as_string() const -> const std::string&;

    /**
     * @brief Gets the value as a number.
     * @return The numeric value.
     * @throws std::invalid_argument if the value is not a number.
     */
    [[nodiscard]] auto as_number() const -> double;

    /**
     * @brief Gets the value as a boolean.
     * @return The boolean value.
     * @throws std::invalid_argument if the value is not a boolean.
     */
    [[nodiscard]] auto as_bool() const -> bool;

    /**
     * @brief Gets the value as a JSON object.
     * @return The JSON object (map).
     * @throws std::invalid_argument if the value is not an object.
     */
    [[nodiscard]] auto as_object() const -> const JsonObject&;

    /**
     * @brief Gets the value as a JSON array.
     * @return The JSON array (vector).
     * @throws std::invalid_argument if the value is not an array.
     */
    [[nodiscard]] auto as_array() const -> const JsonArray&;

    /**
     * @brief Converts the JSON value to its string representation.
     * @return The string representation of the JSON value.
     */
    [[nodiscard]] auto to_string() const -> std::string;

    /**
     * @brief Accesses a value in a JSON object by key.
     * @param key The key in the JSON object.
     * @return The value associated with the key.
     * @throws std::invalid_argument if the value is not an object or the key is
     * not present.
     */
    auto operator[](const std::string& key) const -> const JsonValue&;

    /**
     * @brief Accesses a value in a JSON array by index.
     * @param index The index in the JSON array.
     * @return The value at the specified index.
     * @throws std::invalid_argument if the value is not an array or the index
     * is out of range.
     */
    auto operator[](size_t index) const -> const JsonValue&;

private:
    Type type_;  ///< The type of the JSON value.
    std::variant<std::nullptr_t, std::string, double, bool, JsonObject,
                 JsonArray>
        value_;  ///< The actual value stored.
};

/**
 * @brief Parses a JSON document from a string.
 *
 * This class provides methods to parse a JSON document represented as a string
 * and convert it into a `JsonValue` object that reflects the structure and
 * values of the JSON document.
 */
class JsonParser {
public:
    /**
     * @brief Parses a JSON string into a JsonValue.
     * @param str The JSON string.
     * @return The parsed JsonValue object.
     */
    static auto parse(const std::string& str) -> JsonValue;

private:
    /**
     * @brief Parses a JSON value from a string starting at the given index.
     * @param str The JSON string.
     * @param index The current position in the string.
     * @return The parsed JsonValue.
     */
    static auto parse_value(const std::string& str, size_t& index) -> JsonValue;

    /**
     * @brief Parses a JSON string enclosed in quotes.
     * @param str The JSON string.
     * @param index The current position in the string.
     * @return The parsed string value.
     */
    static auto parse_string(const std::string& str,
                             size_t& index) -> std::string;

    /**
     * @brief Parses a JSON number from a string.
     * @param str The JSON string.
     * @param index The current position in the string.
     * @return The parsed numeric value.
     */
    static auto parse_number(const std::string& str, size_t& index) -> double;

    /**
     * @brief Parses a JSON boolean value from a string.
     * @param str The JSON string.
     * @param index The current position in the string.
     * @return The parsed boolean value.
     */
    static auto parse_bool(const std::string& str, size_t& index) -> bool;

    /**
     * @brief Parses a JSON null value from a string.
     * @param str The JSON string.
     * @param index The current position in the string.
     */
    static void parse_null(const std::string& str, size_t& index);

    /**
     * @brief Parses a JSON object (map) from a string.
     * @param str The JSON string.
     * @param index The current position in the string.
     * @return The parsed JSON object.
     */
    static auto parse_object(const std::string& str,
                             size_t& index) -> JsonObject;

    /**
     * @brief Parses a JSON array from a string.
     * @param str The JSON string.
     * @param index The current position in the string.
     * @return The parsed JSON array.
     */
    static auto parse_array(const std::string& str, size_t& index) -> JsonArray;

    /**
     * @brief Skips whitespace characters in the string.
     * @param str The JSON string.
     * @param index The current position in the string.
     */
    static void skip_whitespace(const std::string& str, size_t& index);
};

}  // namespace atom::type

#endif  // ATOM_TYPE_RJSON_HPP
