#include "dispatchkit.hpp"
#include "boxed_number.hpp"

namespace Carbon {

Module &Module::add(Type_Info ti, std::string name) {
    m_typeinfos.emplace_back(ti, std::move(name));
    return *this;
}

Module &Module::add(Type_Conversion d) {
    m_conversions.push_back(std::move(d));
    return *this;
}

Module &Module::add(Proxy_Function f, std::string name) {
    m_funcs.emplace_back(std::move(f), std::move(name));
    return *this;
}

Module &Module::add_global_const(Boxed_Value t_bv, std::string t_name) {
    if (!t_bv.is_const()) {
        throw Carbon::exception::global_non_const();
    }

    m_globals.emplace_back(std::move(t_bv), std::move(t_name));
    return *this;
}

// Add a bit of ChaiScript to eval during module implementation
Module &Module::eval(std::string str) {
    m_evals.push_back(std::move(str));
    return *this;
}

bool Module::has_function(const Proxy_Function &new_f, std::string_view name) {
    return std::any_of(
        m_funcs.begin(), m_funcs.end(),
        [&](const std::pair<Proxy_Function, std::string> &existing_f) {
            return existing_f.second == name && *(existing_f.first) == *(new_f);
        });
}

namespace detail {
Dispatch_Function::Dispatch_Function(std::vector<Proxy_Function> t_funcs)
    : Proxy_Function_Base(build_type_infos(t_funcs), calculate_arity(t_funcs)),
      m_funcs(std::move(t_funcs)) {}

bool Dispatch_Function::operator==(
    const dispatch::Proxy_Function_Base &rhs) const {
    try {
        const auto &dispatch_fun = dynamic_cast<const Dispatch_Function &>(rhs);
        return m_funcs == dispatch_fun.m_funcs;
    } catch (const std::bad_cast &) {
        return false;
    }
}

std::vector<Const_Proxy_Function> Dispatch_Function::get_contained_functions()
    const {
    return std::vector<Const_Proxy_Function>(m_funcs.begin(), m_funcs.end());
}

int Dispatch_Function::calculate_arity(
    const std::vector<Proxy_Function> &t_funcs) {
    if (t_funcs.empty()) {
        return -1;
    }

    const auto arity = t_funcs.front()->get_arity();

    for (const auto &func : t_funcs) {
        if (arity != func->get_arity()) {
            // The arities in the list do not match, so it's unspecified
            return -1;
        }
    }

    return arity;
}

bool Dispatch_Function::call_match(
    const Function_Params &vals,
    const Type_Conversions_State &t_conversions) const {
    return std::any_of(std::begin(m_funcs), std::end(m_funcs),
                       [&vals, &t_conversions](const Proxy_Function &f) {
                           return f->call_match(vals, t_conversions);
                       });
}

Boxed_Value Dispatch_Function::do_call(
    const Function_Params &params,
    const Type_Conversions_State &t_conversions) const {
    return dispatch::dispatch(m_funcs, params, t_conversions);
}

std::vector<Type_Info> Dispatch_Function::build_type_infos(
    const std::vector<Proxy_Function> &t_funcs) {
    auto begin = t_funcs.cbegin();
    const auto &end = t_funcs.cend();

    if (begin != end) {
        std::vector<Type_Info> type_infos = (*begin)->get_param_types();

        ++begin;

        bool size_mismatch = false;

        while (begin != end) {
            std::vector<Type_Info> param_types = (*begin)->get_param_types();

            if (param_types.size() != type_infos.size()) {
                size_mismatch = true;
            }

            for (size_t i = 0; i < type_infos.size() && i < param_types.size();
                 ++i) {
                if (!(type_infos[i] == param_types[i])) {
                    type_infos[i] = Get_Type_Info<Boxed_Value>::get();
                }
            }

            ++begin;
        }

        assert(!type_infos.empty() &&
               " type_info vector size is < 0, this is only possible if "
               "something else is broken");

        if (size_mismatch) {
            type_infos.resize(1);
        }

        return type_infos;
    }

    return std::vector<Type_Info>();
}
}  // namespace detail

namespace detail {

Dispatch_Engine::Dispatch_Engine(Carbon::parser::Carbon_Parser_Base &parser)
    : m_stack_holder(), m_parser(parser) {}

void Dispatch_Engine::add(const Type_Conversion &d) {
    m_conversions.add_conversion(d);
}

void Dispatch_Engine::add(const Proxy_Function &f, const std::string &name) {
    add_function(f, name);
}

void Dispatch_Engine::add(Boxed_Value obj, const std::string &name) {
    auto &stack = get_stack_data();

    for (auto stack_elem = stack.rbegin(); stack_elem != stack.rend();
         ++stack_elem) {
        if (auto itr = stack_elem->find(name); itr != stack_elem->end()) {
            itr->second = std::move(obj);
            return;
        }
    }

    add_object(name, std::move(obj));
}

Boxed_Value &Dispatch_Engine::add_get_object(std::string t_name,
                                             Boxed_Value obj,
                                             Stack_Holder &t_holder) {
    auto &stack_elem = get_stack_data(t_holder).back();

    if (auto result =
            stack_elem.insert(std::pair{std::move(t_name), std::move(obj)});
        result.second) {
        return result.first->second;
    } else {
        // insert failed
        throw Carbon::exception::name_conflict_error(result.first->first);
    }
}

void Dispatch_Engine::add_object(std::string t_name, Boxed_Value obj,
                                 Stack_Holder &t_holder) {
    auto &stack_elem = get_stack_data(t_holder).back();

    if (auto result =
            stack_elem.insert(std::pair{std::move(t_name), std::move(obj)});
        !result.second) {
        // insert failed
        throw Carbon::exception::name_conflict_error(result.first->first);
    }
}

void Dispatch_Engine::add_object(const std::string &name, Boxed_Value obj) {
    add_object(name, std::move(obj), get_stack_holder());
}

/// Adds a new global shared object, between all the threads
void Dispatch_Engine::add_global_const(const Boxed_Value &obj,
                                       const std::string &name) {
    if (!obj.is_const()) {
        throw Carbon::exception::global_non_const();
    }

    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    if (m_state.m_global_objects.find(name) != m_state.m_global_objects.end()) {
        throw Carbon::exception::name_conflict_error(name);
    } else {
        m_state.m_global_objects.insert(std::make_pair(name, obj));
    }
}

/// Adds a new global (non-const) shared object, between all the threads
Boxed_Value Dispatch_Engine::add_global_no_throw(Boxed_Value obj,
                                                 std::string name) {
    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    return m_state.m_global_objects
        .insert(std::pair{std::move(name), std::move(obj)})
        .first->second;
}

/// Adds a new global (non-const) shared object, between all the threads
void Dispatch_Engine::add_global(Boxed_Value obj, std::string name) {
    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    if (auto result = m_state.m_global_objects.insert(
            std::pair{std::move(name), std::move(obj)});
        !result.second) {
        // insert failed
        throw Carbon::exception::name_conflict_error(result.first->first);
    }
}

void Dispatch_Engine::set_global(Boxed_Value obj, std::string name) {
    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);
    m_state.m_global_objects.insert_or_assign(std::move(name), std::move(obj));
}

void Dispatch_Engine::new_scope() { new_scope(*m_stack_holder); }

void Dispatch_Engine::pop_scope() { pop_scope(*m_stack_holder); }

void Dispatch_Engine::new_scope(Stack_Holder &t_holder) {
    t_holder.push_stack_data();
    t_holder.push_call_params();
}

void Dispatch_Engine::pop_scope(Stack_Holder &t_holder) {
    t_holder.call_params.pop_back();
    StackData &stack = get_stack_data(t_holder);

    assert(!stack.empty());

    stack.pop_back();
}

void Dispatch_Engine::new_stack(Stack_Holder &t_holder) {
    // add a new Stack with 1 element
    t_holder.push_stack();
}

void Dispatch_Engine::pop_stack(Stack_Holder &t_holder) {
    t_holder.stacks.pop_back();
}

Boxed_Value Dispatch_Engine::get_object(std::string_view name,
                                        std::atomic_uint_fast32_t &t_loc,
                                        Stack_Holder &t_holder) const {
    enum class Loc : uint_fast32_t {
        located = 0x80000000,
        is_local = 0x40000000,
        stack_mask = 0x0FFF0000,
        loc_mask = 0x0000FFFF
    };

    uint_fast32_t loc = t_loc;

    if (loc == 0) {
        auto &stack = get_stack_data(t_holder);

        // Is it in the stack?
        for (auto stack_elem = stack.rbegin(); stack_elem != stack.rend();
             ++stack_elem) {
            for (auto s = stack_elem->begin(); s != stack_elem->end(); ++s) {
                if (s->first == name) {
                    t_loc =
                        static_cast<uint_fast32_t>(
                            std::distance(stack.rbegin(), stack_elem) << 16) |
                        static_cast<uint_fast32_t>(
                            std::distance(stack_elem->begin(), s)) |
                        static_cast<uint_fast32_t>(Loc::located) |
                        static_cast<uint_fast32_t>(Loc::is_local);
                    return s->second;
                }
            }
        }

        t_loc = static_cast<uint_fast32_t>(Loc::located);
    } else if ((loc & static_cast<uint_fast32_t>(Loc::is_local)) != 0u) {
        auto &stack = get_stack_data(t_holder);

        return stack[stack.size() - 1 -
                     ((loc & static_cast<uint_fast32_t>(Loc::stack_mask)) >>
                      16)]
            .at_index(loc & static_cast<uint_fast32_t>(Loc::loc_mask));
    }

    // Is the value we are looking for a global or function?
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    const auto itr = m_state.m_global_objects.find(name);
    if (itr != m_state.m_global_objects.end()) {
        return itr->second;
    }

    // no? is it a function object?
    auto obj = get_function_object_int(name, loc);
    if (obj.first != loc) {
        t_loc = uint_fast32_t(obj.first);
    }

    return obj.second;
}

void Dispatch_Engine::add(const Type_Info &ti, const std::string &name) {
    add_global_const(const_var(ti), name + "_type");

    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    m_state.m_types.insert(std::make_pair(name, ti));
}

Type_Info Dispatch_Engine::get_type(std::string_view name,
                                    bool t_throw = true) const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    const auto itr = m_state.m_types.find(name);

