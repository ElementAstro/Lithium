
#ifndef CARBON_DYNAMIC_OBJECT_HPP
#define CARBON_DYNAMIC_OBJECT_HPP

#include <map>
#include <string>
#include <utility>

#include "boxed_value.hpp"

namespace Carbon {
class Type_Conversions;
namespace dispatch {
class Proxy_Function_Base;
}  // namespace dispatch
}  // namespace Carbon

namespace Carbon {
namespace dispatch {
struct option_explicit_set : std::runtime_error {
    explicit option_explicit_set(const std::string &t_param_name)
        : std::runtime_error("option explicit set and parameter '" +
                             t_param_name + "' does not exist") {}

    option_explicit_set(const option_explicit_set &) = default;

    ~option_explicit_set() noexcept override = default;
};

class Dynamic_Object {
public:
    explicit Dynamic_Object(std::string t_type_name);

    Dynamic_Object() = default;

    bool is_explicit() const noexcept;

    void set_explicit(const bool t_explicit) noexcept;

    const std::string &get_type_name() const noexcept;

    const Boxed_Value &operator[](const std::string &t_attr_name) const;

    Boxed_Value &operator[](const std::string &t_attr_name);

    const Boxed_Value &get_attr(const std::string &t_attr_name) const;

    bool has_attr(const std::string &t_attr_name) const;

    Boxed_Value &get_attr(const std::string &t_attr_name);

    Boxed_Value &method_missing(const std::string &t_method_name);

    const Boxed_Value &method_missing(const std::string &t_method_name) const;

    std::map<std::string, Boxed_Value> get_attrs();

private:
    const std::string m_type_name = "";
    bool m_option_explicit = false;

    std::map<std::string, Boxed_Value> m_attrs;
};

}  // namespace dispatch
}  // namespace Carbon
#endif
