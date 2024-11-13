#ifndef ATOM_TYPE_JSON_SCHEMA_HPP
#define ATOM_TYPE_JSON_SCHEMA_HPP

#include <regex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "atom/macro.hpp"
#include "atom/type/json.hpp"

namespace atom::type {

using json = nlohmann::json;

// Structure to store validation error information
struct ValidationError {
    std::string message;
    std::string path;

    ValidationError(std::string msg, std::string p = "")
        : message(std::move(msg)), path(std::move(p)) {}
} ATOM_ALIGNAS(64);

class JsonValidator {
public:
    JsonValidator() = default;

    /**
     * @brief Sets the root schema
     *
     * @param schema_json JSON formatted schema
     */
    void setRootSchema(const json& schema_json) {
        root_schema_ = schema_json;
        errors_.clear();
    }

    /**
     * @brief Validates the given JSON instance against the schema
     *
     * @param instance JSON instance to validate
     * @return true if validation passes
     * @return false if validation fails
     */
    auto validate(const json& instance) -> bool {
        errors_.clear();
        validateSchema(instance, root_schema_, "");
        return errors_.empty();
    }

    /**
     * @brief Gets the validation errors
     *
     * @return const std::vector<ValidationError>& List of validation errors
     */
    [[nodiscard]] auto getErrors() const
        -> const std::vector<ValidationError>& {
        return errors_;
    }

private:
    json root_schema_;
    std::vector<ValidationError> errors_;

    /**
     * @brief Recursively validates JSON instance against the schema
     *
     * @param instance Current JSON instance part
     * @param schema Current schema part
     * @param path Current path for error messages
     */
    void validateSchema(const json& instance, const json& schema,
                        const std::string& path) {
        // Handle "type" keyword
        if (schema.contains("type")) {
            const auto& type = schema["type"];
            if (!validateType(instance, type)) {
                errors_.emplace_back(
                    "Type mismatch, expected type: " + typeToString(type),
                    path);
                return;  // Type mismatch, cannot continue validating other
                         // keywords
            }
        }

        // Handle "required" keyword
        if (schema.contains("required") && instance.is_object()) {
            const auto& required = schema["required"];
            for (const auto& req : required) {
                if (!instance.contains(req)) {
                    errors_.emplace_back(
                        "Missing required field: " + req.get<std::string>(),
                        path);
                }
            }
        }

        // Handle "properties" keyword
        if (schema.contains("properties") && instance.is_object()) {
            const auto& properties = schema["properties"];
            for (auto it = properties.begin(); it != properties.end(); ++it) {
                const std::string& key = it.key();
                const json& prop_schema = it.value();
                std::string current_path =
                    path.empty() ? key : path + "." + key;
                if (instance.contains(key)) {
                    validateSchema(instance[key], prop_schema, current_path);
                }
            }
        }

        // Handle "items" keyword (for arrays)
        if (schema.contains("items") && instance.is_array()) {
            const json& items_schema = schema["items"];
            for (size_t i = 0; i < instance.size(); ++i) {
                std::string current_path = path + "[" + std::to_string(i) + "]";
                validateSchema(instance[i], items_schema, current_path);
            }
        }

        // Handle "enum" keyword
        if (schema.contains("enum")) {
            bool found = false;
            for (const auto& enum_val : schema["enum"]) {
                if (instance == enum_val) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                errors_.emplace_back("Value not in enum range", path);
            }
        }

        // Handle "minimum" and "maximum" keywords
        if (schema.contains("minimum") && instance.is_number()) {
            double minimum = schema["minimum"].get<double>();
            if (instance.get<double>() < minimum) {
                errors_.emplace_back(
                    "Value less than minimum: " + std::to_string(minimum),
                    path);
            }
        }
        if (schema.contains("maximum") && instance.is_number()) {
            double maximum = schema["maximum"].get<double>();
            if (instance.get<double>() > maximum) {
                errors_.emplace_back(
                    "Value greater than maximum: " + std::to_string(maximum),
                    path);
            }
        }

        // Handle "minLength" and "maxLength" keywords
        if (schema.contains("minLength") && instance.is_string()) {
            size_t minLength = schema["minLength"].get<size_t>();
            if (instance.get<std::string>().length() < minLength) {
                errors_.emplace_back(
                    "String length less than minimum length: " +
                        std::to_string(minLength),
                    path);
            }
        }
        if (schema.contains("maxLength") && instance.is_string()) {
            size_t maxLength = schema["maxLength"].get<size_t>();
            if (instance.get<std::string>().length() > maxLength) {
                errors_.emplace_back(
                    "String length greater than maximum length: " +
                        std::to_string(maxLength),
                    path);
            }
        }

        // Handle "pattern" keyword
        if (schema.contains("pattern") && instance.is_string()) {
            const std::string& pattern = schema["pattern"];
            std::regex regexPattern(pattern);
            if (!std::regex_match(instance.get<std::string>(), regexPattern)) {
                errors_.emplace_back(
                    "String does not match pattern: " + pattern, path);
            }
        }

        // Handle "minItems" and "maxItems" keywords
        if (schema.contains("minItems") && instance.is_array()) {
            size_t minItems = schema["minItems"].get<size_t>();
            if (instance.size() < minItems) {
                errors_.emplace_back("Array size less than minimum items: " +
                                         std::to_string(minItems),
                                     path);
            }
        }
        if (schema.contains("maxItems") && instance.is_array()) {
            size_t maxItems = schema["maxItems"].get<size_t>();
            if (instance.size() > maxItems) {
                errors_.emplace_back("Array size greater than maximum items: " +
                                         std::to_string(maxItems),
                                     path);
            }
        }

        // Handle "uniqueItems" keyword
        if (schema.contains("uniqueItems") && instance.is_array()) {
            std::set<json> uniqueItems(instance.begin(), instance.end());
            if (uniqueItems.size() != instance.size()) {
                errors_.emplace_back("Array items are not unique", path);
            }
        }

        // Handle "const" keyword
        if (schema.contains("const")) {
            if (instance != schema["const"]) {
                errors_.emplace_back("Value does not match const value", path);
            }
        }

        // Handle "dependencies" keyword
        if (schema.contains("dependencies") && instance.is_object()) {
            const auto& dependencies = schema["dependencies"];
            for (auto it = dependencies.begin(); it != dependencies.end();
                 ++it) {
                const std::string& key = it.key();
                if (instance.contains(key)) {
                    const json& dependency = it.value();
                    if (dependency.is_array()) {
                        for (const auto& dep : dependency) {
                            if (!instance.contains(dep)) {
                                errors_.emplace_back("Missing dependency: " +
                                                         dep.get<std::string>(),
                                                     path);
                            }
                        }
                    } else if (dependency.is_object()) {
                        validateSchema(instance, dependency, path);
                    }
                }
            }
        }

        // Handle "allOf" keyword
        if (schema.contains("allOf")) {
            for (const auto& subschema : schema["allOf"]) {
                validateSchema(instance, subschema, path);
            }
        }

        // Handle "anyOf" keyword
        if (schema.contains("anyOf")) {
            bool valid = false;
            for (const auto& subschema : schema["anyOf"]) {
                std::vector<ValidationError> temp_errors = errors_;
                errors_.clear();
                validateSchema(instance, subschema, path);
                if (errors_.empty()) {
                    valid = true;
                    break;
                }
                errors_ = temp_errors;
            }
            if (!valid) {
                errors_.emplace_back(
                    "Value does not match any of the schemas in anyOf", path);
            }
        }

        // Handle "oneOf" keyword
        if (schema.contains("oneOf")) {
            int valid_count = 0;
            for (const auto& subschema : schema["oneOf"]) {
                std::vector<ValidationError> temp_errors = errors_;
                errors_.clear();
                validateSchema(instance, subschema, path);
                if (errors_.empty()) {
                    valid_count++;
                }
                errors_ = temp_errors;
            }
            if (valid_count != 1) {
                errors_.emplace_back(
                    "Value does not match exactly one of the schemas in oneOf",
                    path);
            }
        }

        // Handle "not" keyword
        if (schema.contains("not")) {
            std::vector<ValidationError> temp_errors = errors_;
            errors_.clear();
            validateSchema(instance, schema["not"], path);
            if (errors_.empty()) {
                errors_ = temp_errors;
                errors_.emplace_back("Value matches schema in not", path);
            } else {
                errors_ = temp_errors;
            }
        }
    }

