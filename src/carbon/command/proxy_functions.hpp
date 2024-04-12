
#ifndef CARBON_PROXY_FUNCTIONS_HPP
#define CARBON_PROXY_FUNCTIONS_HPP

#include <cassert>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "../defines.hpp"
#include "atom/experiment/type_info.hpp"
#include "boxed_cast.hpp"
#include "boxed_value.hpp"
#include "dynamic_object.hpp"
#include "function_params.hpp"
#include "proxy_functions_detail.hpp"

namespace Carbon {
class Type_Conversions;
namespace exception {
class bad_boxed_cast;
struct arity_error;
}  // namespace exception
class Boxed_Number;
struct AST_Node;

using AST_NodePtr = std::unique_ptr<AST_Node>;

namespace dispatch {
template <typename FunctionType>
std::function<FunctionType> functor(
    std::shared_ptr<const Proxy_Function_Base> func,
    const Type_Conversions_State *t_conversions);

/**
 * @brief A class representing the parameter types of a function.
 *
 * This class stores information about the types of parameters in a function.
 */
class Param_Types {
public:
    /**
     * @brief Default constructor.
     */
    Param_Types();

    /**
     * @brief Constructor with parameters.
     *
     * @param t_types A vector of pairs containing parameter names and type
     * information.
     */
    explicit Param_Types(
        std::vector<std::pair<std::string, Type_Info>> t_types);

    /**
     * @brief Pushes a new parameter type to the front of the list.
     *
     * @param t_name The name of the parameter.
     * @param t_ti The type information of the parameter.
     */
    void push_front(const std::string &t_name, Type_Info t_ti);

    /**
     * @brief Checks if two Param_Types objects are equal.
     *
     * @param t_rhs The right-hand side Param_Types object.
     * @return True if the objects are equal, false otherwise.
     */
    bool operator==(const Param_Types &t_rhs) const noexcept;

    /**
     * @brief Converts function parameters to boxed values using type
     * conversions.
     *
     * @param t_params The function parameters.
     * @param t_conversions The state of type conversions.
     * @return A vector of boxed values representing the converted parameters.
     */
    std::vector<Boxed_Value> convert(
        Function_Params t_params,
        const Type_Conversions_State &t_conversions) const;

    /**
     * @brief Checks if given values match the parameter types.
     *
     * @param vals The values to check.
     * @param t_conversions The state of type conversions.
     * @return A pair of booleans indicating if there is a match and if
     * conversions are needed.
     */
    std::pair<bool, bool> match(
        const Function_Params &vals,
        const Type_Conversions_State &t_conversions) const noexcept;

    /**
     * @brief Gets the vector of parameter types.
     *
     * @return A vector of pairs containing parameter names and type
     * information.
     */
    const std::vector<std::pair<std::string, Type_Info>> &types()
        const noexcept;

private:
    /**
     * @brief Updates the flag indicating whether the Param_Types object has
     * types.
     */
    void update_has_types();

    std::vector<std::pair<std::string, Type_Info>>
        m_types;       ///< Vector of parameter names and type information.
    bool m_has_types;  ///< Flag indicating whether the Param_Types object has
                       ///< types.
};

/**
 * @brief Pure virtual base class for all Proxy_Function implementations.
 *
 * Proxy_Functions are a type erasure of type safe C++ function calls. At
 * runtime, parameter types are expected to be tested against passed in types.
 * Dispatch_Engine only knows how to work with Proxy_Function, no other
 * function classes.
 */
class Proxy_Function_Base {
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~Proxy_Function_Base() = default;

    /**
     * @brief Calls the function with the given parameters.
     *
     * @param params The parameters to pass to the function.
     * @param t_conversions The state of type conversions.
     * @return The result of the function call.
     */
    Boxed_Value operator()(
        const Function_Params &params,
        const Carbon::Type_Conversions_State &t_conversions) const;

    /**
     * @brief Returns the types of all parameters.
     *
     * If the function is variadic or takes no arguments (arity of 0 or -1),
     * the returned value contains exactly 1 Type_Info object: the return type.
     *
     * @return A vector containing all of the types of the parameters the
     * function returns/takes.
     */
    const std::vector<Type_Info> &get_param_types() const noexcept;

