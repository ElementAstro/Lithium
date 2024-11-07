#ifndef ATOM_TYPE_JSON_SCHEMA_HPP
#define ATOM_TYPE_JSON_SCHEMA_HPP

#include <string>
#include <utility>
#include <vector>

#include "atom/macro.hpp"
#include "atom/type/json.hpp"

namespace json_schema {

using json = nlohmann::json;

// 定义用于存储验证错误的信息
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
     * @brief 设置根模式(schema)
     *
     * @param schema_json JSON格式的模式
     */
    void setRootSchema(const json& schema_json) {
        root_schema_ = schema_json;
        errors_.clear();
    }

    /**
     * @brief 验证给定的JSON实例是否符合模式
     *
     * @param instance 要验证的JSON实例
     * @return true 验证通过
     * @return false 验证失败
     */
    auto validate(const json& instance) -> bool {
        errors_.clear();
        validateSchema(instance, root_schema_, "");
        return errors_.empty();
    }

    /**
     * @brief 获取验证过程中产生的错误信息
     *
     * @return const std::vector<ValidationError>& 错误信息列表
     */
    [[nodiscard]] auto getErrors() const
        -> const std::vector<ValidationError>& {
        return errors_;
    }

private:
    json root_schema_;
    std::vector<ValidationError> errors_;

    /**
     * @brief 递归验证JSON实例与模式
     *
     * @param instance 当前JSON实例部分
     * @param schema 当前模式部分
     * @param path 当前路径，用于错误信息
     */
    void validateSchema(const json& instance, const json& schema,
                        const std::string& path) {
        // 处理 "type" 关键字
        if (schema.contains("type")) {
            const auto& type = schema["type"];
            if (!validate_type(instance, type)) {
                errors_.emplace_back(
                    "类型不匹配，期望类型为 " + typeToString(type), path);
                return;  // 类型不匹配，无法继续验证其他关键字
            }
        }

        // 处理 "required" 关键字
        if (schema.contains("required") && instance.is_object()) {
            const auto& required = schema["required"];
            for (const auto& req : required) {
                if (!instance.contains(req)) {
                    errors_.emplace_back(
                        "缺少必需的字段: " + req.get<std::string>(), path);
                }
            }
        }

        // 处理 "properties" 关键字
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

        // 处理 "items" 关键字（用于数组）
        if (schema.contains("items") && instance.is_array()) {
            const json& items_schema = schema["items"];
            for (size_t i = 0; i < instance.size(); ++i) {
                std::string current_path = path + "[" + std::to_string(i) + "]";
                validateSchema(instance[i], items_schema, current_path);
            }
        }

        // 处理 "enum" 关键字
        if (schema.contains("enum")) {
            bool found = false;
            for (const auto& enum_val : schema["enum"]) {
                if (instance == enum_val) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                errors_.emplace_back("值不在枚举范围内", path);
            }
        }

        // 处理 "minimum" 和 "maximum" 关键字
        if (schema.contains("minimum") && instance.is_number()) {
            double minimum = schema["minimum"].get<double>();
            if (instance.get<double>() < minimum) {
                errors_.emplace_back("值小于最小值 " + std::to_string(minimum),
                                     path);
            }
        }
        if (schema.contains("maximum") && instance.is_number()) {
            double maximum = schema["maximum"].get<double>();
            if (instance.get<double>() > maximum) {
                errors_.emplace_back("值大于最大值 " + std::to_string(maximum),
                                     path);
            }
        }

        // 处理 "minLength" 和 "maxLength" 关键字
        if (schema.contains("minLength") && instance.is_string()) {
            size_t minLength = schema["minLength"].get<size_t>();
            if (instance.get<std::string>().length() < minLength) {
                errors_.emplace_back(
                    "字符串长度小于最小长度 " + std::to_string(minLength),
                    path);
            }
        }
        if (schema.contains("maxLength") && instance.is_string()) {
            size_t maxLength = schema["maxLength"].get<size_t>();
            if (instance.get<std::string>().length() > maxLength) {
                errors_.emplace_back(
                    "字符串长度大于最大长度 " + std::to_string(maxLength),
                    path);
            }
        }

        // 可以根据需要继续添加更多的关键字支持
    }

    /**
     * @brief 验证JSON实例的类型是否符合模式要求
     *
     * @param instance JSON实例
     * @param type_mode 期望的类型，可以是字符串或者字符串数组
     * @return true 类型匹配
     * @return false 类型不匹配
     */
    bool validate_type(const json& instance, const json& type_mode) {
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
     * @brief 检查JSON实例的具体类型
     *
     * @param instance JSON实例
     * @param type_str 期望的类型字符串
     * @return true 类型匹配
     * @return false 类型不匹配
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
     * @brief 将类型模式转换为字符串表示
     *
     * @param type_mode 类型模式，可以是字符串或字符串数组
     * @return std::string 类型的字符串表示
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

}  // namespace json_schema

#endif  // ATOM_TYPE_JSON_SCHEMA_HPP