    if (itr != m_state.m_types.end()) {
        return itr->second;
    }

    if (t_throw) {
        throw std::range_error("Type Not Known: " + std::string(name));
    } else {
        return Type_Info();
    }
}

std::string Dispatch_Engine::get_type_name(const Type_Info &ti) const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    for (const auto &elem : m_state.m_types) {
        if (elem.second.bare_equal(ti)) {
            return elem.first;
        }
    }

    return ti.bare_name();
}

std::vector<std::pair<std::string, Type_Info>> Dispatch_Engine::get_types()
    const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    return std::vector<std::pair<std::string, Type_Info>>(
        m_state.m_types.begin(), m_state.m_types.end());
}

std::shared_ptr<std::vector<Proxy_Function>>
Dispatch_Engine::get_method_missing_functions() const {
    uint_fast32_t method_missing_loc = m_method_missing_loc;
    auto method_missing_funs =
        get_function("method_missing", method_missing_loc);
    if (method_missing_funs.first != method_missing_loc) {
        m_method_missing_loc = uint_fast32_t(method_missing_funs.first);
    }

    return std::move(method_missing_funs.second);
}

std::pair<size_t, std::shared_ptr<std::vector<Proxy_Function>>>
Dispatch_Engine::get_function(std::string_view t_name,
                              const size_t t_hint) const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    const auto &funs = get_functions_int();

    if (const auto itr = funs.find(t_name, t_hint); itr != funs.end()) {
        return std::make_pair(std::distance(funs.begin(), itr), itr->second);
    } else {
        return std::make_pair(size_t(0),
                              std::make_shared<std::vector<Proxy_Function>>());
    }
}

