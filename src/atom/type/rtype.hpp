#ifndef ATOM_TYPE_RTYPE_HPP
#define ATOM_TYPE_RTYPE_HPP

#include <functional>
#include <type_traits>

#include "atom/error/exception.hpp"
#include "atom/function/concept.hpp"
#include "rjson.hpp"
#include "ryaml.hpp"

namespace atom::type {

/**
 * @struct Field
 * @brief Represents a field in a reflectable type.
 * @tparam T The type containing the field.
 * @tparam MemberType The type of the field.
 */
template <typename T, typename MemberType>
struct Field {
    using member_type = MemberType;
    const char* name;          ///< The name of the field.
    const char* description;   ///< The description of the field.
    MemberType T::*member;     ///< Pointer to the member field.
    bool required;             ///< Indicates if the field is required.
    MemberType default_value;  ///< The default value of the field.
    using Validator =
        std::function<bool(const MemberType&)>;  ///< Validator function type.
    Validator validator;                         ///< Validator function.

    /**
     * @brief Constructs a Field.
     * @param n The name of the field.
     * @param desc The description of the field.
     * @param m Pointer to the member field.
     * @param r Indicates if the field is required (default is true).
     * @param def The default value of the field (default is an empty value).
     * @param v The validator function (default is nullptr).
     */
    Field(const char* n, const char* desc, MemberType T::*m, bool r = true,
          MemberType def = {}, Validator v = nullptr)
        : name(n),
          description(desc),
          member(m),
          required(r),
          default_value(std::move(def)),
          validator(std::move(v)) {}
};

/**
 * @struct ComplexField
 * @brief Represents a complex field in a reflectable type.
 * @tparam T The type containing the field.
 * @tparam MemberType The type of the field.
 * @tparam ReflectType The type used for reflection.
 */
template <typename T, typename MemberType, typename ReflectType>
struct ComplexField {
    using member_type = MemberType;
    const char* name;          ///< The name of the field.
    const char* description;   ///< The description of the field.
    MemberType T::*member;     ///< Pointer to the member field.
    ReflectType reflect_type;  ///< The reflection type.

    /**
     * @brief Constructs a ComplexField.
     * @param n The name of the field.
     * @param desc The description of the field.
     * @param m Pointer to the member field.
     * @param reflect The reflection type.
     */
    ComplexField(const char* n, const char* desc, MemberType T::*m,
                 ReflectType reflect)
        : name(n), description(desc), member(m), reflect_type(reflect) {}
};

/**
 * @struct Reflectable
 * @brief Represents a type that can be reflected upon.
 * @tparam T The type to reflect.
 * @tparam Fields The fields of the type.
 */
template <typename T, typename... Fields>
struct Reflectable {
    using ReflectedType = T;
    std::tuple<Fields...> fields;  ///< The fields of the type.

    /**
     * @brief Constructs a Reflectable.
     * @param flds The fields of the type.
     */
    explicit Reflectable(Fields... flds) : fields(flds...) {}

    /**
     * @brief Creates an object from a JSON representation.
     * @param j The JSON object.
     * @return The created object.
     */
    [[nodiscard]] auto from_json(const JsonObject& j) const -> T {
        T obj;
        std::apply(
            [&](auto&&... field) {
                (([&] {
                     auto it = j.find(field.name);
                     using MemberType =
                         typename std::decay_t<decltype(field)>::member_type;

                     if (it != j.end()) {
                         if constexpr (String<MemberType> || Char<MemberType>) {
                             obj.*(field.member) = it->second.as_string();
                         } else if constexpr (Number<MemberType>) {
                             obj.*(field.member) =
                                 static_cast<int>(it->second.as_number());
                         } else if constexpr (std::is_same_v<MemberType,
                                                             bool>) {
                             obj.*(field.member) = it->second.as_bool();
                         } else if constexpr (StringContainer<MemberType>) {
                             for (const auto& item : it->second.as_array()) {
                                 (obj.*(field.member))
                                     .push_back(item.as_string());
                             }
                         } else if constexpr (NumberContainer<MemberType>) {
                             for (const auto& item : it->second.as_array()) {
                                 (obj.*(field.member))
                                     .push_back(
                                         static_cast<MemberType::value_type>(
                                             item.as_number()));
                             }
                         } else if constexpr (std::is_class_v<MemberType>) {
                             // Handle complex object reflection
                             obj.*(field.member) = field.reflect_type.from_json(
                                 it->second.as_object());
                         } else {
                             THROW_INVALID_ARGUMENT(
                                 "Unsupported type for field: " +
                                 std::string(field.name));
                         }
                         if constexpr (requires { field.validator; }) {
                             if (field.validator &&
                                 !field.validator(obj.*(field.member))) {
                                 THROW_INVALID_ARGUMENT(
                                     "Validation failed for field: " +
                                     std::string(field.name));
                             }
                         }
                     } else if constexpr (requires { field.default_value; }) {
                         if (!field.required) {
                             obj.*(field.member) = field.default_value;
                         } else {
                             THROW_INVALID_ARGUMENT("Missing required field: " +
                                                    std::string(field.name));
                         }
                     }
                 }()),
                 ...);
            },
            fields);
        return obj;
    }

