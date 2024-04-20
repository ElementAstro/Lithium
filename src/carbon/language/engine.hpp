

#ifndef CARBON_ENGINE_HPP
#define CARBON_ENGINE_HPP

#include <cassert>
#include <cstring>
#include <exception>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <vector>

#include "../command/boxed_cast_helper.hpp"
#include "../command/boxed_value.hpp"
#include "../command/dispatchkit.hpp"
#include "../command/proxy_functions.hpp"
#include "../command/register_function.hpp"
#include "../command/type_conversions.hpp"
#include "../defines.hpp"
#include "../threading.hpp"
#include "common.hpp"

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || \
    defined(__HAIKU__)
#include <unistd.h>
#endif

#if !defined(CARBON_NO_DYNLOAD) && defined(_POSIX_VERSION) && \
    !defined(__CYGWIN__)
#include <dlfcn.h>
#endif

#if defined(CARBON_NO_DYNLOAD)
#include "unknown.hpp"
#elif defined(CARBON_WINDOWS)
#include "windows.hpp"
#elif _POSIX_VERSION
#include "posix.hpp"
#else
#include "unknown.hpp"
#endif

#include "../command/exception_specification.hpp"

namespace Carbon {
/// Namespace alias to provide cleaner and more explicit syntax to users.
using Namespace = dispatch::Dynamic_Object;

namespace detail {
using Loadable_Module_Ptr = std::shared_ptr<Loadable_Module>;
}

/// \brief The main object that the ChaiScript user will use.
class Carbon_Basic {
    mutable Carbon::detail::threading::shared_mutex m_mutex;
    mutable Carbon::detail::threading::recursive_mutex m_use_mutex;

    std::set<std::string> m_used_files;
    std::map<std::string, detail::Loadable_Module_Ptr> m_loaded_modules;
    std::set<std::string> m_active_loaded_modules;

    std::vector<std::string> m_module_paths;
    std::vector<std::string> m_use_paths;

    std::unique_ptr<parser::Carbon_Parser_Base> m_parser;

    Carbon::detail::Dispatch_Engine m_engine;

    std::map<std::string, std::function<Namespace &()>> m_namespace_generators;

    /// Evaluates the given string in by parsing it and running the results
    /// through the evaluator
    Boxed_Value do_eval(const std::string &t_input,
                        const std::string &t_filename, bool /* t_internal*/);

    /// Evaluates the given file and looks in the 'use' paths
    Boxed_Value internal_eval_file(const std::string &t_filename);

    /// Evaluates the given string, used during eval() inside of a script
    Boxed_Value internal_eval(const std::string &t_e);

    /// Returns the current evaluation m_engine
    Carbon::detail::Dispatch_Engine &get_eval_engine() noexcept;

    /// Builds all the requirements for ChaiScript, including its evaluator and
    /// a run of its prelude.
    void build_eval_system(const ModulePtr &t_lib,
                           const std::vector<Options> &t_opts);

    /// Skip BOM at the beginning of file
    static bool skip_bom(std::ifstream &infile);

    /// Helper function for loading a file
    static std::string load_file(const std::string &t_filename);

    std::vector<std::string> ensure_minimum_path_vec(
        std::vector<std::string> paths);

public:
    /// \brief Virtual destructor for ChaiScript
    virtual ~Carbon_Basic() = default;

    /// \brief Constructor for ChaiScript
    /// \param[in] t_lib Standard library to apply to this ChaiScript instance
    /// \param[in] t_modulepaths Vector of paths to search when attempting to
    /// load a binary module \param[in] t_usepaths Vector of paths to search
    /// when attempting to "use" an included ChaiScript file
    Carbon_Basic(const ModulePtr &t_lib,
                 std::unique_ptr<parser::Carbon_Parser_Base> &&parser,
                 std::vector<std::string> t_module_paths,
                 std::vector<std::string> t_use_paths,
                 const std::vector<Carbon::Options> &t_opts);

#ifndef CARBON_NO_DYNLOAD
    /// \brief Constructor for ChaiScript.
    ///
    /// This version of the ChaiScript constructor attempts to find the stdlib
    /// module to load at runtime generates an error if it cannot be found.
    ///
    /// \param[in] t_modulepaths Vector of paths to search when attempting to
    /// load a binary module \param[in] t_usepaths Vector of paths to search
    /// when attempting to "use" an included ChaiScript file
    explicit Carbon_Basic(std::unique_ptr<parser::Carbon_Parser_Base> &&parser,
                          std::vector<std::string> t_module_paths,
                          std::vector<std::string> t_use_paths,
                          const std::vector<Carbon::Options> &t_opts);
#else  // CARBON_NO_DYNLOAD
    explicit Carbon_Basic(std::unique_ptr<parser::Carbon_Parser_Base> &&parser,
                          std::vector<std::string> t_module_paths,
                          std::vector<std::string> t_use_paths,
                          const std::vector<Carbon::Options> &t_opts) = delete;
#endif

    parser::Carbon_Parser_Base &get_parser() noexcept;

    const Boxed_Value eval(const AST_Node &t_ast);

