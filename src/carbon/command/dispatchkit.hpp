

#ifndef CARBON_DISPATCHKIT_HPP
#define CARBON_DISPATCHKIT_HPP

#include <algorithm>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>
#include <utility>
#include <vector>

#include "../defines.hpp"
#include "../threading.hpp"
#include "atom/experiment/flatmap.hpp"
#include "atom/experiment/short_alloc.hpp"
#include "atom/experiment/type_info.hpp"
#include "bad_boxed_cast.hpp"
#include "boxed_cast.hpp"
#include "boxed_cast_helper.hpp"
#include "boxed_value.hpp"
#include "dynamic_object.hpp"
#include "proxy_constructors.hpp"
#include "proxy_functions.hpp"
#include "type_conversions.hpp"

namespace Carbon {
class Boxed_Number;
}  // namespace Carbon

namespace Carbon {
namespace parser {
class Carbon_Parser_Base;
}
namespace dispatch {
class Dynamic_Proxy_Function;
class Proxy_Function_Base;
struct Placeholder_Object;
}  // namespace dispatch
}  // namespace Carbon

/// \namespace Carbon::dispatch
/// \brief Classes and functions specific to the runtime dispatch side of
/// ChaiScript. Some items may be of use to the end user.

namespace Carbon {
namespace exception {
/// Exception thrown in the case that an object name is invalid because it is a
/// reserved word
class reserved_word_error : public std::runtime_error {
public:
    explicit reserved_word_error(const std::string &t_word) noexcept
        : std::runtime_error("Reserved word not allowed in object name: " +
                             t_word),
          m_word(t_word) {}

    reserved_word_error(const reserved_word_error &) = default;

    ~reserved_word_error() noexcept override = default;

    std::string word() const { return m_word; }

private:
    std::string m_word;
};

/// Exception thrown in the case that an object name is invalid because it
/// contains illegal characters
class illegal_name_error : public std::runtime_error {
public:
    explicit illegal_name_error(const std::string &t_name) noexcept
        : std::runtime_error("Reserved name not allowed in object name: " +
                             t_name),
          m_name(t_name) {}

    illegal_name_error(const illegal_name_error &) = default;

    ~illegal_name_error() noexcept override = default;

    std::string name() const { return m_name; }

private:
    std::string m_name;
};

/// Exception thrown in the case that an object name is invalid because it
/// already exists in current context
class name_conflict_error : public std::runtime_error {
public:
    explicit name_conflict_error(const std::string &t_name) noexcept
        : std::runtime_error("Name already exists in current context " +
                             t_name),
          m_name(t_name) {}

    name_conflict_error(const name_conflict_error &) = default;

    ~name_conflict_error() noexcept override = default;

    std::string name() const { return m_name; }

private:
    std::string m_name;
};

/// Exception thrown in the case that a non-const object was added as a shared
/// object
class global_non_const : public std::runtime_error {
public:
    global_non_const() noexcept
        : std::runtime_error("a global object must be const") {}

    global_non_const(const global_non_const &) = default;
    ~global_non_const() noexcept override = default;
};
}  // namespace exception

/// \brief Holds a collection of ChaiScript settings which can be applied to the
/// ChaiScript runtime.
///        Used to implement loadable module support.
class Module {
public:
    Module &add(Type_Info ti, std::string name);

    Module &add(Type_Conversion d);

    Module &add(Proxy_Function f, std::string name);

    Module &add_global_const(Boxed_Value t_bv, std::string t_name);

    // Add a bit of ChaiScript to eval during module implementation
    Module &eval(std::string str);

    template <typename Eval, typename Engine>
    void apply(Eval &t_eval, Engine &t_engine) const {
        apply(m_typeinfos.begin(), m_typeinfos.end(), t_engine);
        apply(m_funcs.begin(), m_funcs.end(), t_engine);
        apply_eval(m_evals.begin(), m_evals.end(), t_eval);
        apply_single(m_conversions.begin(), m_conversions.end(), t_engine);
        apply_globals(m_globals.begin(), m_globals.end(), t_engine);
    }

    bool has_function(const Proxy_Function &new_f,
                      std::string_view name) noexcept;

private:
    std::vector<std::pair<Type_Info, std::string>> m_typeinfos;
    std::vector<std::pair<Proxy_Function, std::string>> m_funcs;
    std::vector<std::pair<Boxed_Value, std::string>> m_globals;
    std::vector<std::string> m_evals;
    std::vector<Type_Conversion> m_conversions;