Boxed_Value Dispatch_Engine::get_function_object(
    const std::string &t_name) const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    return get_function_object_int(t_name, 0).second;
}

std::pair<size_t, Boxed_Value> Dispatch_Engine::get_function_object_int(
    std::string_view t_name, const size_t t_hint) const {
    const auto &funs = get_boxed_functions_int();

    if (const auto itr = funs.find(t_name, t_hint); itr != funs.end()) {
        return std::make_pair(std::distance(funs.begin(), itr), itr->second);
    } else {
        throw std::range_error("Object not found: " + std::string(t_name));
    }
}

bool Dispatch_Engine::function_exists(std::string_view name) const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    return get_functions_int().count(name) > 0;
}

std::map<std::string, Boxed_Value> Dispatch_Engine::get_parent_locals() const {
    auto &stack = get_stack_data();
    if (stack.size() > 1) {
        return std::map<std::string, Boxed_Value>(stack[1].begin(),
                                                  stack[1].end());
    } else {
        return std::map<std::string, Boxed_Value>(stack[0].begin(),
                                                  stack[0].end());
    }
}

std::map<std::string, Boxed_Value> Dispatch_Engine::get_locals() const {
    auto &stack = get_stack_data();
    auto &scope = stack.front();
    return std::map<std::string, Boxed_Value>(scope.begin(), scope.end());
}