    AST_NodePtr parse(const std::string &t_input, const bool t_debug_print = false);

    std::string get_type_name(const Type_Info &ti) const;

    template <typename T>
    std::string get_type_name() const {
        return get_type_name(user_type<T>());
    }

    /// \brief Loads and parses a file. If the file is already open, it is not
    /// reloaded. The use paths specified at ChaiScript construction time are
    /// searched for the requested file.
    ///
    /// \param[in] t_filename Filename to load and evaluate
    Boxed_Value use(const std::string &t_filename);

    /// \brief Adds a constant object that is available in all contexts and to
    /// all threads \param[in] t_bv Boxed_Value to add as a global \param[in]
    /// t_name Name of the value to add \throw
    /// Carbon::exception::global_non_const If t_bv is not a constant object
    /// \sa Boxed_Value::is_const
    Carbon_Basic &add_global_const(const Boxed_Value &t_bv,
                                   const std::string &t_name);

    /// \brief Adds a mutable object that is available in all contexts and to
    /// all threads \param[in] t_bv Boxed_Value to add as a global \param[in]
    /// t_name Name of the value to add \warning The user is responsible for
    /// making sure the object is thread-safe if necessary
    ///          ChaiScript is thread-safe but provides no threading locking
    ///          mechanism to the script
    Carbon_Basic &add_global(const Boxed_Value &t_bv,
                             const std::string &t_name);

    Carbon_Basic &set_global(const Boxed_Value &t_bv,
                             const std::string &t_name);

    /// \brief Represents the current state of the ChaiScript system. State and
    /// be saved and restored \warning State object does not contain the user
    /// defined type conversions of the engine. They
    ///          are left out due to performance considerations involved in
    ///          tracking the state
    /// \sa ChaiScript::get_state
    /// \sa ChaiScript::set_state
    struct State {
        std::set<std::string> used_files;
        Carbon::detail::Dispatch_Engine::State engine_state;
        std::set<std::string> active_loaded_modules;
    };

    /// \brief Returns a state object that represents the current state of the
    /// global system
    ///
    /// The global system includes the reserved words, global const objects,
    /// functions and types. local variables are thread specific and not
    /// included.
    ///
    /// \return Current state of the global system
    ///
    /// \b Example:
    ///
    /// \code
    /// Carbon::ChaiScript chai;
    /// Carbon::ChaiScript::State s = chai.get_state(); // represents
    /// bootstrapped initial state \endcode
    State get_state() const;

    /// \brief Sets the state of the system
    ///
    /// The global system includes the reserved words, global objects, functions
    /// and types. local variables are thread specific and not included.
    ///
    /// \param[in] t_state New state to set
    ///
    /// \b Example:
    /// \code
    /// Carbon::ChaiScript chai;
    /// Carbon::ChaiScript::State s = chai.get_state(); // get initial state
    /// chai.add(Carbon::fun(&somefunction), "somefunction");
    /// chai.set_state(s); // restore initial state, which does not have the
    /// recently added "somefunction" \endcode
    void set_state(const State &t_state);

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

    /// \brief Adds a type, function or object to ChaiScript. Objects are added
    /// to the local thread state. \param[in] t_t Item to add \param[in] t_name
    /// Name of item to add \returns Reference to current ChaiScript object
    ///
    /// \b Examples:
    /// \code
    /// Carbon::ChaiScript chai;
    /// chai.add(Carbon::user_type<MyClass>(), "MyClass"); // Add explicit
    /// type info (not strictly necessary)
    /// chai.add(Carbon::fun(&MyClass::function), "function"); // Add a
    /// class method MyClass obj; chai.add(Carbon::var(&obj), "obj"); // Add
    /// a pointer to a locally defined object \endcode
    ///
    /// \sa \ref adding_items
    template <typename T>
    Carbon_Basic &add(const T &t_t, const std::string &t_name) {
        Name_Validator::validate_object_name(t_name);
        m_engine.add(t_t, t_name);
        return *this;
    }

    /// \brief Add a new conversion for upcasting to a base class
    /// \sa Carbon::base_class
    /// \param[in] d Base class / parent class
    ///
    /// \b Example:
    /// \code
    /// Carbon::ChaiScript chai;
    /// chai.add(Carbon::base_class<std::runtime_error,
    /// Carbon::dispatch_error>()); \endcode
    Carbon_Basic &add(const Type_Conversion &d);

    /// \brief Adds all elements of a module to ChaiScript runtime
    /// \param[in] t_p The module to add.
    /// \sa Carbon::Module
    Carbon_Basic &add(const ModulePtr &t_p);

    /// \brief Load a binary module from a dynamic library. Works on platforms
    /// that support
    ///        dynamic libraries.
    /// \param[in] t_module_name Name of the module to load
    ///
    /// The module is searched for in the registered module path folders
    /// (Carbon::ChaiScript::ChaiScript) and with standard prefixes and
    /// postfixes: ("lib"|"")\<t_module_name\>(".dll"|".so"|".bundle"|"").
    ///
    /// Once the file is located, the system looks for the symbol
    /// "create_module_\<t_module_name\>". If no file can be found
    /// matching the search criteria and containing the appropriate entry point
    /// (the symbol mentioned above), an exception is thrown.
    ///
    /// \throw Carbon::exception::load_module_error In the event that no
    /// matching module can be found.
    std::string load_module(const std::string &t_module_name);