    /**
     * @brief Converts an object to a JSON representation.
     * @param obj The object to convert.
     * @return The JSON representation.
     */
    [[nodiscard]] auto to_json(const T& obj) const -> JsonObject {
        JsonObject j;
        std::apply(
            [&](auto&&... field) {
                (([&] {
                     using MemberType =
                         typename std::decay_t<decltype(field)>::member_type;
                     if constexpr (String<MemberType> || Char<MemberType>) {
                         j[field.name] = JsonValue(obj.*(field.member));
                     } else if constexpr (Number<MemberType>) {
                         j[field.name] = JsonValue(
                             static_cast<MemberType>(obj.*(field.member)));
                     } else if constexpr (std::is_same_v<MemberType, bool>) {
                         j[field.name] = JsonValue(obj.*(field.member));
                     } else if constexpr (StringContainer<MemberType>) {
                         JsonArray arr;
                         for (const auto& item : obj.*(field.member)) {
                             arr.push_back(JsonValue(item));
                         }
                         j[field.name] = JsonValue(arr);
                     } else if constexpr (NumberContainer<MemberType>) {
                         JsonArray arr;
                         for (const auto& item : obj.*(field.member)) {
                             arr.push_back(JsonValue(
                                 static_cast<MemberType::value_type>(item)));
                         }
                         j[field.name] = JsonValue(arr);
                     } else if constexpr (std::is_class_v<MemberType>) {
                         // Handle complex object reflection
                         j[field.name] = JsonValue(
                             field.reflect_type.to_json(obj.*(field.member)));
                     } else {
                         THROW_INVALID_ARGUMENT("Unsupported type for field: " +
                                                std::string(field.name));
                     }
                 }()),
                 ...);
            },
            fields);
        return j;
    }

    /**
     * @brief Creates an object from a YAML representation.
     * @param y The YAML object.
     * @return The created object.
     */
    [[nodiscard]] auto from_yaml(const YamlObject& y) const -> T {
        T obj;
        std::apply(
            [&](auto&&... field) {
                (([&] {
                     auto it = y.find(field.name);
                     using MemberType =
                         typename std::decay_t<decltype(field)>::member_type;

                     if (it != y.end()) {
                         if constexpr (String<MemberType> || Char<MemberType>) {
                             obj.*(field.member) = it->second.as_string();
                         } else if constexpr (Number<MemberType>) {
                             obj.*(field.member) =
                                 static_cast<int>(it->second.as_number());
                         } else if constexpr (std::is_same_v<MemberType,
                                                             bool>) {
                             obj.*(field.member) = it->second.as_bool();
                         } else if constexpr (StringContainer<MemberType>) {
                             for (const auto& item : it->second.as_array()) {
                                 (obj.*(field.member))
                                     .push_back(item.as_string());
                             }
                         } else if constexpr (Container<MemberType>) {
                             if constexpr (Number<typename MemberType::
                                                      value_type>) {
                                 for (const auto& item :
                                      it->second.as_array()) {
                                     (obj.*(field.member))
                                         .push_back(
                                             static_cast<typename MemberType::
                                                             value_type>(
                                                 item.as_number()));
                                 }
                             } else if constexpr (String<typename MemberType::
                                                             value_type>) {
                                 for (const auto& item :
                                      it->second.as_array()) {
                                     (obj.*(field.member))
                                         .push_back(item.as_string());
                                 }
                             } else if constexpr (
                                 std::is_same_v<typename MemberType::value_type,
                                                bool>) {
                                 for (const auto& item :
                                      it->second.as_array()) {
                                     (obj.*(field.member))
                                         .push_back(item.as_bool());
                                 }
                             }
                         } else if constexpr (AssociativeContainer<
                                                  MemberType>) {
                             if constexpr (Number<typename MemberType::
                                                      mapped_type>) {
                                 for (const auto& item :
                                      it->second.as_object()) {
                                     (obj.*(field.member))[item.first] =
                                         static_cast<
                                             typename MemberType::mapped_type>(
                                             item.second.as_number());
                                 }
                             } else if constexpr (String<typename MemberType::
                                                             mapped_type>) {
                                 for (const auto& item :
                                      it->second.as_object()) {
                                     (obj.*(field.member))[item.first] =
                                         item.second.as_string();
                                 }
                             }
                         } else if constexpr (std::is_class_v<MemberType>) {
                             obj.*(field.member) = field.reflect_type.from_yaml(
                                 it->second.as_object());
                         } else {
                             THROW_INVALID_ARGUMENT(
                                 "Unsupported type for field: " +
                                 std::string(field.name));
                         }
                         if constexpr (requires { field.validator; }) {
                             if (field.validator &&
                                 !field.validator(obj.*(field.member))) {
                                 THROW_INVALID_ARGUMENT(
                                     "Validation failed for field: " +
                                     std::string(field.name));
                             }
                         }
                     } else if constexpr (requires { field.default_value; }) {
                         if (!field.required) {
                             obj.*(field.member) = field.default_value;
                         } else {
                             THROW_INVALID_ARGUMENT("Missing required field: " +
                                                    std::string(field.name));
                         }
                     }
                 }()),
                 ...);
            },
            fields);
        return obj;
    }