    template <typename T, typename InItr>
    static void apply(InItr begin, const InItr end, T &t) {
        for_each(begin, end, [&t](const auto &obj) {
            try {
                t.add(obj.first, obj.second);
            } catch (const Carbon::exception::name_conflict_error &) {
                /// \todo Should we throw an error if there's a name conflict
                ///       while applying a module?
            }
        });
    }

    template <typename T, typename InItr>
    static void apply_globals(InItr begin, InItr end, T &t) {
        while (begin != end) {
            t.add_global_const(begin->first, begin->second);
            ++begin;
        }
    }

    template <typename T, typename InItr>
    static void apply_single(InItr begin, InItr end, T &t) {
        while (begin != end) {
            t.add(*begin);
            ++begin;
        }
    }

    template <typename T, typename InItr>
    static void apply_eval(InItr begin, InItr end, T &t) {
        while (begin != end) {
            t.eval(*begin);
            ++begin;
        }
    }
};

/// Convenience typedef for Module objects to be added to the ChaiScript runtime
using ModulePtr = std::shared_ptr<Module>;

namespace detail {
/// A Proxy_Function implementation that is able to take
/// a vector of Proxy_Functions and perform a dispatch on them. It is
/// used specifically in the case of dealing with Function object variables
class Dispatch_Function final : public dispatch::Proxy_Function_Base {
public:
    explicit Dispatch_Function(std::vector<Proxy_Function> t_funcs);

    bool operator==(
        const dispatch::Proxy_Function_Base &rhs) const noexcept override;

    std::vector<Const_Proxy_Function> get_contained_functions() const override;

    static int calculate_arity(
        const std::vector<Proxy_Function> &t_funcs) noexcept;

    bool call_match(
        const Function_Params &vals,
        const Type_Conversions_State &t_conversions) const noexcept override;

protected:
    Boxed_Value do_call(
        const Function_Params &params,
        const Type_Conversions_State &t_conversions) const override;

private:
    std::vector<Proxy_Function> m_funcs;

    static std::vector<Type_Info> build_type_infos(
        const std::vector<Proxy_Function> &t_funcs);
};
}  // namespace detail

namespace detail {
struct Stack_Holder {
    // template <class T, std::size_t BufSize = sizeof(T)*20000>
    //  using SmallVector = std::vector<T, short_alloc<T, BufSize>>;

    template <class T>
    using SmallVector = std::vector<T>;

    using Scope = QuickFlatMap<std::string, Boxed_Value, str_equal>;
    using StackData = SmallVector<Scope>;
    using Stacks = SmallVector<StackData>;
    using Call_Param_List = SmallVector<Boxed_Value>;
    using Call_Params = SmallVector<Call_Param_List>;

    Stack_Holder() {
        push_stack();
        push_call_params();
    }

    void push_stack_data() {
        stacks.back().emplace_back();
        //        stacks.back().emplace_back(Scope(scope_allocator));
    }

    void push_stack() { stacks.emplace_back(1); }

    void push_call_params() { call_params.emplace_back(); }

    Stacks stacks;
    Call_Params call_params;

    int call_depth = 0;
};

/// Main class for the dispatchkit. Handles management
/// of the object stack, functions and registered types.
class Dispatch_Engine {
public:
    using Type_Name_Map = std::map<std::string, Type_Info, str_less>;
    using Scope = QuickFlatMap<std::string, Boxed_Value, str_equal>;
    using StackData = Stack_Holder::StackData;

    struct State {
        QuickFlatMap<std::string, std::shared_ptr<std::vector<Proxy_Function>>,
                     str_equal>
            m_functions;
        QuickFlatMap<std::string, Proxy_Function, str_equal> m_function_objects;
        QuickFlatMap<std::string, Boxed_Value, str_equal> m_boxed_functions;
        std::map<std::string, Boxed_Value, str_less> m_global_objects;
        Type_Name_Map m_types;
    };

    explicit Dispatch_Engine(Carbon::parser::Carbon_Parser_Base &parser);

    /// \brief casts an object while applying any Dynamic_Conversion available
    template <typename Type>
    decltype(auto) boxed_cast(const Boxed_Value &bv) const {
        Type_Conversions_State state(m_conversions,
                                     m_conversions.conversion_saves());
        return (Carbon::boxed_cast<Type>(bv, &state));
    }

    /// Add a new conversion for upcasting to a base class
    void add(const Type_Conversion &d);

    /// Add a new named Proxy_Function to the system
    void add(const Proxy_Function &f, const std::string &name);

    /// Set the value of an object, by name. If the object
    /// is not available in the current scope it is created
    void add(Boxed_Value obj, const std::string &name);

