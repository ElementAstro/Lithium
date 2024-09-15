#ifndef ATOM_TYPE_RTYPE_HPP
#define ATOM_TYPE_RTYPE_HPP

#include <functional>
#include <type_traits>

#include "atom/error/exception.hpp"
#include "atom/function/concept.hpp"
#include "rjson.hpp"
#include "ryaml.hpp"

namespace atom::type {
template <typename T, typename MemberType>
struct Field {
    using member_type = MemberType;
    const char* name;
    const char* description;
    MemberType T::*member;
    bool required;
    MemberType default_value;
    using Validator = std::function<bool(const MemberType&)>;
    Validator validator;

    Field(const char* n, const char* desc, MemberType T::*m, bool r = true,
          MemberType def = {}, Validator v = nullptr)
        : name(n),
          description(desc),
          member(m),
          required(r),
          default_value(std::move(def)),
          validator(std::move(v)) {}
};

template <typename T, typename MemberType, typename ReflectType>
struct ComplexField {
    using member_type = MemberType;
    const char* name;
    const char* description;
    MemberType T::*member;
    ReflectType reflect_type;

    ComplexField(const char* n, const char* desc, MemberType T::*m,
                 ReflectType reflect)
        : name(n), description(desc), member(m), reflect_type(reflect) {}
};

template <typename T, typename... Fields>
struct Reflectable {
    using ReflectedType = T;
    std::tuple<Fields...> fields;

    explicit Reflectable(Fields... flds) : fields(flds...) {}

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
                             // 处理复杂对象反射
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
                         // 处理复杂对象反射
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

// 普通字段的 make_field
template <typename T, typename MemberType>
auto make_field(const char* name, const char* description,
                MemberType T::*member, bool required = true,
                MemberType default_value = {},
                typename Field<T, MemberType>::Validator validator = nullptr)
    -> Field<T, MemberType> {
    return Field<T, MemberType>(name, description, member, required,
                                default_value, validator);
}

// 复杂字段（嵌套反射类型）的 make_field
template <typename T, typename MemberType, typename ReflectType>
auto make_field(const char* name, const char* description,
                MemberType T::*member, ReflectType reflect_type)
    -> ComplexField<T, MemberType, ReflectType> {
    return ComplexField<T, MemberType, ReflectType>(name, description, member,
                                                    reflect_type);
}
}  // namespace atom::type

#endif