    /**
     * @brief Checks if two Proxy_Function_Base objects are equal.
     *
     * @param rhs The right-hand side Proxy_Function_Base object.
     * @return True if the objects are equal, false otherwise.
     */
    virtual bool operator==(const Proxy_Function_Base &) const noexcept = 0;

    /**
     * @brief Checks if the function matches the given parameters.
     *
     * @param vals The values to check.
     * @param t_conversions The state of type conversions.
     * @return True if the function matches, false otherwise.
     */
    virtual bool call_match(
        const Function_Params &vals,
        const Type_Conversions_State &t_conversions) const = 0;

    /**
     * @brief Checks if the function is an attribute function.
     *
     * @return True if the function is an attribute function, false otherwise.
     */
    virtual bool is_attribute_function() const noexcept;

    /**
     * @brief Checks if the function has an arithmetic parameter.
     *
     * @return True if the function has an arithmetic parameter, false
     * otherwise.
     */
    bool has_arithmetic_param() const noexcept;

    /**
     * @brief Gets the contained functions.
     *
     * @return A vector containing shared pointers to the contained functions.
     */
    virtual std::vector<std::shared_ptr<const Proxy_Function_Base>>
    get_contained_functions() const;

    /**
     * @brief Filters the function based on the passed in values.
     *
     * @param vals The values to filter.
     * @param t_conversions The state of type conversions.
     * @return True if the function is a possible match to the passed in values,
     * false otherwise.
     */
    bool filter(const Function_Params &vals,
                const Type_Conversions_State &t_conversions) const noexcept;

    /**
     * @brief Gets the number of arguments the function takes.
     *
     * @return The number of arguments the function takes, or -1 if it is
     * variadic.
     */
    int get_arity() const noexcept;

    /**
     * @brief Compares a type to the first parameter.
     *
     * @param bv The boxed value to compare.
     * @param t_conversions The state of type conversions.
     * @return True if the type matches the first parameter, false otherwise.
     */
    static bool compare_type_to_param(
        const Type_Info &ti, const Boxed_Value &bv,
        const Type_Conversions_State &t_conversions) noexcept;

    virtual bool compare_first_type(
        const Boxed_Value &bv,
        const Type_Conversions_State &t_conversions) const noexcept {
        /// TODO is m_types guaranteed to be at least 2??
        return compare_type_to_param(m_types[1], bv, t_conversions);
    }

protected:
    /**
     * @brief Performs the function call.
     *
     * @param params The parameters to pass to the function.
     * @param t_conversions The state of type conversions.
     * @return The result of the function call.
     */
    virtual Boxed_Value do_call(
        const Function_Params &params,
        const Type_Conversions_State &t_conversions) const = 0;

    /**
     * @brief Constructor.
     *
     * @param t_types The types of the parameters.
     * @param t_arity The arity of the function.
     */
    Proxy_Function_Base(std::vector<Type_Info> t_types, int t_arity);

    /**
     * @brief Compares types to parameters.
     *
     * @param tis The types to compare.
     * @param bvs The parameters to compare against.
     * @param t_conversions The state of type conversions.
     * @return True if the types match the parameters, false otherwise.
     */
    static bool compare_types(
        const std::vector<Type_Info> &tis, const Function_Params &bvs,
        const Type_Conversions_State &t_conversions) noexcept;

    std::vector<Type_Info> m_types;  ///< Vector of parameter types.
    int m_arity;                     ///< The arity of the function.
    bool m_has_arithmetic_param;     ///< Flag indicating if the function has an
                                     ///< arithmetic parameter.
};
}  // namespace dispatch

/// \brief Common typedef used for passing of any registered function in
/// ChaiScript
using Proxy_Function = std::shared_ptr<dispatch::Proxy_Function_Base>;

/// \brief Const version of Proxy_Function. Points to a const Proxy_Function.
/// This is how most registered functions
///        are handled internally.
using Const_Proxy_Function =
    std::shared_ptr<const dispatch::Proxy_Function_Base>;

namespace exception {
/// \brief  Exception thrown if a function's guard fails
class guard_error : public std::runtime_error {
public:
    guard_error() noexcept : std::runtime_error("Guard evaluation failed") {}

