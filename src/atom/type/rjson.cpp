#include "rjson.hpp"
#include <cmath>
#include <sstream>
#include <stdexcept>
#include "atom/error/exception.hpp"

namespace atom::type {

// JsonValue Constructors
JsonValue::JsonValue() : type_(Type::Null), value_(nullptr) {}
JsonValue::JsonValue(const std::string& value)
    : type_(Type::String), value_(value) {}
JsonValue::JsonValue(double value) : type_(Type::Number), value_(value) {}
JsonValue::JsonValue(bool value) : type_(Type::Bool), value_(value) {}
JsonValue::JsonValue(const JsonObject& value)
    : type_(Type::Object), value_(value) {}
JsonValue::JsonValue(const JsonArray& value)
    : type_(Type::Array), value_(value) {}

// Accessors for JsonValue types
auto JsonValue::type() const -> Type { return type_; }

auto JsonValue::asString() const -> const std::string& {
    if (type_ != Type::String) {
        THROW_INVALID_ARGUMENT("Not a string");
    }
    return std::get<std::string>(value_);
}

auto JsonValue::asNumber() const -> double {
    if (type_ != Type::Number) {
        THROW_INVALID_ARGUMENT("Not a number");
    }
    return std::get<double>(value_);
}

auto JsonValue::asBool() const -> bool {
    if (type_ != Type::Bool) {
        THROW_INVALID_ARGUMENT("Not a bool");
    }
    return std::get<bool>(value_);
}

auto JsonValue::asObject() const -> const JsonObject& {
    if (type_ != Type::Object) {
        THROW_INVALID_ARGUMENT("Not an object");
    }
    return std::get<JsonObject>(value_);
}

auto JsonValue::asArray() const -> const JsonArray& {
    if (type_ != Type::Array) {
        THROW_INVALID_ARGUMENT("Not an array");
    }
    return std::get<JsonArray>(value_);
}

auto JsonValue::toString() const -> std::string {
    switch (type_) {
        case Type::Null:
            return "null";
        case Type::String:
            return "\"" + asString() + "\"";
        case Type::Number:
            return std::to_string(asNumber());
        case Type::Bool:
            return asBool() ? "true" : "false";
        case Type::Object: {
            std::string result = "{";
            const auto& obj = asObject();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                if (it != obj.begin()) {
                    result += ",";
                }
                result += "\"" + it->first + "\":" + it->second.toString();
            }
            result += "}";
            return result;
        }
        case Type::Array: {
            std::string result = "[";
            const auto& arr = asArray();
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0)
                    result += ",";
                result += arr[i].toString();
            }
            result += "]";
            return result;
        }
    }
    THROW_INVALID_ARGUMENT("Unknown type");
}

// Overloaded operators for object and array access
auto JsonValue::operator[](const std::string& key) const -> const JsonValue& {
    if (type_ != Type::Object) {
        THROW_INVALID_ARGUMENT("Not an object");
    }
    return asObject().at(key);
}

auto JsonValue::operator[](size_t index) const -> const JsonValue& {
    if (type_ != Type::Array) {
        THROW_INVALID_ARGUMENT("Not an array");
    }
    return asArray().at(index);
}

// JsonParser Implementation
auto JsonParser::parse(const std::string& str) -> JsonValue {
    size_t index = 0;
    return parseValue(str, index);
}

auto JsonParser::parseValue(const std::string& str,
                            size_t& index) -> JsonValue {
    skipWhitespace(str, index);
    if (str[index] == '"') {
        return JsonValue(parseString(str, index));
    }
    if (str[index] == 't' || str[index] == 'f') {
        return JsonValue(parseBool(str, index));
    }
    if (str[index] == 'n') {
        parseNull(str, index);
        return {};
    }
    if (str[index] == '{') {
        return JsonValue(parseObject(str, index));
    }
    if (str[index] == '[') {
        return JsonValue(parseArray(str, index));
    }
    if ((std::isdigit(str[index]) != 0) || str[index] == '-') {
        return JsonValue(parseNumber(str, index));
    }
    THROW_INVALID_ARGUMENT("Invalid JSON value");
}

