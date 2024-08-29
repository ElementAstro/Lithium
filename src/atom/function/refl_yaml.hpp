#ifndef ATOM_META_REFL_YAML_HPP
#define ATOM_META_REFL_YAML_HPP

#if __has_include(<yaml-cpp/yaml.h>)
#include <yaml-cpp/yaml.h>
#include <functional>
#include <string>
#include <tuple>
#include <utility>

#include "atom/error/exception.hpp"

namespace atom::meta {
// Helper structure: used to store field names and member pointers
template <typename T, typename MemberType>
struct Field {
    const char* name;
    MemberType T::*member;
    bool required;
    MemberType default_value;
    using Validator = std::function<bool(const MemberType&)>;
    Validator validator;

    Field(const char* n, MemberType T::*m, bool r = true, MemberType def = {},
          Validator v = nullptr)
        : name(n),
          member(m),
          required(r),
          default_value(std::move(def)),
          validator(std::move(v)) {}
};

// Reflectable class template
template <typename T, typename... Fields>
struct Reflectable {
    using ReflectedType = T;
    std::tuple<Fields...> fields;

    explicit Reflectable(Fields... flds) : fields(flds...) {}

    [[nodiscard]] auto from_yaml(const YAML::Node& node) const -> T {
        T obj;
        std::apply(
            [&](auto... field) {
                (([&] {
                     using MemberType = decltype(T().*(field.member));
                     if (node[field.name]) {
                         // Deserialize into a value first
                         auto temp = node[field.name].template as<MemberType>();
                         // Then assign the value to the object
                         obj.*(field.member) = std::move(temp);
                         if (field.validator &&
                             !field.validator(obj.*(field.member))) {
                             THROW_INVALID_ARGUMENT(
                                 std::string("Validation failed for field: ") +
                                 field.name);
                         }
                     } else if (!field.required) {
                         obj.*(field.member) = field.default_value;
                     } else {
                         THROW_MISSING_ARGUMENT(
                             std::string("Missing required field: ") +
                             field.name);
                     }
                 }()),
                 ...);
            },
            fields);
        return obj;
    }

    [[nodiscard]] auto to_yaml(const T& obj) const -> YAML::Node {
        YAML::Node node;
        std::apply(
            [&](auto... field) {
                ((node[field.name] = obj.*(field.member)), ...);
            },
            fields);
        return node;
    }
};

// Field creation function
template <typename T, typename MemberType>
auto make_field(const char* name, MemberType T::*member, bool required = true,
                MemberType default_value = {},
                typename Field<T, MemberType>::Validator validator = nullptr)
    -> Field<T, MemberType> {
    return Field<T, MemberType>(name, member, required, default_value,
                                validator);
}
}  // namespace atom::meta

#endif

#endif
