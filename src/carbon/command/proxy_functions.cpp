

#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "proxy_functions.hpp"

#include "../defines.hpp"
#include "boxed_cast.hpp"
#include "boxed_value.hpp"
#include "dynamic_object.hpp"
#include "function_params.hpp"
#include "proxy_functions_detail.hpp"
#include "atom/experiment/type_info.hpp"

namespace Carbon {

namespace dispatch {

Param_Types::Param_Types() : m_has_types(false) {}

Param_Types::Param_Types(std::vector<std::pair<std::string, Type_Info>> t_types)
    : m_types(std::move(t_types)), m_has_types(false) {
    update_has_types();
}

void Param_Types::push_front(const std::string &t_name, Type_Info t_ti) {
    m_types.emplace(m_types.begin(), std::move(t_name), t_ti);
    update_has_types();
}

bool Param_Types::operator==(const Param_Types &t_rhs) const {
    return m_types == t_rhs.m_types;
}

std::vector<Boxed_Value> Param_Types::convert(
    Function_Params t_params,
    const Type_Conversions_State &t_conversions) const {
    auto vals = t_params.to_vector();
    const auto dynamic_object_type_info = user_type<Dynamic_Object>();
    for (size_t i = 0; i < vals.size(); ++i) {
        const auto &name = m_types[i].first;
        if (!name.empty()) {
            const auto &bv = vals[i];

            if (!bv.get_type_info().bare_equal(dynamic_object_type_info)) {
                const auto &ti = m_types[i].second;
                if (!ti.is_undef()) {
                    if (!bv.get_type_info().bare_equal(ti)) {
                        if (t_conversions->converts(ti, bv.get_type_info())) {
                            try {
                                // We will not catch any
                                // bad_boxed_dynamic_cast that is thrown,
                                // let the user get it either way, we are
                                // not responsible if it doesn't work
                                vals[i] = t_conversions->boxed_type_conversion(
                                    m_types[i].second, t_conversions.saves(),
                                    vals[i]);
                            } catch (...) {
                                try {
                                    // try going the other way
                                    vals[i] =
                                        t_conversions
                                            ->boxed_type_down_conversion(
                                                m_types[i].second,
                                                t_conversions.saves(), vals[i]);
                                } catch (const Carbon::detail::exception::
                                             bad_any_cast &) {
                                    throw exception::bad_boxed_cast(
                                        bv.get_type_info(),
                                        *m_types[i].second.bare_type_info());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return vals;
}

// first result: is a match
// second result: needs conversions
std::pair<bool, bool> Param_Types::match(
    const Function_Params &vals,
    const Type_Conversions_State &t_conversions) const {
    const auto dynamic_object_type_info = user_type<Dynamic_Object>();
    bool needs_conversion = false;

    if (!m_has_types) {
        return std::make_pair(true, needs_conversion);
    }
    if (vals.size() != m_types.size()) {
        return std::make_pair(false, needs_conversion);
    }

    for (size_t i = 0; i < vals.size(); ++i) {
        const auto &name = m_types[i].first;
        if (!name.empty()) {
            const auto &bv = vals[i];

            if (bv.get_type_info().bare_equal(dynamic_object_type_info)) {
                try {
                    const Dynamic_Object &d =
                        boxed_cast<const Dynamic_Object &>(bv, &t_conversions);
                    if (!(name == "Dynamic_Object" ||
                          d.get_type_name() == name)) {
                        return std::make_pair(false, false);
                    }
                } catch (const std::bad_cast &) {
                    return std::make_pair(false, false);
                }
            } else {
                const auto &ti = m_types[i].second;
                if (!ti.is_undef()) {
                    if (!bv.get_type_info().bare_equal(ti)) {
                        if (!t_conversions->converts(ti, bv.get_type_info())) {
                            return std::make_pair(false, false);
                        } else {
                            needs_conversion = true;
                        }
                    }
                } else {
                    return std::make_pair(false, false);
                }
            }
        }
    }

    return std::make_pair(true, needs_conversion);
}

const std::vector<std::pair<std::string, Type_Info>> &Param_Types::types()
    const {
    return m_types;
}

void Param_Types::update_has_types() {
    for (const auto &type : m_types) {
        if (!type.first.empty()) {
            m_has_types = true;
            return;
        }
    }

    m_has_types = false;
}

Boxed_Value Proxy_Function_Base::operator()(
    const Function_Params &params,
    const Carbon::Type_Conversions_State &t_conversions) const {
    if (m_arity < 0 || size_t(m_arity) == params.size()) {
        return do_call(params, t_conversions);
    } else {
        throw exception::arity_error(static_cast<int>(params.size()), m_arity);
    }
}

/// Returns a vector containing all of the types of the parameters the
/// function returns/takes if the function is variadic or takes no arguments
/// (arity of 0 or -1), the returned value contains exactly 1 Type_Info
/// object: the return type \returns the types of all parameters.
const std::vector<Type_Info> &Proxy_Function_Base::get_param_types() const {
    return m_types;
}

bool Proxy_Function_Base::is_attribute_function() const;
{ return false; }

bool Proxy_Function_Base::has_arithmetic_param() const {
    return m_has_arithmetic_param;
}

std::vector<std::shared_ptr<const Proxy_Function_Base>>
Proxy_Function_Base::get_contained_functions() const {
    return std::vector<std::shared_ptr<const Proxy_Function_Base>>();
}

//! Return true if the function is a possible match
//! to the passed in values
bool Proxy_Function_Base::filter(
    const Function_Params &vals,
    const Type_Conversions_State &t_conversions) const {
    assert(m_arity == -1 ||
           (m_arity > 0 && static_cast<int>(vals.size()) == m_arity));

    if (m_arity < 0) {
        return true;
    } else if (m_arity > 1) {
        return compare_type_to_param(m_types[1], vals[0], t_conversions) &&
               compare_type_to_param(m_types[2], vals[1], t_conversions);
    } else {
        return compare_type_to_param(m_types[1], vals[0], t_conversions);
    }
}

/// \returns the number of arguments the function takes or -1 if it is
/// variadic
int Proxy_Function_Base::get_arity() const { return m_arity; }

bool Proxy_Function_Base::compare_type_to_param(
    const Type_Info &ti, const Boxed_Value &bv,
    const Type_Conversions_State &t_conversions) {
    const auto boxed_value_ti = user_type<Boxed_Value>();
    const auto boxed_number_ti = user_type<Boxed_Number>();
    const auto function_ti =
        user_type<std::shared_ptr<const Proxy_Function_Base>>();

    if (ti.is_undef() || ti.bare_equal(boxed_value_ti) ||
        (!bv.get_type_info().is_undef() &&
         ((ti.bare_equal(boxed_number_ti) &&
           bv.get_type_info().is_arithmetic()) ||
          ti.bare_equal(bv.get_type_info()) ||
          bv.get_type_info().bare_equal(function_ti) ||
          t_conversions->converts(ti, bv.get_type_info())))) {
        return true;
    } else {
        return false;
    }
}

bool Proxy_Function_Base::compare_first_type(
    const Boxed_Value &bv, const Type_Conversions_State &t_conversions) const {
    /// TODO is m_types guaranteed to be at least 2??
    return compare_type_to_param(m_types[1], bv, t_conversions);
}

Boxed_Value Proxy_Function_Base::do_call(
    const Function_Params &params,
    const Type_Conversions_State &t_conversions) const = 0;

Proxy_Function_Base::Proxy_Function_Base(std::vector<Type_Info> t_types,
                                         int t_arity)
    : m_types(std::move(t_types)),
      m_arity(t_arity),
      m_has_arithmetic_param(false) {
    for (size_t i = 1; i < m_types.size(); ++i) {
        if (m_types[i].is_arithmetic()) {
            m_has_arithmetic_param = true;
            return;
        }
    }
}

bool Proxy_Function_Base::compare_types(
    const std::vector<Type_Info> &tis, const Function_Params &bvs,
    const Type_Conversions_State &t_conversions) {
    if (tis.size() - 1 != bvs.size()) {
        return false;
    } else {
        const size_t size = bvs.size();
        for (size_t i = 0; i < size; ++i) {
            if (!compare_type_to_param(tis[i + 1], bvs[i], t_conversions)) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace dispatch

namespace dispatch {
Dynamic_Proxy_Function::Dynamic_Proxy_Function(
    const int t_arity, std::shared_ptr<AST_Node> t_parsenode,
    Param_Types t_param_types = Param_Types(),
    Proxy_Function t_guard = Proxy_Function())
    : Proxy_Function_Base(build_param_type_list(t_param_types), t_arity),
      m_param_types(std::move(t_param_types)),
      m_guard(std::move(t_guard)),
      m_parsenode(std::move(t_parsenode)) {
    // assert(t_parsenode);
}

bool Dynamic_Proxy_Function::operator==(const Proxy_Function_Base &rhs) const {
    const Dynamic_Proxy_Function *prhs =
        dynamic_cast<const Dynamic_Proxy_Function *>(&rhs);

    return this == &rhs ||
           ((prhs != nullptr) && this->m_arity == prhs->m_arity &&
            !this->m_guard && !prhs->m_guard &&
            this->m_param_types == prhs->m_param_types);
}

bool Dynamic_Proxy_Function::call_match(
    const Function_Params &vals,
    const Type_Conversions_State &t_conversions) const {
    return call_match_internal(vals, t_conversions).first;
}

bool Dynamic_Proxy_Function::has_guard() const { return bool(m_guard); }

Proxy_Function Dynamic_Proxy_Function::get_guard() const { return m_guard; }

bool Dynamic_Proxy_Function::has_parse_tree() const {
    return static_cast<bool>(m_parsenode);
}

const AST_Node &Dynamic_Proxy_Function::get_parse_tree() const {
    if (m_parsenode) {
        return *m_parsenode;
    } else {
        throw std::runtime_error(
            "Dynamic_Proxy_Function does not have parse_tree");
    }
}

bool Dynamic_Proxy_Function::test_guard(
    const Function_Params &params,
    const Type_Conversions_State &t_conversions) const {
    if (m_guard) {
        try {
            return boxed_cast<bool>((*m_guard)(params, t_conversions));
        } catch (const exception::arity_error &) {
            return false;
        } catch (const exception::bad_boxed_cast &) {
            return false;
        }
    } else {
        return true;
    }
}

// first result: is a match
// second result: needs conversions
std::pair<bool, bool> Dynamic_Proxy_Function::call_match_internal(
    const Function_Params &vals,
    const Type_Conversions_State &t_conversions) const {
    const auto comparison_result = [&]() {
        if (m_arity < 0) {
            return std::make_pair(true, false);
        } else if (vals.size() == size_t(m_arity)) {
            return m_param_types.match(vals, t_conversions);
        } else {
            return std::make_pair(false, false);
        }
    }();

    return std::make_pair(
        comparison_result.first && test_guard(vals, t_conversions),
        comparison_result.second);
}

std::vector<Type_Info> Dynamic_Proxy_Function::build_param_type_list(
    const Param_Types &t_types) {
    // For the return type
    std::vector<Type_Info> types{Get_Type_Info<Boxed_Value>::get()};

    for (const auto &t : t_types.types()) {
        if (t.second.is_undef()) {
            types.push_back(Get_Type_Info<Boxed_Value>::get());
        } else {
            types.push_back(t.second);
        }
    }

    return types;
}

Bound_Function::Bound_Function(const Const_Proxy_Function &t_f,
                               const std::vector<Boxed_Value> &t_args)
    : Proxy_Function_Base(
          build_param_type_info(t_f, t_args),
          (t_f->get_arity() < 0
               ? -1
               : static_cast<int>(build_param_type_info(t_f, t_args).size()) -
                     1)),
      m_f(t_f),
      m_args(t_args) {
    assert(m_f->get_arity() < 0 ||
           m_f->get_arity() == static_cast<int>(m_args.size()));
}

bool Bound_Function::operator==(const Proxy_Function_Base &t_f) const {
    return &t_f == this;
}

bool Bound_Function::call_match(
    const Function_Params &vals,
    const Type_Conversions_State &t_conversions) const {
    return m_f->call_match(Function_Params(build_param_list(vals)),
                           t_conversions);
}

std::vector<Const_Proxy_Function> Bound_Function::get_contained_functions()
    const {
    return std::vector<Const_Proxy_Function>{m_f};
}

std::vector<Boxed_Value> Bound_Function::build_param_list(
    const Function_Params &params) const {
    auto parg = params.begin();
    auto barg = m_args.begin();

    std::vector<Boxed_Value> args;

    while (!(parg == params.end() && barg == m_args.end())) {
        while (barg != m_args.end() &&
               !(barg->get_type_info() ==
                 Get_Type_Info<Placeholder_Object>::get())) {
            args.push_back(*barg);
            ++barg;
        }

        if (parg != params.end()) {
            args.push_back(*parg);
            ++parg;
        }

        if (barg != m_args.end() &&
            barg->get_type_info() == Get_Type_Info<Placeholder_Object>::get()) {
            ++barg;
        }
    }
    return args;
}

std::vector<Type_Info> Bound_Function::build_param_type_info(
    const Const_Proxy_Function &t_f, const std::vector<Boxed_Value> &t_args) {
    assert(t_f->get_arity() < 0 ||
           t_f->get_arity() == static_cast<int>(t_args.size()));

    if (t_f->get_arity() < 0) {
        return std::vector<Type_Info>();
    }

    const auto types = t_f->get_param_types();
    assert(types.size() == t_args.size() + 1);

    // this analysis warning is invalid in MSVC12 and doesn't exist in
    // MSVC14
    std::vector<Type_Info> retval{types[0]};

    for (size_t i = 0; i < types.size() - 1; ++i) {
        if (t_args[i].get_type_info() ==
            Get_Type_Info<Placeholder_Object>::get()) {
            retval.push_back(types[i + 1]);
        }
    }

    return retval;
}

Boxed_Value Bound_Function::do_call(
    const Function_Params &params,
    const Type_Conversions_State &t_conversions) const {
    return (*m_f)(Function_Params{build_param_list(params)}, t_conversions);
}
Proxy_Function_Impl_Base::Proxy_Function_Impl_Base(
    const std::vector<Type_Info> &t_types)
    : Proxy_Function_Base(t_types, static_cast<int>(t_types.size()) - 1) {}

bool Proxy_Function_Impl_Base::call_match(
    const Function_Params &vals,
    const Type_Conversions_State &t_conversions) const {
    return static_cast<int>(vals.size()) == get_arity() &&
           (compare_types(m_types, vals, t_conversions) &&
            compare_types_with_cast(vals, t_conversions));
}

}  // namespace dispatch

}  // namespace Carbon
