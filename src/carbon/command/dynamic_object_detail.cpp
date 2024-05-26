#include "dynamic_object_detail.hpp"

namespace Carbon {
namespace dispatch {
namespace detail {
Dynamic_Object_Function::Dynamic_Object_Function(std::string t_type_name,
                                                 const Proxy_Function &t_func,
                                                 bool t_is_attribute)
    : Proxy_Function_Base(t_func->get_param_types(), t_func->get_arity()),
      m_type_name(std::move(t_type_name)),
      m_func(t_func),
      m_doti(atom::meta::user_type<Dynamic_Object>()),
      m_is_attribute(t_is_attribute) {
    assert((t_func->get_arity() > 0 || t_func->get_arity() < 0) &&
           "Programming error, Dynamic_Object_Function must have at least "
           "one parameter (this)");
}

Dynamic_Object_Function::Dynamic_Object_Function(
    std::string t_type_name, const Proxy_Function &t_func,
    const atom::meta::Type_Info &t_ti, bool t_is_attribute)
    : Proxy_Function_Base(build_param_types(t_func->get_param_types(), t_ti),
                          t_func->get_arity()),
      m_type_name(std::move(t_type_name)),
      m_func(t_func),
      m_ti(t_ti.is_undef() ? nullptr : new atom::meta::Type_Info(t_ti)),
      m_doti(atom::meta::user_type<Dynamic_Object>()),
      m_is_attribute(t_is_attribute) {
    assert((t_func->get_arity() > 0 || t_func->get_arity() < 0) &&
           "Programming error, Dynamic_Object_Function must have at least "
           "one parameter (this)");
}

bool Dynamic_Object_Function::operator==(
    const Proxy_Function_Base &f) const noexcept {
    if (const auto *df = dynamic_cast<const Dynamic_Object_Function *>(&f)) {
        return df->m_type_name == m_type_name && (*df->m_func) == (*m_func);
    } else {
        return false;
    }
}

bool Dynamic_Object_Function::is_attribute_function() const noexcept {
    return m_is_attribute;
}

bool Dynamic_Object_Function::call_match(
    const Carbon::Function_Params &vals,
    const Type_Conversions_State &t_conversions) const noexcept {
    if (dynamic_object_typename_match(vals, m_type_name, m_ti, t_conversions)) {
        return m_func->call_match(vals, t_conversions);
    } else {
        return false;
    }
}

std::vector<Const_Proxy_Function>
Dynamic_Object_Function::get_contained_functions() const {
    return {m_func};
}

Boxed_Value Dynamic_Object_Function::do_call(
    const Carbon::Function_Params &params,
    const Type_Conversions_State &t_conversions) const {
    if (dynamic_object_typename_match(params, m_type_name, m_ti,
                                      t_conversions)) {
        return (*m_func)(params, t_conversions);
    } else {
        throw exception::guard_error();
    }
}

bool Dynamic_Object_Function::compare_first_type(
    const Boxed_Value &bv,
    const Type_Conversions_State &t_conversions) const noexcept {
    return dynamic_object_typename_match(bv, m_type_name, m_ti, t_conversions);
}

std::vector<atom::meta::Type_Info> Dynamic_Object_Function::build_param_types(
    const std::vector<atom::meta::Type_Info> &t_inner_types,
    const atom::meta::Type_Info &t_objectti) {
    std::vector<atom::meta::Type_Info> types(t_inner_types);

    assert(types.size() > 1);
    // assert(types[1].bare_equal(atom::meta::user_type<Boxed_Value>()));
    types[1] = t_objectti;
    return types;
}

bool Dynamic_Object_Function::dynamic_object_typename_match(
    const Boxed_Value &bv, const std::string &name,
    const std::unique_ptr<atom::meta::Type_Info> &ti,
    const Type_Conversions_State &t_conversions) const noexcept {
    if (bv.get_type_info().bare_equal(m_doti)) {
        try {
            const Dynamic_Object &d =
                boxed_cast<const Dynamic_Object &>(bv, &t_conversions);
            return name == "Dynamic_Object" || d.get_type_name() == name;
        } catch (const std::bad_cast &) {
            return false;
        }
    } else {
        if (ti) {
            return bv.get_type_info().bare_equal(*ti);
        } else {
            return false;
        }
    }
}

bool Dynamic_Object_Function::dynamic_object_typename_match(
    const Carbon::Function_Params &bvs, const std::string &name,
    const std::unique_ptr<atom::meta::Type_Info> &ti,
    const Type_Conversions_State &t_conversions) const noexcept {
    if (!bvs.empty()) {
        return dynamic_object_typename_match(bvs[0], name, ti, t_conversions);
    } else {
        return false;
    }
}

Dynamic_Object_Constructor::Dynamic_Object_Constructor(
    std::string t_type_name, const Proxy_Function &t_func)
    : Proxy_Function_Base(build_type_list(t_func->get_param_types()),
                          t_func->get_arity() - 1),
      m_type_name(std::move(t_type_name)),
      m_func(t_func) {
    assert((t_func->get_arity() > 0 || t_func->get_arity() < 0) &&
           "Programming error, Dynamic_Object_Function must have at least "
           "one parameter (this)");
}

std::vector<atom::meta::Type_Info> Dynamic_Object_Constructor::build_type_list(
    const std::vector<atom::meta::Type_Info> &tl) {
    auto begin = tl.begin();
    auto end = tl.end();

    if (begin != end) {
        ++begin;
    }

    return std::vector<atom::meta::Type_Info>(begin, end);
}

bool Dynamic_Object_Constructor::operator==(
    const Proxy_Function_Base &f) const noexcept {
    const Dynamic_Object_Constructor *dc =
        dynamic_cast<const Dynamic_Object_Constructor *>(&f);
    return (dc != nullptr) && dc->m_type_name == m_type_name &&
           (*dc->m_func) == (*m_func);
}

bool Dynamic_Object_Constructor::call_match(
    const Carbon::Function_Params &vals,
    const Type_Conversions_State &t_conversions) const {
    std::vector<Boxed_Value> new_vals{Boxed_Value(Dynamic_Object(m_type_name))};
    new_vals.insert(new_vals.end(), vals.begin(), vals.end());

    return m_func->call_match(Carbon::Function_Params{new_vals}, t_conversions);
}

Boxed_Value Dynamic_Object_Constructor::do_call(
    const Carbon::Function_Params &params,
    const Type_Conversions_State &t_conversions) const {
    auto bv = Boxed_Value(Dynamic_Object(m_type_name), true);
    std::vector<Boxed_Value> new_params{bv};
    new_params.insert(new_params.end(), params.begin(), params.end());

    (*m_func)(Carbon::Function_Params{new_params}, t_conversions);

    return bv;
}

}  // namespace detail
}  // namespace dispatch
}  // namespace Carbon
