#ifndef ATOM_META_REFL_JSON_HPP
#define ATOM_META_REFL_JSON_HPP

#include <functional>
#include <string>
#include <tuple>
#include <utility>

#include "atom/error/exception.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

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

    [[nodiscard]] auto from_json(const json& j) const -> T {
        T obj;
        std::apply(
            [&](auto... field) {
                (([&] {
                     if (j.contains(field.name)) {
                         j.at(field.name).get_to(obj.*(field.member));
                         if (field.validator &&
                             !field.validator(obj.*(field.member))) {
                             THROW_INVALID_ARGUMENT(
                                 std::string("Validation failed for field: ") +
                                 field.name);
                         }
                     } else if (!field.required) {
                         obj.*(field.member) =
                             field.default_value;
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

    [[nodiscard]] auto to_json(const T& obj) const -> json {
        json j;
        std::apply(
            [&](auto... field) {
                ((j[field.name] = obj.*(field.member)), ...);
            },
            fields);
        return j;
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