void Dispatch_Engine::set_locals(
    const std::map<std::string, Boxed_Value> &t_locals) {
    auto &stack = get_stack_data();
    auto &scope = stack.front();
    scope.assign(t_locals.begin(), t_locals.end());
}

std::map<std::string, Boxed_Value> Dispatch_Engine::get_scripting_objects()
    const {
    const Stack_Holder &s = *m_stack_holder;

    // We don't want the current context, but one up if it exists
    const StackData &stack = (s.stacks.size() == 1)
                                 ? (s.stacks.back())
                                 : (s.stacks[s.stacks.size() - 2]);

    std::map<std::string, Boxed_Value> retval;

    // note: map insert doesn't overwrite existing values, which is why this
    // works
    for (auto itr = stack.rbegin(); itr != stack.rend(); ++itr) {
        retval.insert(itr->begin(), itr->end());
    }

    // add the global values
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);
    retval.insert(m_state.m_global_objects.begin(),
                  m_state.m_global_objects.end());

    return retval;
}

std::map<std::string, Boxed_Value> Dispatch_Engine::get_function_objects()
    const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    const auto &funs = get_function_objects_int();

    std::map<std::string, Boxed_Value> objs;

    for (const auto &fun : funs) {
        objs.insert(std::make_pair(fun.first, const_var(fun.second)));
    }

    return objs;
}

/// Get a vector of all registered functions
std::vector<std::pair<std::string, Proxy_Function>>
Dispatch_Engine::get_functions() const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    std::vector<std::pair<std::string, Proxy_Function>> rets;

    const auto &functions = get_functions_int();

    for (const auto &function : functions) {
        for (const auto &internal_func : *function.second) {
            rets.emplace_back(function.first, internal_func);
        }
    }

    return rets;
}

const Type_Conversions &Dispatch_Engine::conversions() const {
    return m_conversions;
}

bool Dispatch_Engine::is_attribute_call(
    const std::vector<Proxy_Function> &t_funs, const Function_Params &t_params,
    bool t_has_params, const Type_Conversions_State &t_conversions) {
    if (!t_has_params || t_params.empty()) {
        return false;
    }

    return std::any_of(
        std::begin(t_funs), std::end(t_funs), [&](const auto &fun) {
            return fun->is_attribute_function() &&
                   fun->compare_first_type(t_params[0], t_conversions);
        });
}