    guard_error(const guard_error &) = default;

    ~guard_error() noexcept override = default;
};
}  // namespace exception

namespace dispatch {
/**
 * @brief A Proxy_Function implementation that is not type safe.
 *
 * The called function is expecting a vector<Boxed_Value> that it works with how
 * it chooses.
 */
class Dynamic_Proxy_Function : public Proxy_Function_Base {
public:
    /**
     * @brief Constructor.
     *
     * @param t_arity The arity of the function.
     * @param t_parsenode The parse node.
     * @param t_param_types The parameter types.
     * @param t_guard The guard function.
     */
    explicit Dynamic_Proxy_Function(const int t_arity,
                                    std::shared_ptr<AST_Node> t_parsenode,
                                    Param_Types t_param_types = Param_Types(),
                                    Proxy_Function t_guard = Proxy_Function());

    /**
     * @brief Checks if two Dynamic_Proxy_Function objects are equal.
     *
     * @param rhs The right-hand side Dynamic_Proxy_Function object.
     * @return True if the objects are equal, false otherwise.
     */
    bool operator==(const Proxy_Function_Base &rhs) const noexcept override;

    /**
     * @brief Checks if the function matches the given parameters.
     *
     * @param vals The values to check.
     * @param t_conversions The state of type conversions.
     * @return True if the function matches, false otherwise.
     */
    bool call_match(const Function_Params &vals,
                    const Type_Conversions_State &t_conversions) const override;

    /**
     * @brief Checks if the function has a guard.
     *
     * @return True if the function has a guard, false otherwise.
     */
    bool has_guard() const noexcept;

    /**
     * @brief Gets the guard function.
     *
     * @return The guard function.
     */
    Proxy_Function get_guard() const noexcept;

    /**
     * @brief Checks if the function has a parse tree.
     *
     * @return True if the function has a parse tree, false otherwise.
     */
    bool has_parse_tree() const noexcept;

    /**
     * @brief Gets the parse tree.
     *
     * @return The parse tree.
     */
    const AST_Node &get_parse_tree() const;

protected:
    /**
     * @brief Tests the guard function.
     *
     * @param params The parameters to test.
     * @param t_conversions The state of type conversions.
     * @return True if the guard function passes, false otherwise.
     */
    bool test_guard(const Function_Params &params,
                    const Type_Conversions_State &t_conversions) const;

