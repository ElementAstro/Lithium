#include "dynamic_object.hpp"

namespace Carbon {
namespace dispatch {

Dynamic_Object::Dynamic_Object(std::string t_type_name)
    : m_type_name(std::move(t_type_name)), m_option_explicit(false) {}

bool Dynamic_Object::is_explicit() const { return m_option_explicit; }

void Dynamic_Object::set_explicit(const bool t_explicit) {
    m_option_explicit = t_explicit;
}

const std::string &Dynamic_Object::get_type_name() const { return m_type_name; }

const Boxed_Value &Dynamic_Object::operator[](
    const std::string &t_attr_name) const {
    return get_attr(t_attr_name);
}

Boxed_Value &Dynamic_Object::operator[](const std::string &t_attr_name) {
    return get_attr(t_attr_name);
}

const Boxed_Value &Dynamic_Object::get_attr(
    const std::string &t_attr_name) const {
    auto a = m_attrs.find(t_attr_name);

    if (a != m_attrs.end()) {
        return a->second;
    } else {
        throw std::range_error("Attr not found '" + t_attr_name +
                               "' and cannot be added to const obj");
    }
}

bool Dynamic_Object::has_attr(const std::string &t_attr_name) const {
    return m_attrs.find(t_attr_name) != m_attrs.end();
}

Boxed_Value &Dynamic_Object::get_attr(const std::string &t_attr_name) {
    return m_attrs[t_attr_name];
}

Boxed_Value &Dynamic_Object::method_missing(const std::string &t_method_name) {
    if (m_option_explicit && m_attrs.find(t_method_name) == m_attrs.end()) {
        throw option_explicit_set(t_method_name);
    }

    return get_attr(t_method_name);
}

const Boxed_Value &Dynamic_Object::method_missing(
    const std::string &t_method_name) const {
    if (m_option_explicit && m_attrs.find(t_method_name) == m_attrs.end()) {
        throw option_explicit_set(t_method_name);
    }

    return get_attr(t_method_name);
}

std::map<std::string, Boxed_Value> Dynamic_Object::get_attrs() const {
    return m_attrs;
}

}  // namespace dispatch
}  // namespace Carbon