#ifdef CARBON_MSVC
// MSVC is unable to recognize that "rethrow_exception" causes the function to
// return so we must disable it here.
#pragma warning(push)
#pragma warning(disable : 4715)
#endif
Boxed_Value Dispatch_Engine::call_member(
    const std::string &t_name, std::atomic_uint_fast32_t &t_loc,
    const Function_Params &params, bool t_has_params,
    const Type_Conversions_State &t_conversions) {
    uint_fast32_t loc = t_loc;
    const auto funs = get_function(t_name, loc);
    if (funs.first != loc) {
        t_loc = uint_fast32_t(funs.first);
    }

    const auto do_attribute_call =
        [this](int l_num_params, Function_Params l_params,
               const std::vector<Proxy_Function> &l_funs,
               const Type_Conversions_State &l_conversions) -> Boxed_Value {
        Function_Params attr_params(l_params.begin(),
                                    l_params.begin() + l_num_params);
        Boxed_Value bv = dispatch::dispatch(l_funs, attr_params, l_conversions);
        if (l_num_params < int(l_params.size()) ||
            bv.get_type_info().bare_equal(
                user_type<dispatch::Proxy_Function_Base>())) {
            struct This_Foist {
                This_Foist(Dispatch_Engine &e, const Boxed_Value &t_bv)
                    : m_e(e) {
                    m_e.get().new_scope();
                    m_e.get().add_object("__this", t_bv);
                }

                ~This_Foist() { m_e.get().pop_scope(); }

                std::reference_wrapper<Dispatch_Engine> m_e;
            };

            This_Foist fi(*this, l_params.front());

            try {
                auto func =
                    boxed_cast<const dispatch::Proxy_Function_Base *>(bv);
                try {
                    return (*func)(
                        {l_params.begin() + l_num_params, l_params.end()},
                        l_conversions);
                } catch (const Carbon::exception::bad_boxed_cast &) {
                } catch (const Carbon::exception::arity_error &) {
                } catch (const Carbon::exception::guard_error &) {
                }
                throw Carbon::exception::dispatch_error(
                    {l_params.begin() + l_num_params, l_params.end()},
                    std::vector<Const_Proxy_Function>{
                        boxed_cast<Const_Proxy_Function>(bv)});
            } catch (const Carbon::exception::bad_boxed_cast &) {
                // unable to convert bv into a Proxy_Function_Base
                throw Carbon::exception::dispatch_error(
                    {l_params.begin() + l_num_params, l_params.end()},
                    std::vector<Const_Proxy_Function>(l_funs.begin(),
                                                      l_funs.end()));
            }
        } else {
            return bv;
        }
    };

    if (is_attribute_call(*funs.second, params, t_has_params, t_conversions)) {
        return do_attribute_call(1, params, *funs.second, t_conversions);
    } else {
        std::exception_ptr except;

        if (!funs.second->empty()) {
            try {
                return dispatch::dispatch(*funs.second, params, t_conversions);
            } catch (Carbon::exception::dispatch_error &) {
                except = std::current_exception();
            }
        }

        // If we get here we know that either there was no method with that
        // name, or there was no matching method

        const auto functions = [&]() -> std::vector<Proxy_Function> {
            std::vector<Proxy_Function> fs;

            const auto method_missing_funs = get_method_missing_functions();

            for (const auto &f : *method_missing_funs) {
                if (f->compare_first_type(params[0], t_conversions)) {
                    fs.push_back(f);
                }
            }

            return fs;
        }();

        const bool is_no_param = [&]() -> bool {
            for (const auto &f : functions) {
                if (f->get_arity() != 2) {
                    return false;
                }
            }
            return true;
        }();

        if (!functions.empty()) {
            try {
                if (is_no_param) {
                    auto tmp_params = params.to_vector();
                    tmp_params.insert(tmp_params.begin() + 1, var(t_name));
                    return do_attribute_call(2, Function_Params(tmp_params),
                                             functions, t_conversions);
                } else {
                    std::array<Boxed_Value, 3> p{
                        params[0], var(t_name),
                        var(std::vector<Boxed_Value>(params.begin() + 1,
                                                     params.end()))};
                    return dispatch::dispatch(functions, Function_Params{p},
                                              t_conversions);
                }
            } catch (const dispatch::option_explicit_set &e) {
                throw Carbon::exception::dispatch_error(
                    params,
                    std::vector<Const_Proxy_Function>(funs.second->begin(),
                                                      funs.second->end()),
                    e.what());
            }
        }

        // If we get all the way down here we know there was no
        // "method_missing" method at all.
        if (except) {
            std::rethrow_exception(except);
        } else {
            throw Carbon::exception::dispatch_error(
                params, std::vector<Const_Proxy_Function>(funs.second->begin(),
                                                          funs.second->end()));
        }
    }
}
#ifdef CARBON_MSVC
#pragma warning(pop)
#endif