    /// Adds a named object to the current scope
    /// \warning This version does not check the validity of the name
    /// it is meant for internal use only
    Boxed_Value &add_get_object(std::string t_name, Boxed_Value obj,
                                Stack_Holder &t_holder);

    /// Adds a named object to the current scope
    /// \warning This version does not check the validity of the name
    /// it is meant for internal use only
    void add_object(std::string t_name, Boxed_Value obj,
                    Stack_Holder &t_holder);

    /// Adds a named object to the current scope
    /// \warning This version does not check the validity of the name
    /// it is meant for internal use only
    void add_object(const std::string &name, Boxed_Value obj);

    /// Adds a new global shared object, between all the threads
    void add_global_const(const Boxed_Value &obj, const std::string &name);

    /// Adds a new global (non-const) shared object, between all the threads
    Boxed_Value add_global_no_throw(Boxed_Value obj, std::string name);

    /// Adds a new global (non-const) shared object, between all the threads
    void add_global(Boxed_Value obj, std::string name);

    /// Updates an existing global shared object or adds a new global shared
    /// object if not found
    void set_global(Boxed_Value obj, std::string name);

    /// Adds a new scope to the stack
    void new_scope();

    /// Pops the current scope from the stack
    void pop_scope();

    /// Adds a new scope to the stack
    static void new_scope(Stack_Holder &t_holder);

    /// Pops the current scope from the stack
    static void pop_scope(Stack_Holder &t_holder);

    /// Pushes a new stack on to the list of stacks
    static void new_stack(Stack_Holder &t_holder);

    static void pop_stack(Stack_Holder &t_holder);

    /// Searches the current stack for an object of the given name
    /// includes a special overload for the _ place holder object to
    /// ensure that it is always in scope.
    Boxed_Value get_object(std::string_view name,
                           std::atomic_uint_fast32_t &t_loc,
                           Stack_Holder &t_holder) const;

    /// Registers a new named type
    void add(const Type_Info &ti, const std::string &name);

    /// Returns the type info for a named type
    Type_Info get_type(std::string_view name, bool t_throw = true) const;

    /// Returns the registered name of a known type_info object
    /// compares the "bare_type_info" for the broadest possible
    /// match
    std::string get_type_name(const Type_Info &ti) const;

    /// Return all registered types
    std::vector<std::pair<std::string, Type_Info>> get_types() const;

    std::shared_ptr<std::vector<Proxy_Function>> get_method_missing_functions()
        const;

    /// Return a function by name
    std::pair<size_t, std::shared_ptr<std::vector<Proxy_Function>>>
    get_function(std::string_view t_name, const size_t t_hint) const;

    /// \returns a function object (Boxed_Value wrapper) if it exists
    /// \throws std::range_error if it does not
    Boxed_Value get_function_object(const std::string &t_name) const;

    /// \returns a function object (Boxed_Value wrapper) if it exists
    /// \throws std::range_error if it does not
    /// \warn does not obtain a mutex lock. \sa get_function_object for public
    /// version
    std::pair<size_t, Boxed_Value> get_function_object_int(
        std::string_view t_name, const size_t t_hint) const;

    /// Return true if a function exists
    bool function_exists(std::string_view name) const;

    /// \returns All values in the local thread state in the parent scope, or if
    /// it doesn't exist,
    ///          the current scope.
    std::map<std::string, Boxed_Value> get_parent_locals() const;

    /// \returns All values in the local thread state, added through the add()
    /// function
    std::map<std::string, Boxed_Value> get_locals() const;

    /// \brief Sets all of the locals for the current thread state.
    ///
    /// \param[in] t_locals The map<name, value> set of variables to replace the
    /// current state with
    ///
    /// Any existing locals are removed and the given set of variables is added
    void set_locals(const std::map<std::string, Boxed_Value> &t_locals);

    ///
    /// Get a map of all objects that can be seen from the current scope in a
    /// scripting context
    ///
    std::map<std::string, Boxed_Value> get_scripting_objects() const;

    ///
    /// Get a map of all functions that can be seen from a scripting context
    ///
    std::map<std::string, Boxed_Value> get_function_objects() const;

    /// Get a vector of all registered functions
    std::vector<std::pair<std::string, Proxy_Function>> get_functions() const;

    const Type_Conversions &conversions() const noexcept;

    static bool is_attribute_call(
        const std::vector<Proxy_Function> &t_funs,
        const Function_Params &t_params, bool t_has_params,
        const Type_Conversions_State &t_conversions) noexcept;

#ifdef CARBON_MSVC
// MSVC is unable to recognize that "rethrow_exception" causes the function to
// return so we must disable it here.
#pragma warning(push)
#pragma warning(disable : 4715)
#endif
    Boxed_Value call_member(const std::string &t_name,
                            std::atomic_uint_fast32_t &t_loc,
                            const Function_Params &params, bool t_has_params,
                            const Type_Conversions_State &t_conversions);
#ifdef CARBON_MSVC
#pragma warning(pop)
#endif