    /**
     * @brief Checks if the function matches the given parameters internally.
     *
     * @param vals The values to check.
     * @param t_conversions The state of type conversions.
     * @return A pair of booleans indicating if there is a match and if
     * conversions are needed.
     */
    std::pair<bool, bool> call_match_internal(
        const Function_Params &vals,
        const Type_Conversions_State &t_conversions) const;

private:
    /**
     * @brief Builds a list of parameter types.
     *
     * @param t_types The parameter types.
     * @return A vector containing the parameter types.
     */
    static std::vector<Type_Info> build_param_type_list(
        const Param_Types &t_types);

protected:
    Param_Types m_param_types;  ///< The parameter types.

private:
    Proxy_Function m_guard;                 ///< The guard function.
    std::shared_ptr<AST_Node> m_parsenode;  ///< The parse node.
};

template <typename Callable>
class Dynamic_Proxy_Function_Impl final : public Dynamic_Proxy_Function {
public:
    Dynamic_Proxy_Function_Impl(
        Callable t_f, int t_arity = -1,
        std::shared_ptr<AST_Node> t_parsenode = AST_NodePtr(),
        Param_Types t_param_types = Param_Types(),
        Proxy_Function t_guard = Proxy_Function())
        : Dynamic_Proxy_Function(t_arity, std::move(t_parsenode),
                                 std::move(t_param_types), std::move(t_guard)),
          m_f(std::move(t_f)) {}

protected:
    Boxed_Value do_call(
        const Function_Params &params,
        const Type_Conversions_State &t_conversions) const override {
        const auto [is_a_match, needs_conversions] =
            call_match_internal(params, t_conversions);
        if (is_a_match) {
            if (needs_conversions) {
                return m_f(Function_Params{
                    m_param_types.convert(params, t_conversions)});
            } else {
                return m_f(params);
            }
        } else {
            throw exception::guard_error();
        }
    }

private:
    Callable m_f;
};

template <typename Callable, typename... Arg>
Proxy_Function make_dynamic_proxy_function(Callable &&c, Arg &&...a) {
    return Carbon::make_shared<dispatch::Proxy_Function_Base,
                               dispatch::Dynamic_Proxy_Function_Impl<Callable>>(
        std::forward<Callable>(c), std::forward<Arg>(a)...);
}

/// An object used by Bound_Function to represent "_" parameters
/// of a binding. This allows for unbound parameters during bind.
struct Placeholder_Object {};

/**
 * @brief An implementation of Proxy_Function that takes a Proxy_Function
 * and substitutes bound parameters into the parameter list
 * at runtime, when call() is executed.
 * It is used for bind(function, param1, _, param2) style calls.
 */
class Bound_Function final : public Proxy_Function_Base {
public:
    /**
     * @brief Constructor.
     *
     * @param t_f The original function.
     * @param t_args The arguments to bind.
     */
    Bound_Function(const Const_Proxy_Function &t_f,
                   const std::vector<Boxed_Value> &t_args);

    /**
     * @brief Checks if two Bound_Function objects are equal.
     *
     * @param t_f The right-hand side Bound_Function object.
     * @return True if the objects are equal, false otherwise.
     */
    bool operator==(const Proxy_Function_Base &t_f) const noexcept override;

    /**
     * @brief Checks if the function matches the given parameters.
     *
     * @param vals The values to check.
     * @param t_conversions The state of type conversions.
     * @return True if the function matches, false otherwise.
     */
    bool call_match(const Function_Params &vals,
                    const Type_Conversions_State &t_conversions) const override;

    /**
     * @brief Gets the contained functions.
     *
     * @return A vector containing shared pointers to the contained functions.
     */
    std::vector<Const_Proxy_Function> get_contained_functions() const override;

    std::vector<Boxed_Value> build_param_list(
        const Function_Params &params) const;

protected:
    /**
     * @brief Builds parameter type information.
     *
     * @param t_f The original function.
     * @param t_args The arguments.
     * @return A vector containing the parameter type information.
     */
    static std::vector<Type_Info> build_param_type_info(
        const Const_Proxy_Function &t_f,
        const std::vector<Boxed_Value> &t_args);

    /**
     * @brief Executes the function call.
     *
     * @param params The parameters to pass to the function.
     * @param t_conversions The state of type conversions.
     * @return The result of the function call.
     */
    Boxed_Value do_call(
        const Function_Params &params,
        const Type_Conversions_State &t_conversions) const override;

private:
    Const_Proxy_Function m_f;         ///< The original function.
    std::vector<Boxed_Value> m_args;  ///< The bound arguments.
};

/**
 * @brief Base class for Proxy_Function implementations.
 */
class Proxy_Function_Impl_Base : public Proxy_Function_Base {
public:
    /**
     * @brief Constructor.
     *
     * @param t_types The types of the parameters.
     */
    explicit Proxy_Function_Impl_Base(const std::vector<Type_Info> &t_types);

    /**
     * @brief Checks if the function matches the given parameters.
     *
     * @param vals The values to check.
     * @param t_conversions The state of type conversions.
     * @return True if the function matches, false otherwise.
     */
    bool call_match(
        const Function_Params &vals,
        const Type_Conversions_State &t_conversions) const noexcept override;