Boxed_Value Dispatch_Engine::call_function(
    std::string_view t_name, std::atomic_uint_fast32_t &t_loc,
    const Function_Params &params,
    const Type_Conversions_State &t_conversions) const {
    uint_fast32_t loc = t_loc;
    const auto [func_loc, func] = get_function(t_name, loc);
    if (func_loc != loc) {
        t_loc = uint_fast32_t(func_loc);
    }
    return dispatch::dispatch(*func, params, t_conversions);
}

/// Dump object info to stdout
void Dispatch_Engine::dump_object(const Boxed_Value &o) const {
    std::cout << (o.is_const() ? "const " : "") << type_name(o) << '\n';
}

/// Dump type info to stdout
void Dispatch_Engine::dump_type(const Type_Info &type) const {
    std::cout << (type.is_const() ? "const " : "") << get_type_name(type);
}

/// Dump function to stdout
void Dispatch_Engine::dump_function(
    const std::pair<const std::string, Proxy_Function> &f) const {
    const auto params = f.second->get_param_types();

    dump_type(params.front());
    std::cout << " " << f.first << "(";

    for (auto itr = params.begin() + 1; itr != params.end();) {
        dump_type(*itr);
        ++itr;

        if (itr != params.end()) {
            std::cout << ", ";
        }
    }

    std::cout << ") \n";
}

/// Returns true if a call can be made that consists of the first parameter
/// (the function) with the remaining parameters as its arguments.
Boxed_Value Dispatch_Engine::call_exists(const Function_Params &params) const {
    if (params.empty()) {
        throw Carbon::exception::arity_error(static_cast<int>(params.size()),
                                             1);
    }

    const auto &f = this->boxed_cast<Const_Proxy_Function>(params[0]);
    const Type_Conversions_State convs(m_conversions,
                                       m_conversions.conversion_saves());

    return const_var(f->call_match(
        Function_Params(params.begin() + 1, params.end()), convs));
}

/// Dump all system info to stdout
void Dispatch_Engine::dump_system() const {
    std::cout << "Registered Types: \n";
    for (const auto &[type_name, type] : get_types()) {
        std::cout << type_name << ": " << type.bare_name() << '\n';
    }

    std::cout << '\n';

    std::cout << "Functions: \n";
    for (const auto &func : get_functions()) {
        dump_function(func);
    }
    std::cout << '\n';
}

/// return true if the Boxed_Value matches the registered type by name
bool Dispatch_Engine::is_type(const Boxed_Value &r,
                              std::string_view user_typename) const {
    try {
        if (get_type(user_typename).bare_equal(r.get_type_info())) {
            return true;
        }
    } catch (const std::range_error &) {
    }

    try {
        const dispatch::Dynamic_Object &d =
            boxed_cast<const dispatch::Dynamic_Object &>(r);
        return d.get_type_name() == user_typename;
    } catch (const std::bad_cast &) {
    }

    return false;
}

std::string Dispatch_Engine::type_name(const Boxed_Value &obj) const {
    return get_type_name(obj.get_type_info());
}

