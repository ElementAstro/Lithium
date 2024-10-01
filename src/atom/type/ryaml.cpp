#include "ryaml.hpp"
#include <cctype>
#include <sstream>
#include <stdexcept>
#include "atom/error/exception.hpp"

namespace atom::type {

YamlValue::YamlValue() : type_(Type::Null), value_(nullptr) {}

YamlValue::YamlValue(const std::string& value)
    : type_(Type::String), value_(value) {}

YamlValue::YamlValue(double value) : type_(Type::Number), value_(value) {}

YamlValue::YamlValue(bool value) : type_(Type::Bool), value_(value) {}

YamlValue::YamlValue(const YamlObject& value)
    : type_(Type::Object), value_(value) {}

YamlValue::YamlValue(const YamlArray& value)
    : type_(Type::Array), value_(value) {}

YamlValue::YamlValue(const YamlValue& alias)
    : type_(Type::Alias), value_(const_cast<YamlValue*>(&alias)) {}

auto YamlValue::type() const -> Type { return type_; }

auto YamlValue::as_string() const -> const std::string& {
    if (type_ != Type::String) {
        THROW_INVALID_ARGUMENT("Not a string");
    }
    return std::get<std::string>(value_);
}

auto YamlValue::as_number() const -> double {
    if (type_ != Type::Number)
        THROW_INVALID_ARGUMENT("Not a number");
    return std::get<double>(value_);
}

auto YamlValue::as_bool() const -> bool {
    if (type_ != Type::Bool)
        THROW_INVALID_ARGUMENT("Not a boolean");
    return std::get<bool>(value_);
}

auto YamlValue::as_object() const -> const YamlObject& {
    if (type_ != Type::Object)
        THROW_INVALID_ARGUMENT("Not an object");
    return std::get<YamlObject>(value_);
}

auto YamlValue::as_array() const -> const YamlArray& {
    if (type_ != Type::Array)
        THROW_INVALID_ARGUMENT("Not an array");
    return std::get<YamlArray>(value_);
}

std::string YamlValue::to_string() const {
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
            std::string result;
            const auto& obj = as_object();
            for (const auto& [key, value] : obj) {
                result += key + ": " + value.to_string() + "\n";
            }
            return result;
        }
        case Type::Array: {
            std::string result;
            const auto& arr = as_array();
            for (const auto& item : arr) {
                result += "- " + item.to_string() + "\n";
            }
            return result;
        }
        case Type::Alias:
            return "*" + std::get<YamlValue*>(value_)->to_string();
    }
    THROW_INVALID_ARGUMENT("Unknown type");
}

auto YamlValue::operator[](const std::string& key) const -> const YamlValue& {
    if (type_ != Type::Object)
        THROW_INVALID_ARGUMENT("Not an object");
    return as_object().at(key);
}

auto YamlValue::operator[](size_t index) const -> const YamlValue& {
    if (type_ != Type::Array)
        THROW_INVALID_ARGUMENT("Not an array");
    return as_array().at(index);
}

auto YamlParser::parse(const std::string& str) -> YamlValue {
    size_t index = 0;
    return parse_value(str, index);
}

auto YamlParser::parse_value(const std::string& str,
                             size_t& index) -> YamlValue {
    skip_whitespace(str, index);

    if (str[index] == '"')
        return YamlValue(parse_string(str, index));
    if (str[index] == '-')
        return YamlValue(parse_array(str, index));
    if (str[index] == '&')  // Parsing anchors
        return YamlValue(parse_key_value(str, index));
    if (str[index] == '*')  // Parsing aliases
        return YamlValue(parse_alias(str, index));
    if (std::isalpha(str[index]))
        return YamlValue(parse_key_value(str, index));
    if (std::isdigit(str[index]) || str[index] == '-')
        return YamlValue(parse_number(str, index));
    if (str[index] == 't' || str[index] == 'f')
        return YamlValue(parse_bool(str, index));
    if (str[index] == 'n') {
        parse_null(str, index);
        return YamlValue();
    }

    THROW_INVALID_ARGUMENT("Invalid YAML value");
}

auto YamlParser::parse_string(const std::string& str,
                              size_t& index) -> std::string {
    ++index;
    std::string result;
    while (str[index] != '"') {
        result += str[index++];
    }
    ++index;
    return result;
}

auto YamlParser::parse_number(const std::string& str, size_t& index) -> double {
    size_t startIndex = index;
    if (str[index] == '-')
        ++index;
    while (std::isdigit(str[index]) || str[index] == '.') {
        ++index;
    }
    return std::stod(str.substr(startIndex, index - startIndex));
}

auto YamlParser::parse_bool(const std::string& str, size_t& index) -> bool {
    if (str.substr(index, 4) == "true") {
        index += 4;
        return true;
    }
    if (str.substr(index, 5) == "false") {
        index += 5;
        return false;
    }
    THROW_INVALID_ARGUMENT("Invalid YAML boolean");
}

void YamlParser::parse_null(const std::string& str, size_t& index) {
    if (str.substr(index, 4) == "null") {
        index += 4;
    } else {
        THROW_INVALID_ARGUMENT("Invalid YAML null");
    }
}

auto YamlParser::parse_key_value(const std::string& str,
                                 size_t& index) -> YamlObject {
    YamlObject obj;
    while (index < str.size()) {
        skip_whitespace(str, index);
        std::string key = parse_string(str, index);
        skip_whitespace(str, index);
        if (str[index] != ':')
            THROW_INVALID_ARGUMENT("Expected ':' in YAML key-value pair");
        ++index;
        skip_whitespace(str, index);
        obj[key] = parse_value(str, index);
        skip_whitespace(str, index);
        if (str[index] == '\n')
            ++index;
    }
    return obj;
}

auto YamlParser::parse_array(const std::string& str,
                             size_t& index) -> YamlArray {
    YamlArray arr;
    while (index < str.size()) {
        if (str[index] == '-') {
            ++index;
            skip_whitespace(str, index);
            arr.push_back(parse_value(str, index));
        } else {
            break;
        }
        skip_whitespace(str, index);
        if (str[index] == '\n')
            ++index;
    }
    return arr;
}

auto YamlParser::parse_alias(const std::string& str,
                             size_t& index) -> YamlValue {
    ++index;
    std::string aliasName;
    while (index < str.size() && !std::isspace(str[index])) {
        aliasName += str[index++];
    }
    // Assuming we store the aliases in some global map.
    // auto alias = aliases.find(aliasName);
    // if (alias == aliases.end()) {
    //     THROW_INVALID_ARGUMENT("Alias not found: " + aliasName);
    // }
    // return YamlValue(*alias->second);
    THROW_INVALID_ARGUMENT("Alias parsing is not yet implemented");
}

void YamlParser::skip_whitespace(const std::string& str, size_t& index) {
    while (index < str.size() && std::isspace(str[index])) {
        ++index;
    }
}

}  // namespace atom::type
