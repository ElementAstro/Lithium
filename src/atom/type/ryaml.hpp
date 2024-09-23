#ifndef ATOM_TYPE_RYAML_HPP
#define ATOM_TYPE_RYAML_HPP

#include <regex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "atom/error/exception.hpp"

namespace atom::type {

/**
 * @brief Forward declaration of YamlValue class.
 */
class YamlValue;

/**
 * @brief Alias for a map of string to YamlValue representing a YAML object.
 */
using YamlObject = std::unordered_map<std::string, YamlValue>;

/**
 * @brief Alias for a vector of YamlValue representing a YAML array.
 */
using YamlArray = std::vector<YamlValue>;

/**
 * @brief Represents a value in a YAML document.
 *
 * This class can represent various YAML value types including null, string,
 * number, boolean, object (map), and array. It provides methods to access the
 * value in its appropriate type and convert it to a string representation.
 */
class YamlValue {
public:
    /**
     * @brief Enumeration of YAML value types.
     */
    enum class Type { Null, String, Number, Bool, Object, Array, Alias };

    /**
     * @brief Default constructor initializing a null value.
     */
    YamlValue();

    /**
     * @brief Constructs a YamlValue from a string.
     * @param value The string value.
     */
    explicit YamlValue(const std::string& value);

    /**
     * @brief Constructs a YamlValue from a number.
     * @param value The numeric value.
     */
    explicit YamlValue(double value);

    /**
     * @brief Constructs a YamlValue from a boolean.
     * @param value The boolean value.
     */
    explicit YamlValue(bool value);

    /**
     * @brief Constructs a YamlValue from a YAML object.
     * @param value The YAML object (map).
     */
    explicit YamlValue(const YamlObject& value);

    /**
     * @brief Constructs a YamlValue from a YAML array.
     * @param value The YAML array (vector).
     */
    explicit YamlValue(const YamlArray& value);

    /**
     * @brief Constructs a YamlValue from an alias.
     * @param alias The alias value.
     */
    explicit YamlValue(const YamlValue& alias);

    /**
     * @brief Gets the type of the YAML value.
     * @return The type of the YAML value.
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
     * @brief Gets the value as a YAML object.
     * @return The YAML object (map).
     * @throws std::invalid_argument if the value is not an object.
     */
    [[nodiscard]] auto as_object() const -> const YamlObject&;

    /**
     * @brief Gets the value as a YAML array.
     * @return The YAML array (vector).
     * @throws std::invalid_argument if the value is not an array.
     */
    [[nodiscard]] auto as_array() const -> const YamlArray&;

    /**
     * @brief Converts the YAML value to its string representation.
     * @return The string representation of the YAML value.
     */
    std::string to_string() const;

    /**
     * @brief Accesses a value in a YAML object by key.
     * @param key The key in the YAML object.
     * @return The value associated with the key.
     * @throws std::invalid_argument if the value is not an object or the key is
     * not present.
     */
    auto operator[](const std::string& key) const -> const YamlValue&;

    /**
     * @brief Accesses a value in a YAML array by index.
     * @param index The index in the YAML array.
     * @return The value at the specified index.
     * @throws std::invalid_argument if the value is not an array or the index
     * is out of range.
     */
    auto operator[](size_t index) const -> const YamlValue&;

private:
    Type type_;  ///< The type of the YAML value.
    std::variant<std::nullptr_t, std::string, double, bool, YamlObject,
                 YamlArray, YamlValue*>
        value_;  ///< The actual value stored.
};

/**
 * @brief Parses a YAML document from a string.
 *
 * This class provides methods to parse a YAML document represented as a string
 * and convert it into a `YamlValue` object that reflects the structure and
 * values of the YAML document.
 */
class YamlParser {
public:
    /**
     * @brief Parses a YAML string into a YamlValue.
     * @param str The YAML string.
     * @return The parsed YamlValue object.
     */
    static auto parse(const std::string& str) -> YamlValue;

private:
    /**
     * @brief Parses a YAML value from a string.
     * @param str The string containing the YAML value.
     * @param index The current index in the string.
     * @return The parsed YamlValue.
     */
    static auto parse_value(const std::string& str, size_t& index) -> YamlValue;

    /**
     * @brief Parses a YAML string from a string.
     * @param str The string containing the YAML string.
     * @param index The current index in the string.
     * @return The parsed string.
     */
    static auto parse_string(const std::string& str,
                             size_t& index) -> std::string;

    /**
     * @brief Parses a YAML number from a string.
     * @param str The string containing the YAML number.
     * @param index The current index in the string.
     * @return The parsed number.
     */
    static auto parse_number(const std::string& str, size_t& index) -> double;

    /**
     * @brief Parses a YAML boolean from a string.
     * @param str The string containing the YAML boolean.
     * @param index The current index in the string.
     * @return The parsed boolean.
     */
    static auto parse_bool(const std::string& str, size_t& index) -> bool;

    /**
     * @brief Parses a YAML null value from a string.
     * @param str The string containing the YAML null.
     * @param index The current index in the string.
     */
    static void parse_null(const std::string& str, size_t& index);

    /**
     * @brief Parses a key-value pair from a YAML string.
     * @param str The string containing the key-value pair.
     * @param index The current index in the string.
     * @return The parsed YamlObject.
     */
    static auto parse_key_value(const std::string& str,
                                size_t& index) -> YamlObject;

    /**
     * @brief Parses a YAML array from a string.
     * @param str The string containing the YAML array.
     * @param index The current index in the string.
     * @return The parsed YamlArray.
     */
    static auto parse_array(const std::string& str, size_t& index) -> YamlArray;

    /**
     * @brief Parses a YAML alias from a string.
     * @param str The string containing the YAML alias.
     * @param index The current index in the string.
     * @return The parsed YamlValue.
     */
    static auto parse_alias(const std::string& str, size_t& index) -> YamlValue;

    /**
     * @brief Skips whitespace characters in a string.
     * @param str The string to process.
     * @param index The current index in the string.
     */
    static void skip_whitespace(const std::string& str, size_t& index);
};

}  // namespace atom::type

#endif  // ATOM_TYPE_RYAML_HPP
