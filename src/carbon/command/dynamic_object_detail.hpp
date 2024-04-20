// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009-2012, Jonathan Turner (jonathan@emptycrate.com)
// Copyright 2009-2018, Jason Turner (jason@emptycrate.com)
// http://www.chaiscript.com

#ifndef CARBON_DYNAMIC_OBJECT_DETAIL_HPP
#define CARBON_DYNAMIC_OBJECT_DETAIL_HPP

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include <utility>
#include <vector>

#include "../defines.hpp"
#include "boxed_cast.hpp"
#include "boxed_cast_helper.hpp"
#include "boxed_value.hpp"
#include "dynamic_object.hpp"
#include "proxy_functions.hpp"
#include "atom/experiment/type_info.hpp"

namespace Carbon {
class Type_Conversions;
namespace dispatch {
class Proxy_Function_Base;
}  // namespace dispatch
}  // namespace Carbon

namespace Carbon {
namespace dispatch {
namespace detail {
/// A Proxy_Function implementation designed for calling a function
/// that is automatically guarded based on the first param based on the
/// param's type name
class Dynamic_Object_Function final : public Proxy_Function_Base {
public:
    Dynamic_Object_Function(std::string t_type_name,
                            const Proxy_Function &t_func,
                            bool t_is_attribute = false);

    Dynamic_Object_Function(std::string t_type_name,
                            const Proxy_Function &t_func, const Type_Info &t_ti,
                            bool t_is_attribute = false);

    Dynamic_Object_Function &operator=(const Dynamic_Object_Function) = delete;
    Dynamic_Object_Function(Dynamic_Object_Function &) = delete;

    bool operator==(const Proxy_Function_Base &f) const noexcept override;

    bool is_attribute_function() const noexcept override;

    bool call_match(
        const Carbon::Function_Params &vals,
        const Type_Conversions_State &t_conversions) const noexcept override;

    std::vector<Const_Proxy_Function> get_contained_functions() const override;

protected:
    Boxed_Value do_call(
        const Carbon::Function_Params &params,
        const Type_Conversions_State &t_conversions) const override;

    bool compare_first_type(
        const Boxed_Value &bv,
        const Type_Conversions_State &t_conversions) const noexcept override;

private:
    static std::vector<Type_Info> build_param_types(
        const std::vector<Type_Info> &t_inner_types,
        const Type_Info &t_objectti);

    bool dynamic_object_typename_match(
        const Boxed_Value &bv, const std::string &name,
        const std::unique_ptr<Type_Info> &ti,
        const Type_Conversions_State &t_conversions) const noexcept;

    bool dynamic_object_typename_match(
        const Carbon::Function_Params &bvs, const std::string &name,
        const std::unique_ptr<Type_Info> &ti,
        const Type_Conversions_State &t_conversions) const noexcept;

    std::string m_type_name;
    Proxy_Function m_func;
    std::unique_ptr<Type_Info> m_ti;
    const Type_Info m_doti;
    const bool m_is_attribute;
};

/**
 * A Proxy_Function implementation designed for creating a new
 * Dynamic_Object
 * that is automatically guarded based on the first param based on the
 * param's type name
 */
class Dynamic_Object_Constructor final : public Proxy_Function_Base {
public:
    Dynamic_Object_Constructor(std::string t_type_name,
                               const Proxy_Function &t_func);

    static std::vector<Type_Info> build_type_list(
        const std::vector<Type_Info> &tl);

    bool operator==(const Proxy_Function_Base &f) const noexcept override;

    bool call_match(const Carbon::Function_Params &vals,
                    const Type_Conversions_State &t_conversions) const override;

protected:
    Boxed_Value do_call(
        const Carbon::Function_Params &params,
        const Type_Conversions_State &t_conversions) const override;

private:
    const std::string m_type_name;
    const Proxy_Function m_func;
};
}  // namespace detail
}  // namespace dispatch
}  // namespace Carbon
#endif