    /**
     * @brief Compares types with casting.
     *
     * @param vals The values to compare.
     * @param t_conversions The state of type conversions.
     * @return True if the types match with casting, false otherwise.
     */
    virtual bool compare_types_with_cast(
        const Function_Params &vals,
        const Type_Conversions_State &t_conversions) const noexcept = 0;
};

/// For any callable object
template <typename Func, typename Callable>
class Proxy_Function_Callable_Impl final : public Proxy_Function_Impl_Base {
public:
    explicit Proxy_Function_Callable_Impl(Callable f)
        : Proxy_Function_Impl_Base(
              detail::build_param_type_list(static_cast<Func *>(nullptr))),
          m_f(std::move(f)) {}

    bool compare_types_with_cast(
        const Function_Params &vals,
        const Type_Conversions_State &t_conversions) const noexcept override {
        return detail::compare_types_cast(static_cast<Func *>(nullptr), vals,
                                          t_conversions);
    }

    bool operator==(const Proxy_Function_Base &t_func) const noexcept override {
        return dynamic_cast<
                   const Proxy_Function_Callable_Impl<Func, Callable> *>(
                   &t_func) != nullptr;
    }

protected:
    Boxed_Value do_call(
        const Function_Params &params,
        const Type_Conversions_State &t_conversions) const override {
        return detail::call_func(static_cast<Func *>(nullptr), m_f, params,
                                 t_conversions);
    }

private:
    Callable m_f;
};

class Assignable_Proxy_Function : public Proxy_Function_Impl_Base {
public:
    explicit Assignable_Proxy_Function(const std::vector<Type_Info> &t_types)
        : Proxy_Function_Impl_Base(t_types) {}

    virtual void assign(
        const std::shared_ptr<const Proxy_Function_Base> &t_rhs) = 0;
};

template <typename Func>
class Assignable_Proxy_Function_Impl final : public Assignable_Proxy_Function {
public:
    Assignable_Proxy_Function_Impl(
        std::reference_wrapper<std::function<Func>> t_f,
        std::shared_ptr<std::function<Func>> t_ptr)
        : Assignable_Proxy_Function(
              detail::build_param_type_list(static_cast<Func *>(nullptr))),
          m_f(std::move(t_f)),
          m_shared_ptr_holder(std::move(t_ptr)) {
        assert(!m_shared_ptr_holder || m_shared_ptr_holder.get() == &m_f.get());
    }

    bool compare_types_with_cast(
        const Function_Params &vals,
        const Type_Conversions_State &t_conversions) const noexcept override {
        return detail::compare_types_cast(static_cast<Func *>(nullptr), vals,
                                          t_conversions);
    }

    bool operator==(const Proxy_Function_Base &t_func) const noexcept override {
        return dynamic_cast<const Assignable_Proxy_Function_Impl<Func> *>(
                   &t_func) != nullptr;
    }

    std::function<Func> internal_function() const { return m_f.get(); }

    void assign(
        const std::shared_ptr<const Proxy_Function_Base> &t_rhs) override {
        m_f.get() = dispatch::functor<Func>(t_rhs, nullptr);
    }

protected:
    Boxed_Value do_call(
        const Function_Params &params,
        const Type_Conversions_State &t_conversions) const override {
        return detail::call_func(static_cast<Func *>(nullptr), m_f.get(),
                                 params, t_conversions);
    }

private:
    std::reference_wrapper<std::function<Func>> m_f;
    std::shared_ptr<std::function<Func>> m_shared_ptr_holder;
};

/// Attribute getter Proxy_Function implementation
template <typename T, typename Class>
class Attribute_Access final : public Proxy_Function_Base {
public:
    explicit Attribute_Access(T Class::*t_attr)
        : Proxy_Function_Base(param_types(), 1), m_attr(t_attr) {}

    bool is_attribute_function() const noexcept override { return true; }

    bool operator==(const Proxy_Function_Base &t_func) const noexcept override {
        const Attribute_Access<T, Class> *aa =
            dynamic_cast<const Attribute_Access<T, Class> *>(&t_func);

        if (aa) {
            return m_attr == aa->m_attr;
        } else {
            return false;
        }
    }