Dispatch_Engine::State Dispatch_Engine::get_state() const {
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    return m_state;
}

void Dispatch_Engine::set_state(const State &t_state) {
    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    m_state = t_state;
}

void Dispatch_Engine::save_function_params(
    Stack_Holder &t_s, std::vector<Boxed_Value> &&t_params) {
    for (auto &&param : t_params) {
        t_s.call_params.back().insert(t_s.call_params.back().begin(),
                                      std::move(param));
    }
}

void Dispatch_Engine::save_function_params(Stack_Holder &t_s,
                                           const Function_Params &t_params) {
    t_s.call_params.back().insert(t_s.call_params.back().begin(),
                                  t_params.begin(), t_params.end());
}

void Dispatch_Engine::save_function_params(
    std::vector<Boxed_Value> &&t_params) {
    save_function_params(*m_stack_holder, std::move(t_params));
}

void Dispatch_Engine::save_function_params(const Function_Params &t_params) {
    save_function_params(*m_stack_holder, t_params);
}

void Dispatch_Engine::new_function_call(
    Stack_Holder &t_s, Type_Conversions::Conversion_Saves &t_saves) {
    if (t_s.call_depth == 0) {
        m_conversions.enable_conversion_saves(t_saves, true);
    }

    ++t_s.call_depth;

    save_function_params(m_conversions.take_saves(t_saves));
}

void Dispatch_Engine::pop_function_call(
    Stack_Holder &t_s, Type_Conversions::Conversion_Saves &t_saves) {
    --t_s.call_depth;

    assert(t_s.call_depth >= 0);

    if (t_s.call_depth == 0) {
        t_s.call_params.back().clear();
        m_conversions.enable_conversion_saves(t_saves, false);
    }
}

void Dispatch_Engine::new_function_call() {
    new_function_call(*m_stack_holder, m_conversions.conversion_saves());
}

void Dispatch_Engine::pop_function_call() {
    pop_function_call(*m_stack_holder, m_conversions.conversion_saves());
}

Stack_Holder &Dispatch_Engine::get_stack_holder() { return *m_stack_holder; }

const Dispatch_Engine::StackData &Dispatch_Engine::get_stack_data() const {
    return m_stack_holder->stacks.back();
}

Dispatch_Engine::StackData &Dispatch_Engine::get_stack_data(
    Stack_Holder &t_holder) {
    return t_holder.stacks.back();
}

Dispatch_Engine::StackData &Dispatch_Engine::get_stack_data() {
    return m_stack_holder->stacks.back();
}

parser::Carbon_Parser_Base &Dispatch_Engine::get_parser() {
    return m_parser.get();
}

bool Dispatch_Engine::function_less_than(const Proxy_Function &lhs,
                                         const Proxy_Function &rhs) {
    auto dynamic_lhs(
        std::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(lhs));
    auto dynamic_rhs(
        std::dynamic_pointer_cast<const dispatch::Dynamic_Proxy_Function>(rhs));

    if (dynamic_lhs && dynamic_rhs) {
        if (dynamic_lhs->get_guard()) {
            return dynamic_rhs->get_guard() ? false : true;
        } else {
            return false;
        }
    }

    if (dynamic_lhs && !dynamic_rhs) {
        return false;
    }

    if (!dynamic_lhs && dynamic_rhs) {
        return true;
    }

    const auto &lhsparamtypes = lhs->get_param_types();
    const auto &rhsparamtypes = rhs->get_param_types();

    const auto lhssize = lhsparamtypes.size();
    const auto rhssize = rhsparamtypes.size();

    const auto boxed_type = user_type<Boxed_Value>();
    const auto boxed_pod_type = user_type<Boxed_Number>();

    for (size_t i = 1; i < lhssize && i < rhssize; ++i) {
        const Type_Info &lt = lhsparamtypes[i];
        const Type_Info &rt = rhsparamtypes[i];

        if (lt.bare_equal(rt) && lt.is_const() == rt.is_const()) {
            continue;  // The first two types are essentially the same, next
                       // iteration
        }

        // const is after non-const for the same type
        if (lt.bare_equal(rt) && lt.is_const() && !rt.is_const()) {
            return false;
        }

        if (lt.bare_equal(rt) && !lt.is_const()) {
            return true;
        }

        // boxed_values are sorted last
        if (lt.bare_equal(boxed_type)) {
            return false;
        }

        if (rt.bare_equal(boxed_type)) {
            return true;
        }

        if (lt.bare_equal(boxed_pod_type)) {
            return false;
        }

        if (rt.bare_equal(boxed_pod_type)) {
            return true;
        }

        // otherwise, we want to sort by typeid
        return lt < rt;
    }

    return false;
}