    /**
     * @brief Validates the type of the JSON instance against the schema
     *
     * @param instance JSON instance
     * @param type_mode Expected type, can be a string or an array of strings
     * @return true if type matches
     * @return false if type does not match
     */
    bool validateType(const json& instance, const json& type_mode) {
        if (type_mode.is_string()) {
            return checkType(instance, type_mode.get<std::string>());
        }
        if (type_mode.is_array()) {
            for (const auto& typeStr : type_mode) {
                if (typeStr.is_string() &&
                    checkType(instance, typeStr.get<std::string>())) {
                    return true;
                }
            }
            return false;
        }
        return false;
    }

    /**
     * @brief Checks the specific type of the JSON instance
     *
     * @param instance JSON instance
     * @param type_str Expected type string
     * @return true if type matches
     * @return false if type does not match
     */
    static auto checkType(const json& instance,
                          const std::string& type_str) -> bool {
        if (type_str == "object") {
            return instance.is_object();
        }
        if (type_str == "array") {
            return instance.is_array();
        }
        if (type_str == "string") {
            return instance.is_string();
        }
        if (type_str == "number") {
            return instance.is_number();
        }
        if (type_str == "integer") {
            return instance.is_number_integer();
        }
        if (type_str == "boolean") {
            return instance.is_boolean();
        }
        if (type_str == "null") {
            return instance.is_null();
        }
        return false;
    }

    /**
     * @brief Converts the type schema to a string representation
     *
     * @param type_mode Type schema, can be a string or an array of strings
     * @return std::string String representation of the type
     */
    static auto typeToString(const json& type_mode) -> std::string {
        if (type_mode.is_string()) {
            return type_mode.get<std::string>();
        }
        if (type_mode.is_array()) {
            std::string types = "[";
            for (size_t i = 0; i < type_mode.size(); ++i) {
                if (i > 0)
                    types += ", ";
                types += type_mode[i].get<std::string>();
            }
            types += "]";
            return types;
        }
        return "unknown";
    }
};

}  // namespace atom::type

#endif  // ATOM_TYPE_JSON_SCHEMA_HPP