    bool call_match(const Function_Params &vals,
                    const Type_Conversions_State &) const noexcept override {
        if (vals.size() != 1) {
            return false;
        }
        const auto class_type_info = user_type<Class>();
        return vals[0].get_type_info().bare_equal(class_type_info);
    }

protected:
    Boxed_Value do_call(
        const Function_Params &params,
        const Type_Conversions_State &t_conversions) const override {
        const Boxed_Value &bv = params[0];
        if (bv.is_const()) {
            const Class *o = boxed_cast<const Class *>(bv, &t_conversions);
            return do_call_impl<T>(o);
        } else {
            Class *o = boxed_cast<Class *>(bv, &t_conversions);
            return do_call_impl<T>(o);
        }
    }

private:
    template <typename Type>
    auto do_call_impl(Class *o) const {
        if constexpr (std::is_pointer<Type>::value) {
            return detail::Handle_Return<Type>::handle(o->*m_attr);
        } else {
            return detail::Handle_Return<typename std::add_lvalue_reference<
                Type>::type>::handle(o->*m_attr);
        }
    }

    template <typename Type>
    auto do_call_impl(const Class *o) const {
        if constexpr (std::is_pointer<Type>::value) {
            return detail::Handle_Return<const Type>::handle(o->*m_attr);
        } else {
            return detail::Handle_Return<typename std::add_lvalue_reference<
                typename std::add_const<Type>::type>::type>::handle(o->*m_attr);
        }
    }

    static std::vector<Type_Info> param_types() {
        return {user_type<T>(), user_type<Class>()};
    }

    std::vector<Type_Info> m_param_types{user_type<T>(), user_type<Class>()};
    T Class::*m_attr;
};
}  // namespace dispatch

namespace exception {
/// \brief Exception thrown in the case that a method dispatch fails
///        because no matching function was found
///
/// May be thrown due to an arity_error, a guard_error or a bad_boxed_cast
/// exception
class dispatch_error : public std::runtime_error {
public:
    dispatch_error(const Function_Params &t_parameters,
                   std::vector<Const_Proxy_Function> t_functions)
        : std::runtime_error("Error with function dispatch"),
          parameters(t_parameters.to_vector()),
          functions(std::move(t_functions)) {}

    dispatch_error(const Function_Params &t_parameters,
                   std::vector<Const_Proxy_Function> t_functions,
                   const std::string &t_desc)
        : std::runtime_error(t_desc),
          parameters(t_parameters.to_vector()),
          functions(std::move(t_functions)) {}

    dispatch_error(const dispatch_error &) = default;
    ~dispatch_error() noexcept override = default;