/// Implementation detail for adding a function.
/// \throws exception::name_conflict_error if there's a function matching
/// the given one being added
void Dispatch_Engine::add_function(const Proxy_Function &t_f,
                                   const std::string &t_name) {
    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::shared_mutex>
        l(m_mutex);

    Proxy_Function new_func = [&]() -> Proxy_Function {
        auto &funcs = get_functions_int();
        auto itr = funcs.find(t_name);

        if (itr != funcs.end()) {
            auto vec = *itr->second;
            for (const auto &func : vec) {
                if ((*t_f) == *(func)) {
                    throw Carbon::exception::name_conflict_error(t_name);
                }
            }

            vec.reserve(vec.size() + 1);  // tightly control vec growth
            vec.push_back(t_f);
            std::stable_sort(vec.begin(), vec.end(), &function_less_than);
            itr->second = std::make_shared<std::vector<Proxy_Function>>(vec);
            return std::make_shared<Dispatch_Function>(std::move(vec));
        } else if (t_f->has_arithmetic_param()) {
            // if the function is the only function but it also contains
            // arithmetic operators, we must wrap it in a dispatch function
            // to allow for automatic arithmetic type conversions
            std::vector<Proxy_Function> vec;
            vec.push_back(t_f);
            funcs.insert(std::pair{
                t_name, std::make_shared<std::vector<Proxy_Function>>(vec)});
            return std::make_shared<Dispatch_Function>(std::move(vec));
        } else {
            auto vec = std::make_shared<std::vector<Proxy_Function>>();
            vec->push_back(t_f);
            funcs.insert(std::pair{t_name, vec});
            return t_f;
        }
    }();

    get_boxed_functions_int().insert_or_assign(t_name, const_var(new_func));
    get_function_objects_int().insert_or_assign(t_name, std::move(new_func));
}

Dispatch_State::Dispatch_State(Dispatch_Engine &t_engine)
    : m_engine(t_engine),
      m_stack_holder(t_engine.get_stack_holder()),
      m_conversions(t_engine.conversions(),
                    t_engine.conversions().conversion_saves()) {}

Dispatch_Engine *Dispatch_State::operator->() const { return &m_engine.get(); }

Dispatch_Engine &Dispatch_State::operator*() const { return m_engine.get(); }

Stack_Holder &Dispatch_State::stack_holder() const {
    return m_stack_holder.get();
}

const Type_Conversions_State &Dispatch_State::conversions() const {
    return m_conversions;
}

Type_Conversions::Conversion_Saves &Dispatch_State::conversion_saves() const {
    return m_conversions.saves();
}

Boxed_Value &Dispatch_State::add_get_object(const std::string &t_name,
                                            Boxed_Value obj) const {
    return m_engine.get().add_get_object(t_name, std::move(obj),
                                         m_stack_holder.get());
}

void Dispatch_State::add_object(const std::string &t_name,
                                Boxed_Value obj) const {
    m_engine.get().add_object(t_name, std::move(obj), m_stack_holder.get());
}

Boxed_Value Dispatch_State::get_object(std::string_view t_name,
                                       std::atomic_uint_fast32_t &t_loc) const {
    return m_engine.get().get_object(t_name, t_loc, m_stack_holder.get());
}

}  // namespace detail
}  // namespace Carbon
