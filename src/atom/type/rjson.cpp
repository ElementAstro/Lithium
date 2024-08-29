#include "rjson.hpp"

#include "atom/error/exception.hpp"

namespace atom::type {

JsonValue::JsonValue() : type_(Type::Null), value_(nullptr) {}

JsonValue::JsonValue(const std::string& value)
    : type_(Type::String), value_(value) {}

JsonValue::JsonValue(double value) : type_(Type::Number), value_(value) {}

JsonValue::JsonValue(bool value) : type_(Type::Bool), value_(value) {}

JsonValue::JsonValue(const JsonObject& value)
    : type_(Type::Object), value_(value) {}

JsonValue::JsonValue(const JsonArray& value)
    : type_(Type::Array), value_(value) {}

auto JsonValue::type() const -> Type { return type_; }

auto JsonValue::as_string() const -> const std::string& {
    if (type_ != Type::String) {
        THROW_INVALID_ARGUMENT("Not a string");
    }
    return std::get<std::string>(value_);
}

auto JsonValue::as_number() const -> double {
    if (type_ != Type::Number) {
        THROW_INVALID_ARGUMENT("Not a number");
    }
    return std::get<double>(value_);
}

auto JsonValue::as_bool() const -> bool {
    if (type_ != Type::Bool) {
        THROW_INVALID_ARGUMENT("Not a bool");
    }
    return std::get<bool>(value_);
}

auto JsonValue::as_object() const -> const JsonObject& {
    if (type_ != Type::Object) {
        THROW_INVALID_ARGUMENT("Not an object");
    }
    return std::get<JsonObject>(value_);
}

auto JsonValue::as_array() const -> const JsonArray& {
    if (type_ != Type::Array) {
        THROW_INVALID_ARGUMENT("Not an array");
    }
    return std::get<JsonArray>(value_);
}

auto JsonValue::to_string() const -> std::string {
    switch (type_) {
        case Type::Null:
            return "null";
        case Type::String:
            return "\"" + as_string() + "\"";
        case Type::Number:
            return std::to_string(as_number());
        case Type::Bool:
            return as_bool() ? "true" : "false";
        case Type::Object: {
            std::string result = "{";
            const auto& obj = as_object();
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                if (it != obj.begin()) {
                    result += ",";
                }
                result += "\"" + it->first + "\":" + it->second.to_string();
            }
            result += "}";
            return result;
        }
        case Type::Array: {
            std::string result = "[";
            const auto& arr = as_array();
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i > 0) {
                    result += ",";
                }
                result += arr[i].to_string();
            }
            result += "]";
            return result;
        }
    }
    THROW_INVALID_ARGUMENT("Unknown type");
}

auto JsonValue::operator[](const std::string& key) const -> const JsonValue& {
    if (type_ != Type::Object) {
        THROW_INVALID_ARGUMENT("Not an object");
    }
    return as_object().at(key);
}

auto JsonValue::operator[](size_t index) const -> const JsonValue& {
    if (type_ != Type::Array) {
        THROW_INVALID_ARGUMENT("Not an array");
    }
    return as_array().at(index);
}

auto JsonParser::parse(const std::string& str) -> JsonValue {
    size_t index = 0;
    return parse_value(str, index);
}

auto JsonParser::parse_value(const std::string& str,
                             size_t& index) -> JsonValue {
    skip_whitespace(str, index);

    if (str[index] == '"') {
        return JsonValue(parse_string(str, index));
    }
    if (str[index] == 't' || str[index] == 'f') {
        return JsonValue(parse_bool(str, index));
    }
    if (str[index] == 'n') {
        parse_null(str, index);
        return {};
    }
    if (str[index] == '{') {
        return JsonValue(parse_object(str, index));
    }
    if (str[index] == '[') {
        return JsonValue(parse_array(str, index));
    }
    if ((std::isdigit(str[index]) != 0) || str[index] == '-') {
        return JsonValue(parse_number(str, index));
    }

    THROW_INVALID_ARGUMENT("Invalid JSON value");
}

auto JsonParser::parse_string(const std::string& str,
                              size_t& index) -> std::string {
    ++index;
    std::string result;
    while (str[index] != '"') {
        result += str[index++];
    }
    ++index;
    return result;
}

auto JsonParser::parse_number(const std::string& str, size_t& index) -> double {
    size_t startIndex = index;
    if (str[index] == '-') {
        ++index;
    }
    while (std::isdigit(str[index]) != 0) {
        ++index;
    }
    if (str[index] == '.') {
        ++index;
    }
    while (std::isdigit(str[index]) != 0) {
        ++index;
    }
    return std::stod(str.substr(startIndex, index - startIndex));
}

auto JsonParser::parse_bool(const std::string& str, size_t& index) -> bool {
    if (str.substr(index, 4) == "true") {
        index += 4;
        return true;
    }
    if (str.substr(index, 5) == "false") {
        index += 5;
        return false;
    }
    THROW_INVALID_ARGUMENT("Invalid JSON boolean");
}

void JsonParser::parse_null(const std::string& str, size_t& index) {
    if (str.substr(index, 4) == "null") {
        index += 4;
    } else {
        THROW_INVALID_ARGUMENT("Invalid JSON null");
    }
}

auto JsonParser::parse_object(const std::string& str,
                              size_t& index) -> JsonObject {
    ++index;
    JsonObject obj;
    skip_whitespace(str, index);

    while (str[index] != '}') {
        std::string key = parse_string(str, index);
        skip_whitespace(str, index);
        if (str[index] != ':') {
            THROW_INVALID_ARGUMENT("Expected ':' in JSON object");
        }
        ++index;
        JsonValue value = parse_value(str, index);
        obj[key] = value;
        skip_whitespace(str, index);
        if (str[index] == ',') {
            ++index;
        }
        skip_whitespace(str, index);
    }
    ++index;  // Skip the closing '}'
    return obj;
}

auto JsonParser::parse_array(const std::string& str,
                             size_t& index) -> JsonArray {
    ++index;
    JsonArray arr;
    skip_whitespace(str, index);

    while (str[index] != ']') {
        arr.push_back(parse_value(str, index));
        skip_whitespace(str, index);
        if (str[index] == ',') {
            ++index;
        }
        skip_whitespace(str, index);
    }
    ++index;  // Skip the closing ']'
    return arr;
}

void JsonParser::skip_whitespace(const std::string& str, size_t& index) {
    while (std::isspace(str[index]) != 0) {
        ++index;
    }
}

}  // namespace atom::type