auto JsonParser::parseString(const std::string& str,
                             size_t& index) -> std::string {
    ++index;  // Skip opening quote
    std::string result;
    while (str[index] != '"') {
        if (str[index] == '\\') {
            result += parseEscapedChar(str, index);
        } else {
            result += str[index++];
        }
    }
    ++index;  // Skip closing quote
    return result;
}

auto JsonParser::parseEscapedChar(const std::string& str,
                                  size_t& index) -> char {
    ++index;  // Skip backslash
    switch (str[index++]) {
        case '"':
            return '"';
        case '\\':
            return '\\';
        case '/':
            return '/';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'n':
            return '\n';
        case 'r':
            return '\r';
        case 't':
            return '\t';
        default:
            THROW_INVALID_ARGUMENT("Invalid escape sequence");
    }
}

auto JsonParser::parseNumber(const std::string& str, size_t& index) -> double {
    size_t startIndex = index;
    bool hasDecimal = false;
    bool hasExponent = false;

    if (str[index] == '-') {
        ++index;
    }

    while ((std::isdigit(str[index]) != 0) || str[index] == '.') {
        if (str[index] == '.') {
            if (hasDecimal) {
                THROW_INVALID_ARGUMENT(
                    "Invalid number format: multiple decimal points");
            }
            hasDecimal = true;
        }
        ++index;
    }

    // Check for scientific notation (e or E)
    if (str[index] == 'e' || str[index] == 'E') {
        hasExponent = true;
        ++index;
        if (str[index] == '+' || str[index] == '-') {
            ++index;
        }
        while (std::isdigit(str[index]) != 0) {
            ++index;
        }
    }

    try {
        return std::stod(str.substr(startIndex, index - startIndex));
    } catch (const std::invalid_argument&) {
        THROW_INVALID_ARGUMENT("Invalid number format");
    } catch (const std::out_of_range&) {
        THROW_INVALID_ARGUMENT("Number out of range");
    }
}

auto JsonParser::parseBool(const std::string& str, size_t& index) -> bool {
    if (str.substr(index, 4) == "true") {
        index += 4;
        return true;
    }
    if (str.substr(index, 5) == "false") {
        index += 5;
        return false;
    }
    THROW_INVALID_ARGUMENT("Invalid boolean value");
}

void JsonParser::parseNull(const std::string& str, size_t& index) {
    if (str.substr(index, 4) == "null") {
        index += 4;
    } else {
        THROW_INVALID_ARGUMENT("Invalid null value");
    }
}

auto JsonParser::parseObject(const std::string& str,
                             size_t& index) -> JsonObject {
    ++index;  // Skip opening '{'
    JsonObject obj;
    skipWhitespace(str, index);

    while (str[index] != '}') {
        if (str[index] != '"') {
            THROW_INVALID_ARGUMENT("Expected string key in JSON object");
        }
        std::string key = parseString(str, index);
        skipWhitespace(str, index);

        if (str[index] != ':') {
            THROW_INVALID_ARGUMENT("Expected ':' after key in JSON object");
        }
        ++index;  // Skip ':'
        skipWhitespace(str, index);

        JsonValue value = parseValue(str, index);
        obj[key] = value;
        skipWhitespace(str, index);

        if (str[index] == ',') {
            ++index;  // Skip comma and continue
        } else if (str[index] != '}') {
            THROW_INVALID_ARGUMENT("Expected ',' or '}' in JSON object");
        }
        skipWhitespace(str, index);
    }
    ++index;  // Skip closing '}'
    return obj;
}

auto JsonParser::parseArray(const std::string& str,
                            size_t& index) -> JsonArray {
    ++index;  // Skip opening '['
    JsonArray arr;
    skipWhitespace(str, index);

    while (str[index] != ']') {
        arr.push_back(parseValue(str, index));
        skipWhitespace(str, index);

        if (str[index] == ',') {
            ++index;  // Skip comma and continue
        } else if (str[index] != ']') {
            THROW_INVALID_ARGUMENT("Expected ',' or ']' in JSON array");
        }
        skipWhitespace(str, index);
    }
    ++index;  // Skip closing ']'
    return arr;
}

void JsonParser::skipWhitespace(const std::string& str, size_t& index) {
    while (index < str.size() && (std::isspace(str[index]) != 0)) {
        ++index;
    }
}

}  // namespace atom::type
