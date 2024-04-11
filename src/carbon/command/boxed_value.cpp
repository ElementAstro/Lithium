#include "boxed_value.hpp"

namespace Carbon {
Boxed_Value::Data::Data(const Type_Info &ti, Carbon::detail::Any to,
                        bool is_ref, const void *t_void_ptr,
                        bool t_return_value)
    : m_type_info(ti),
      m_obj(std::move(to)),
      m_data_ptr(ti.is_const() ? nullptr : const_cast<void *>(t_void_ptr)),
      m_const_data_ptr(t_void_ptr),
      m_is_ref(is_ref),
      m_return_value(t_return_value) {}

Boxed_Value::Data &Boxed_Value::Data::operator=(const Data &rhs) {
    m_type_info = rhs.m_type_info;
    m_obj = rhs.m_obj;
    m_is_ref = rhs.m_is_ref;
    m_data_ptr = rhs.m_data_ptr;
    m_const_data_ptr = rhs.m_const_data_ptr;
    m_return_value = rhs.m_return_value;

    if (rhs.m_attrs) {
        m_attrs =
            std::make_unique<std::map<std::string, std::shared_ptr<Data>>>(
                *rhs.m_attrs);
    }

    return *this;
}

Boxed_Value::Boxed_Value(std::shared_ptr<Data> t_data, Internal_Construction)
        : m_data(std::move(t_data)) {}

void Boxed_Value::swap(Boxed_Value &rhs) noexcept {
    std::swap(m_data, rhs.m_data);
}

Boxed_Value Boxed_Value::assign(const Boxed_Value &rhs) noexcept {
    (*m_data) = (*rhs.m_data);
    return *this;
}

const Type_Info &Boxed_Value::get_type_info() const noexcept {
    return m_data->m_type_info;
}

/// return true if the object is uninitialized
bool Boxed_Value::is_undef() const noexcept {
    return m_data->m_type_info.is_undef();
}

bool Boxed_Value::is_const() const noexcept {
    return m_data->m_type_info.is_const();
}

bool Boxed_Value::is_type(const Type_Info &ti) const noexcept {
    return m_data->m_type_info.bare_equal(ti);
}

bool Boxed_Value::is_null() const noexcept {
    return (m_data->m_data_ptr == nullptr &&
            m_data->m_const_data_ptr == nullptr);
}

const Carbon::detail::Any &Boxed_Value::get() const noexcept {
    return m_data->m_obj;
}

bool Boxed_Value::is_ref() const noexcept { return m_data->m_is_ref; }

bool Boxed_Value::is_return_value() const noexcept {
    return m_data->m_return_value;
}

void Boxed_Value::reset_return_value() const noexcept {
    m_data->m_return_value = false;
}

bool Boxed_Value::is_pointer() const noexcept { return !is_ref(); }

void *Boxed_Value::get_ptr() const noexcept { return m_data->m_data_ptr; }

const void *Boxed_Value::get_const_ptr() const noexcept {
    return m_data->m_const_data_ptr;
}

Boxed_Value Boxed_Value::get_attr(const std::string &t_name) {
    if (!m_data->m_attrs) {
        m_data->m_attrs =
            std::make_unique<std::map<std::string, std::shared_ptr<Data>>>();
    }

    auto &attr = (*m_data->m_attrs)[t_name];
    if (attr) {
        return Boxed_Value(attr, Internal_Construction());
    } else {
        Boxed_Value bv;  // default construct a new one
        attr = bv.m_data;
        return bv;
    }
}

Boxed_Value &Boxed_Value::copy_attrs(const Boxed_Value &t_obj) {
    if (t_obj.m_data->m_attrs) {
        m_data->m_attrs =
            std::make_unique<std::map<std::string, std::shared_ptr<Data>>>(
                *t_obj.m_data->m_attrs);
    }
    return *this;
}

Boxed_Value &Boxed_Value::clone_attrs(const Boxed_Value &t_obj) {
    copy_attrs(t_obj);
    reset_return_value();
    return *this;
}

/// \returns true if the two Boxed_Values share the same internal type
bool Boxed_Value::type_match(const Boxed_Value &l,
                             const Boxed_Value &r) noexcept {
    return l.get_type_info() == r.get_type_info();
}
}  // namespace Carbon
