#ifndef ATOM_TYPE_RJSON_HPP
#define ATOM_TYPE_RJSON_HPP

#include <cctype>
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
 */
class JsonValue {
public:
    /**
     * @brief Enum representing the type of the JSON value.
     */
    enum class Type { Null, String, Number, Bool, Object, Array };

    /**
     * @brief Constructs a JsonValue of type Null.
     */
    JsonValue();

    /**
     * @brief Constructs a JsonValue of type String.
     * @param value The string value to initialize.
     */
    explicit JsonValue(const std::string& value);

    /**
     * @brief Constructs a JsonValue of type Number.
     * @param value The numeric value to initialize.
     */
    explicit JsonValue(double value);

    /**
     * @brief Constructs a JsonValue of type Bool.
     * @param value The boolean value to initialize.
     */
    explicit JsonValue(bool value);

    /**
     * @brief Constructs a JsonValue of type Object.
     * @param value The JsonObject to initialize.
     */
    explicit JsonValue(const JsonObject& value);

    /**
     * @brief Constructs a JsonValue of type Array.
     * @param value The JsonArray to initialize.
     */
    explicit JsonValue(const JsonArray& value);

    /**
     * @brief Gets the type of the JSON value.
     * @return The type of the JSON value.
     */
    [[nodiscard]] auto type() const -> Type;

    /**
     * @brief Gets the string value.
     * @return The string value.
     * @throws std::bad_variant_access if the value is not a string.
     */
    [[nodiscard]] auto asString() const -> const std::string&;

    /**
     * @brief Gets the numeric value.
     * @return The numeric value.
     * @throws std::bad_variant_access if the value is not a number.
     */
    [[nodiscard]] auto asNumber() const -> double;

    /**
     * @brief Gets the boolean value.
     * @return The boolean value.
     * @throws std::bad_variant_access if the value is not a boolean.
     */
    [[nodiscard]] auto asBool() const -> bool;

    /**
     * @brief Gets the object value.
     * @return The object value.
     * @throws std::bad_variant_access if the value is not an object.
     */
    [[nodiscard]] auto asObject() const -> const JsonObject&;

    /**
     * @brief Gets the array value.
     * @return The array value.
     * @throws std::bad_variant_access if the value is not an array.
     */
    [[nodiscard]] auto asArray() const -> const JsonArray&;

    /**
     * @brief Converts the JSON value to a string representation.
     * @return The string representation of the JSON value.
     */
    [[nodiscard]] auto toString() const -> std::string;

    /**
     * @brief Accesses the value associated with the given key in a JSON object.
     * @param key The key to access.
     * @return The JSON value associated with the key.
     * @throws std::bad_variant_access if the value is not an object.
     */
    auto operator[](const std::string& key) const -> const JsonValue&;

    /**
     * @brief Accesses the value at the given index in a JSON array.
     * @param index The index to access.
     * @return The JSON value at the index.
     * @throws std::bad_variant_access if the value is not an array.
     */
    auto operator[](size_t index) const -> const JsonValue&;

private:
    Type type_;  ///< The type of the JSON value.
    std::variant<std::nullptr_t, std::string, double, bool, JsonObject,
                 JsonArray>
        value_;  ///< The value of the JSON.
};

/**
 * @brief Parses a JSON document from a string.
 */
class JsonParser {
public:
    /**
     * @brief Parses a JSON document from a string.
     * @param str The string containing the JSON document.
     * @return The parsed JsonValue.
     */
    static auto parse(const std::string& str) -> JsonValue;

private:
    /**
     * @brief Parses a JSON value from a string.
     * @param str The string containing the JSON value.
     * @param index The current index in the string.
     * @return The parsed JsonValue.
     */
    static auto parseValue(const std::string& str, size_t& index) -> JsonValue;

    /**
     * @brief Parses a JSON string from a string.
     * @param str The string containing the JSON string.
     * @param index The current index in the string.
     * @return The parsed string.
     */
    static auto parseString(const std::string& str,
                            size_t& index) -> std::string;

    /**
     * @brief Parses a JSON number from a string.
     * @param str The string containing the JSON number.
     * @param index The current index in the string.
     * @return The parsed number.
     */
    static auto parseNumber(const std::string& str, size_t& index) -> double;

    /**
     * @brief Parses a JSON boolean from a string.
     * @param str The string containing the JSON boolean.
     * @param index The current index in the string.
     * @return The parsed boolean.
     */
    static auto parseBool(const std::string& str, size_t& index) -> bool;

    /**
     * @brief Parses a JSON null value from a string.
     * @param str The string containing the JSON null.
     * @param index The current index in the string.
     */
    static void parseNull(const std::string& str, size_t& index);

    /**
     * @brief Parses a JSON object from a string.
     * @param str The string containing the JSON object.
     * @param index The current index in the string.
     * @return The parsed JsonObject.
     */
    static auto parseObject(const std::string& str,
                            size_t& index) -> JsonObject;

    /**
     * @brief Parses a JSON array from a string.
     * @param str The string containing the JSON array.
     * @param index The current index in the string.
     * @return The parsed JsonArray.
     */
    static auto parseArray(const std::string& str, size_t& index) -> JsonArray;

    /**
     * @brief Skips whitespace characters in a string.
     * @param str The string to process.
     * @param index The current index in the string.
     */
    static void skipWhitespace(const std::string& str, size_t& index);

    /**
     * @brief Parses an escaped character from a string.
     * @param str The string containing the escaped character.
     * @param index The current index in the string.
     * @return The parsed character.
     */
    static auto parseEscapedChar(const std::string& str, size_t& index) -> char;
};

}  // namespace atom::type

#endif  // ATOM_TYPE_RJSON_HPP