    Boxed_Value call_function(
        std::string_view t_name, std::atomic_uint_fast32_t &t_loc,
        const Function_Params &params,
        const Type_Conversions_State &t_conversions) const;

    /// Dump object info to stdout
    void dump_object(const Boxed_Value &o) const;

    /// Dump type info to stdout
    void dump_type(const Type_Info &type) const;

    /// Dump function to stdout
    void dump_function(
        const std::pair<const std::string, Proxy_Function> &f) const;

    /// Returns true if a call can be made that consists of the first parameter
    /// (the function) with the remaining parameters as its arguments.
    Boxed_Value call_exists(const Function_Params &params) const;

    /// Dump all system info to stdout
    void dump_system() const;

    /// return true if the Boxed_Value matches the registered type by name
    bool is_type(const Boxed_Value &r,
                 std::string_view user_typename) const noexcept;

    std::string type_name(const Boxed_Value &obj) const;

    State get_state() const;

    void set_state(const State &t_state);

    static void save_function_params(Stack_Holder &t_s,
                                     std::vector<Boxed_Value> &&t_params);

    static void save_function_params(Stack_Holder &t_s,
                                     const Function_Params &t_params);

    void save_function_params(std::vector<Boxed_Value> &&t_params);

    void save_function_params(const Function_Params &t_params);

    void new_function_call(Stack_Holder &t_s,
                           Type_Conversions::Conversion_Saves &t_saves);

    void pop_function_call(Stack_Holder &t_s,
                           Type_Conversions::Conversion_Saves &t_saves);

    void new_function_call();

    void pop_function_call();

    Stack_Holder &get_stack_holder() noexcept;

    /// Returns the current stack
    /// make const/non const versions
    const StackData &get_stack_data() const noexcept;

    static StackData &get_stack_data(Stack_Holder &t_holder) noexcept;

    StackData &get_stack_data() noexcept;

    parser::Carbon_Parser_Base &get_parser() noexcept;

private:
    const decltype(State::m_boxed_functions) &get_boxed_functions_int()
        const noexcept {
        return m_state.m_boxed_functions;
    }

    decltype(State::m_boxed_functions) &get_boxed_functions_int() noexcept {
        return m_state.m_boxed_functions;
    }

    const decltype(State::m_function_objects) &get_function_objects_int()
        const noexcept {
        return m_state.m_function_objects;
    }

    decltype(State::m_function_objects) &get_function_objects_int() noexcept {
        return m_state.m_function_objects;
    }

    const decltype(State::m_functions) &get_functions_int() const noexcept {
        return m_state.m_functions;
    }

    decltype(State::m_functions) &get_functions_int() noexcept {
        return m_state.m_functions;
    }

    static bool function_less_than(const Proxy_Function &lhs,
                                   const Proxy_Function &rhs) noexcept;

    /// Implementation detail for adding a function.
    /// \throws exception::name_conflict_error if there's a function matching
    /// the given one being added
    void add_function(const Proxy_Function &t_f, const std::string &t_name);

    mutable Carbon::detail::threading::shared_mutex m_mutex;

    Type_Conversions m_conversions;
    Carbon::detail::threading::Thread_Storage<Stack_Holder> m_stack_holder;
    std::reference_wrapper<parser::Carbon_Parser_Base> m_parser;

    mutable std::atomic_uint_fast32_t m_method_missing_loc = {0};

    State m_state;
};

class Dispatch_State {
public:
    explicit Dispatch_State(Dispatch_Engine &t_engine);

    Dispatch_Engine *operator->() const noexcept;

    Dispatch_Engine &operator*() const noexcept;

    Stack_Holder &stack_holder() const noexcept;

    const Type_Conversions_State &conversions() const noexcept;

    Type_Conversions::Conversion_Saves &conversion_saves() const noexcept;

    Boxed_Value &add_get_object(const std::string &t_name,
                                Boxed_Value obj) const;

    void add_object(const std::string &t_name, Boxed_Value obj) const;

    Boxed_Value get_object(std::string_view t_name,
                           std::atomic_uint_fast32_t &t_loc) const;

private:
    std::reference_wrapper<Dispatch_Engine> m_engine;
    std::reference_wrapper<Stack_Holder> m_stack_holder;
    Type_Conversions_State m_conversions;
};
}  // namespace detail
}  // namespace Carbon

#endif