    /// \brief Load a binary module from a dynamic library. Works on platforms
    /// that support
    ///        dynamic libraries.
    ///
    /// \param[in] t_module_name Module name to load
    /// \param[in] t_filename Ignore normal filename search process and use
    /// specific filename
    ///
    /// \sa ChaiScript::load_module(const std::string &t_module_name)
    void load_module(const std::string &t_module_name,
                     const std::string &t_filename);

    /// \brief Evaluates a string. Equivalent to ChaiScript::eval.
    ///
    /// \param[in] t_script Script to execute
    /// \param[in] t_handler Optional Exception_Handler used for automatic
    /// unboxing of script thrown exceptions
    ///
    /// \return result of the script execution
    ///
    /// \throw Carbon::exception::eval_error In the case that evaluation
    /// fails.
    Boxed_Value operator()(const std::string &t_script,
                           const Exception_Handler &t_handler);

    /// \brief Evaluates a string and returns a typesafe result.
    ///
    /// \tparam T Type to extract from the result value of the script execution
    /// \param[in] t_input Script to execute
    /// \param[in] t_handler Optional Exception_Handler used for automatic
    /// unboxing of script thrown exceptions \param[in] t_filename Optional
    /// filename to report to the user for where the error occured. Useful
    ///                       in special cases where you are loading a file
    ///                       internally instead of using eval_file
    ///
    /// \return result of the script execution
    ///
    /// \throw Carbon::exception::eval_error In the case that evaluation
    /// fails. \throw Carbon::exception::bad_boxed_cast In the case that
    /// evaluation succeeds but the result value cannot be converted
    ///        to the requested type.
    template <typename T>
    T eval(const std::string &t_input,
           const Exception_Handler &t_handler = Exception_Handler(),
           const std::string &t_filename = "__EVAL__") {
        return m_engine.boxed_cast<T>(eval(t_input, t_handler, t_filename));
    }

    /// \brief casts an object while applying any Dynamic_Conversion available
    template <typename Type>
    decltype(auto) boxed_cast(const Boxed_Value &bv) const {
        return (m_engine.boxed_cast<Type>(bv));
    }

    /// \brief Evaluates a string.
    ///
    /// \param[in] t_input Script to execute
    /// \param[in] t_handler Optional Exception_Handler used for automatic
    /// unboxing of script thrown exceptions \param[in] t_filename Optional
    /// filename to report to the user for where the error occurred. Useful
    ///                       in special cases where you are loading a file
    ///                       internally instead of using eval_file
    ///
    /// \return result of the script execution
    ///
    /// \throw exception::eval_error In the case that evaluation fails.
    Boxed_Value eval(const std::string &t_input,
                     const Exception_Handler &t_handler = Exception_Handler(),
                     const std::string &t_filename = "__EVAL__");

    /// \brief Loads the file specified by filename, evaluates it, and returns
    /// the result. \param[in] t_filename File to load and parse. \param[in]
    /// t_handler Optional Exception_Handler used for automatic unboxing of
    /// script thrown exceptions \return result of the script execution \throw
    /// Carbon::exception::eval_error In the case that evaluation fails.
    Boxed_Value eval_file(
        const std::string &t_filename,
        const Exception_Handler &t_handler = Exception_Handler());

    /// \brief Loads the file specified by filename, evaluates it, and returns
    /// the type safe result. \tparam T Type to extract from the result value of
    /// the script execution \param[in] t_filename File to load and parse.
    /// \param[in] t_handler Optional Exception_Handler used for automatic
    /// unboxing of script thrown exceptions \return result of the script
    /// execution \throw Carbon::exception::eval_error In the case that
    /// evaluation fails. \throw Carbon::exception::bad_boxed_cast In the
    /// case that evaluation succeeds but the result value cannot be converted
    ///        to the requested type.
    template <typename T>
    T eval_file(const std::string &t_filename,
                const Exception_Handler &t_handler = Exception_Handler()) {
        return m_engine.boxed_cast<T>(eval_file(t_filename, t_handler));
    }

    /// \brief Imports a namespace object into the global scope of this
    /// ChaiScript instance. \param[in] t_namespace_name Name of the namespace
    /// to import. \throw std::runtime_error In the case that the namespace name
    /// was never registered.
    void import(const std::string &t_namespace_name);

    /// \brief Registers a namespace generator, which delays generation of the
    /// namespace until it is imported, saving memory if it is never used.
    /// \param[in] t_namespace_generator Namespace generator function.
    /// \param[in] t_namespace_name Name of the Namespace function being
    /// registered. \throw std::runtime_error In the case that the namespace
    /// name was already registered.
    void register_namespace(
        const std::function<void(Namespace &)> &t_namespace_generator,
        const std::string &t_namespace_name);
};

}  // namespace Carbon
#endif /* CARBON_ENGINE_HPP */