    std::vector<Boxed_Value> parameters;
    std::vector<Const_Proxy_Function> functions;
};
}  // namespace exception

namespace dispatch {
namespace detail {
template <typename FuncType>
bool types_match_except_for_arithmetic(
    const FuncType &t_func, const Carbon::Function_Params &plist,
    const Type_Conversions_State &t_conversions) noexcept {
    const std::vector<Type_Info> &types = t_func->get_param_types();

    if (t_func->get_arity() == -1) {
        return false;
    }

    assert(plist.size() == types.size() - 1);

    return std::mismatch(plist.begin(), plist.end(), types.begin() + 1,
                         [&](const Boxed_Value &bv, const Type_Info &ti) {
                             return Proxy_Function_Base::compare_type_to_param(
                                        ti, bv, t_conversions) ||
                                    (bv.get_type_info().is_arithmetic() &&
                                     ti.is_arithmetic());
                         }) == std::make_pair(plist.end(), types.end());
}

template <typename InItr, typename Funcs>
Boxed_Value dispatch_with_conversions(
    InItr begin, const InItr &end, const Carbon::Function_Params &plist,
    const Type_Conversions_State &t_conversions, const Funcs &t_funcs) {
    InItr matching_func(end);

    while (begin != end) {
        if (types_match_except_for_arithmetic(begin->second, plist,
                                              t_conversions)) {
            if (matching_func == end) {
                matching_func = begin;
            } else {
                // handle const members vs non-const member, which is not really
                // ambiguous
                const auto &mat_fun_param_types =
                    matching_func->second->get_param_types();
                const auto &next_fun_param_types =
                    begin->second->get_param_types();

                if (plist[0].is_const() && !mat_fun_param_types[1].is_const() &&
                    next_fun_param_types[1].is_const()) {
                    matching_func =
                        begin;  // keep the new one, the const/non-const matchup
                                // is correct
                } else if (!plist[0].is_const() &&
                           !mat_fun_param_types[1].is_const() &&
                           next_fun_param_types[1].is_const()) {
                    // keep the old one, it has a better const/non-const matchup
                } else {
                    // ambiguous function call
                    throw exception::dispatch_error(
                        plist, std::vector<Const_Proxy_Function>(
                                   t_funcs.begin(), t_funcs.end()));
                }
            }
        }

        ++begin;
    }

    if (matching_func == end) {
        // no appropriate function to attempt arithmetic type conversion on
        throw exception::dispatch_error(
            plist,
            std::vector<Const_Proxy_Function>(t_funcs.begin(), t_funcs.end()));
    }

    std::vector<Boxed_Value> newplist;
    newplist.reserve(plist.size());

    const std::vector<Type_Info> &tis =
        matching_func->second->get_param_types();
    std::transform(
        tis.begin() + 1, tis.end(), plist.begin(), std::back_inserter(newplist),
        [](const Type_Info &ti, const Boxed_Value &param) -> Boxed_Value {
            if (ti.is_arithmetic() && param.get_type_info().is_arithmetic() &&
                param.get_type_info() != ti) {
                return Boxed_Number(param).get_as(ti).bv;
            } else {
                return param;
            }
        });

    try {
        return (*(matching_func->second))(Carbon::Function_Params{newplist},
                                          t_conversions);
    } catch (const exception::bad_boxed_cast &) {
        // parameter failed to cast
    } catch (const exception::arity_error &) {
        // invalid num params
    } catch (const exception::guard_error &) {
        // guard failed to allow the function to execute
    }

    throw exception::dispatch_error(plist, std::vector<Const_Proxy_Function>(
                                               t_funcs.begin(), t_funcs.end()));
}
}  // namespace detail

/// Take a vector of functions and a vector of parameters. Attempt to execute
/// each function against the set of parameters, in order, until a matching
/// function is found or throw dispatch_error if no matching function is found
template <typename Funcs>
Boxed_Value dispatch(const Funcs &funcs, const Function_Params &plist,
                     const Type_Conversions_State &t_conversions) {
    std::vector<std::pair<size_t, const Proxy_Function_Base *>> ordered_funcs;
    ordered_funcs.reserve(funcs.size());

    for (const auto &func : funcs) {
        const auto arity = func->get_arity();

        if (arity == -1) {
            ordered_funcs.emplace_back(plist.size(), func.get());
        } else if (arity == static_cast<int>(plist.size())) {
            size_t numdiffs = 0;
            for (size_t i = 0; i < plist.size(); ++i) {
                if (!func->get_param_types()[i + 1].bare_equal(
                        plist[i].get_type_info())) {
                    ++numdiffs;
                }
            }
            ordered_funcs.emplace_back(numdiffs, func.get());
        }
    }

    for (size_t i = 0; i <= plist.size(); ++i) {
        for (const auto &func : ordered_funcs) {
            try {
                if (func.first == i &&
                    (i == 0 || func.second->filter(plist, t_conversions))) {
                    return (*(func.second))(plist, t_conversions);
                }
            } catch (const exception::bad_boxed_cast &) {
                // parameter failed to cast, try again
            } catch (const exception::arity_error &) {
                // invalid num params, try again
            } catch (const exception::guard_error &) {
                // guard failed to allow the function to execute,
                // try again
            }
        }
    }

    return detail::dispatch_with_conversions(ordered_funcs.cbegin(),
                                             ordered_funcs.cend(), plist,
                                             t_conversions, funcs);
}
}  // namespace dispatch
}  // namespace Carbon

#endif