    /**
     * @brief Converts an object to a YAML representation.
     * @param obj The object to convert.
     * @return The YAML representation.
     */
    [[nodiscard]] auto to_yaml(const T& obj) const -> YamlObject {
        YamlObject y;
        std::apply(
            [&](auto&&... field) {
                (([&] {
                     using MemberType =
                         typename std::decay_t<decltype(field)>::member_type;
                     if constexpr (std::is_same_v<MemberType, std::string>) {
                         y[field.name] = YamlValue(obj.*(field.member));
                     } else if constexpr (std::is_same_v<MemberType, int> ||
                                          std::is_same_v<MemberType, double>) {
                         y[field.name] = YamlValue(
                             static_cast<double>(obj.*(field.member)));
                     } else if constexpr (std::is_same_v<MemberType, bool>) {
                         y[field.name] = YamlValue(obj.*(field.member));
                     } else if constexpr (std::is_same_v<
                                              MemberType,
                                              std::vector<std::string>>) {
                         YamlArray arr;
                         for (const auto& item : obj.*(field.member)) {
                             arr.push_back(YamlValue(item));
                         }
                         y[field.name] = YamlValue(arr);
                     } else if constexpr (std::is_same_v<MemberType,
                                                         std::vector<int>> ||
                                          std::is_same_v<MemberType,
                                                         std::vector<double>>) {
                         YamlArray arr;
                         for (const auto& item : obj.*(field.member)) {
                             arr.push_back(
                                 YamlValue(static_cast<double>(item)));
                         }
                         y[field.name] = YamlValue(arr);
                     } else if constexpr (std::is_class_v<MemberType>) {
                         y[field.name] = YamlValue(
                             field.reflect_type.to_yaml(obj.*(field.member)));
                     } else {
                         THROW_INVALID_ARGUMENT("Unsupported type for field: " +
                                                std::string(field.name));
                     }
                 }()),
                 ...);
            },
            fields);
        return y;
    }
};

/**
 * @brief Creates a Field object.
 * @tparam T The type containing the field.
 * @tparam MemberType The type of the field.
 * @param name The name of the field.
 * @param description The description of the field.
 * @param member Pointer to the member field.
 * @param required Indicates if the field is required (default is true).
 * @param default_value The default value of the field (default is an empty
 * value).
 * @param validator The validator function (default is nullptr).
 * @return The created Field object.
 */
template <typename T, typename MemberType>
auto make_field(const char* name, const char* description,
                MemberType T::*member, bool required = true,
                MemberType default_value = {},
                typename Field<T, MemberType>::Validator validator = nullptr)
    -> Field<T, MemberType> {
    return Field<T, MemberType>(name, description, member, required,
                                default_value, validator);
}

/**
 * @brief Creates a ComplexField object.
 * @tparam T The type containing the field.
 * @tparam MemberType The type of the field.
 * @tparam ReflectType The type used for reflection.
 * @param name The name of the field.
 * @param description The description of the field.
 * @param member Pointer to the member field.
 * @param reflect_type The reflection type.
 * @return The created ComplexField object.
 */
template <typename T, typename MemberType, typename ReflectType>
auto make_field(const char* name, const char* description,
                MemberType T::*member, ReflectType reflect_type)
    -> ComplexField<T, MemberType, ReflectType> {
    return ComplexField<T, MemberType, ReflectType>(name, description, member,
                                                    reflect_type);
}

}  // namespace atom::type

#endif  // ATOM_TYPE_RTYPE_HPP
