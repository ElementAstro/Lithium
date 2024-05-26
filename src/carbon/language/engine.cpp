#include "engine.hpp"

namespace Carbon {
Boxed_Value Carbon_Basic::do_eval(const std::string &t_input,
                                  const std::string &t_filename = "__EVAL__",
                                  bool /* t_internal*/ = false) {
    try {
        const auto p = m_parser->parse(t_input, t_filename);
        return p->eval(Carbon::detail::Dispatch_State(m_engine));
    } catch (Carbon::eval::detail::Return_Value &rv) {
        return rv.retval;
    }
}

Boxed_Value Carbon_Basic::internal_eval_file(const std::string &t_filename) {
    for (const auto &path : m_use_paths) {
        try {
            const auto appendedpath = path + t_filename;
            return do_eval(load_file(appendedpath), appendedpath, true);
        } catch (const exception::file_not_found_error &) {
            // failed to load, try the next path
        } catch (const exception::eval_error &t_ee) {
            throw Boxed_Value(t_ee);
        }
    }

    // failed to load by any name
    throw exception::file_not_found_error(t_filename);
}

Boxed_Value Carbon_Basic::internal_eval(const std::string &t_e) {
    try {
        return do_eval(t_e, "__EVAL__", true);
    } catch (const exception::eval_error &t_ee) {
        throw Boxed_Value(t_ee);
    }
}

Carbon::detail::Dispatch_Engine &Carbon_Basic::get_eval_engine() noexcept {
    return m_engine;
}

void Carbon_Basic::build_eval_system(const ModulePtr &t_lib,
                                     const std::vector<Options> &t_opts) {
    if (t_lib) {
        add(t_lib);
    }

    m_engine.add(fun([this]() { m_engine.dump_system(); }), "dump_system");
    m_engine.add(
        fun([this](const Boxed_Value &t_bv) { m_engine.dump_object(t_bv); }),
        "dump_object");
    m_engine.add(
        fun([this](const Boxed_Value &t_bv, const std::string &t_type) {
            return m_engine.is_type(t_bv, t_type);
        }),
        "is_type");
    m_engine.add(fun([this](const Boxed_Value &t_bv) {
                     return m_engine.type_name(t_bv);
                 }),
                 "type_name");
    m_engine.add(fun([this](const std::string &t_f) {
                     return m_engine.function_exists(t_f);
                 }),
                 "function_exists");
    m_engine.add(fun([this]() { return m_engine.get_function_objects(); }),
                 "get_functions");
    m_engine.add(fun([this]() { return m_engine.get_scripting_objects(); }),
                 "get_objects");

    m_engine.add(dispatch::make_dynamic_proxy_function(
                     [this](const Function_Params &t_params) {
                         return m_engine.call_exists(t_params);
                     }),
                 "call_exists");

    m_engine.add(
        fun([this](const dispatch::Proxy_Function_Base &t_fun,
                   const std::vector<Boxed_Value> &t_params) -> Boxed_Value {
            Type_Conversions_State s(
                this->m_engine.conversions(),
                this->m_engine.conversions().conversion_saves());
            return t_fun(Function_Params{t_params}, s);
        }),
        "call");

    m_engine.add(fun([this](const atom::meta::Type_Info &t_ti) {
                     return m_engine.get_type_name(t_ti);
                 }),
                 "name");

    m_engine.add(fun([this](const std::string &t_type_name, bool t_throw) {
                     return m_engine.get_type(t_type_name, t_throw);
                 }),
                 "type");
    m_engine.add(fun([this](const std::string &t_type_name) {
                     return m_engine.get_type(t_type_name, true);
                 }),
                 "type");

    m_engine.add(
        fun([this](
                const atom::meta::Type_Info &t_from, const atom::meta::Type_Info &t_to,
                const std::function<Boxed_Value(const Boxed_Value &)> &t_func) {
            m_engine.add(Carbon::type_conversion(t_from, t_to, t_func));
        }),
        "add_type_conversion");

    if (std::find(t_opts.begin(), t_opts.end(), Options::No_Load_Modules) ==
            t_opts.end() &&
        std::find(t_opts.begin(), t_opts.end(), Options::Load_Modules) !=
            t_opts.end()) {
        m_engine.add(
            fun([this](const std::string &t_module, const std::string &t_file) {
                load_module(t_module, t_file);
            }),
            "load_module");
        m_engine.add(fun([this](const std::string &t_module) {
                         return load_module(t_module);
                     }),
                     "load_module");
    }

    if (std::find(t_opts.begin(), t_opts.end(), Options::No_External_Scripts) ==
            t_opts.end() &&
        std::find(t_opts.begin(), t_opts.end(), Options::External_Scripts) !=
            t_opts.end()) {
        m_engine.add(
            fun([this](const std::string &t_file) { return use(t_file); }),
            "use");
        m_engine.add(fun([this](const std::string &t_file) {
                         return internal_eval_file(t_file);
                     }),
                     "eval_file");
    }

    m_engine.add(
        fun([this](const std::string &t_str) { return internal_eval(t_str); }),
        "eval");
    m_engine.add(fun([this](const AST_Node &t_ast) { return eval(t_ast); }),
                 "eval");

    m_engine.add(fun([this](const std::string &t_str, const bool t_dump) {
                     return parse(t_str, t_dump);
                 }),
                 "parse");
    m_engine.add(fun([this](const std::string &t_str) { return parse(t_str); }),
                 "parse");

    m_engine.add(
        fun([this](const Boxed_Value &t_bv, const std::string &t_name) {
            add_global_const(t_bv, t_name);
        }),
        "add_global_const");
    m_engine.add(
        fun([this](const Boxed_Value &t_bv, const std::string &t_name) {
            add_global(t_bv, t_name);
        }),
        "add_global");
    m_engine.add(
        fun([this](const Boxed_Value &t_bv, const std::string &t_name) {
            set_global(t_bv, t_name);
        }),
        "set_global");

    // why this unused parameter to Namespace?
    m_engine.add(fun([this](const std::string &t_namespace_name) {
                     register_namespace([](Namespace & /*space*/) noexcept {},
                                        t_namespace_name);
                     import(t_namespace_name);
                 }),
                 "namespace");
    m_engine.add(fun([this](const std::string &t_namespace_name) {
                     import(t_namespace_name);
                 }),
                 "import");
}

bool Carbon_Basic::skip_bom(std::ifstream &infile) {
    size_t bytes_needed = 3;
    char buffer[3];

    memset(buffer, '\0', bytes_needed);

    infile.read(buffer, static_cast<std::streamsize>(bytes_needed));

    if ((buffer[0] == '\xef') && (buffer[1] == '\xbb') &&
        (buffer[2] == '\xbf')) {
        infile.seekg(3);
        return true;
    }

    infile.seekg(0);

    return false;
}

std::string Carbon_Basic::load_file(const std::string &t_filename) {
    std::ifstream infile(t_filename.c_str(),
                         std::ios::in | std::ios::ate | std::ios::binary);

    if (!infile.is_open()) {
        throw Carbon::exception::file_not_found_error(t_filename);
    }

    auto size = infile.tellg();
    infile.seekg(0, std::ios::beg);

    assert(size >= 0);

    if (skip_bom(infile)) {
        size -= 3;          // decrement the BOM size from file size, otherwise
                            // we'll get parsing errors
        assert(size >= 0);  // and check if there's more text
    }

    if (size == std::streampos(0)) {
        return std::string();
    } else {
        std::vector<char> v(static_cast<size_t>(size));
        infile.read(&v[0], static_cast<std::streamsize>(size));
        return std::string(v.begin(), v.end());
    }
}

std::vector<std::string> ensure_minimum_path_vec(
    std::vector<std::string> paths) {
    if (paths.empty()) {
        return {""};
    } else {
        return paths;
    }
}

Carbon_Basic::Carbon_Basic(
    const ModulePtr &t_lib,
    std::unique_ptr<parser::Carbon_Parser_Base> &&parser,
    std::vector<std::string> t_module_paths = {},
    std::vector<std::string> t_use_paths = {},
    const std::vector<Carbon::Options> &t_opts = Carbon::default_options())
    : m_module_paths(ensure_minimum_path_vec(std::move(t_module_paths))),
      m_use_paths(ensure_minimum_path_vec(std::move(t_use_paths))),
      m_parser(std::move(parser)),
      m_engine(*m_parser) {
#if !defined(CARBON_NO_DYNLOAD) && defined(_POSIX_VERSION) && \
    !defined(__CYGWIN__)
    // If on Unix, add the path of the current executable to the module
    // search path as windows would do

    union cast_union {
        Boxed_Value (Carbon_Basic::*in_ptr)(const std::string &);
        void *out_ptr;
    };

    Dl_info rInfo;
    memset(&rInfo, 0, sizeof(rInfo));
    cast_union u;
    u.in_ptr = &Carbon_Basic::use;
    if ((dladdr(static_cast<void *>(u.out_ptr), &rInfo) != 0) &&
        (rInfo.dli_fname != nullptr)) {
        std::string dllpath(rInfo.dli_fname);
        const size_t lastslash = dllpath.rfind('/');
        if (lastslash != std::string::npos) {
            dllpath.erase(lastslash);
        }

        // Let's see if this is a link that we should expand
        std::vector<char> buf(2048);
        const auto pathlen =
            readlink(dllpath.c_str(), &buf.front(), buf.size());
        if (pathlen > 0 && static_cast<size_t>(pathlen) < buf.size()) {
            dllpath = std::string(&buf.front(), static_cast<size_t>(pathlen));
        }

        m_module_paths.insert(m_module_paths.begin(), dllpath + "/");
    }
#endif
    build_eval_system(t_lib, t_opts);
}

#ifndef CARBON_NO_DYNLOAD
Carbon_Basic::Carbon_Basic(
    std::unique_ptr<parser::Carbon_Parser_Base> &&parser,
    std::vector<std::string> t_module_paths = {},
    std::vector<std::string> t_use_paths = {},
    const std::vector<Carbon::Options> &t_opts = Carbon::default_options())
    : Carbon_Basic({}, std::move(parser), t_module_paths, t_use_paths, t_opts) {
    try {
        // attempt to load the stdlib
        load_module("stdlib-" + Build_Info::version());
    } catch (const exception::load_module_error &t_err) {
        std::cout
            << "An error occurred while trying to load the chaiscript "
               "standard library.\n"
               "\n"
               "You must either provide a standard library, or compile it "
               "in.\n"
               "For an example of compiling the standard library in,\n"
               "see: https://gist.github.com/lefticus/9456197\n"
               "Compiling the stdlib in is the recommended and MOST "
               "SUPPORTED method.\n"
               "\n\n"
            << t_err.what();
        throw;
    }
}
#else  // CARBON_NO_DYNLOAD
Carbon_Basic::Carbon_Basic(std::unique_ptr<parser::Carbon_Parser_Base> &&parser,
                           std::vector<std::string> t_module_paths = {},
                           std::vector<std::string> t_use_paths = {},
                           const std::vector<Carbon::Options> &t_opts =
                               Carbon::default_options()) = delete;
#endif

parser::Carbon_Parser_Base &Carbon_Basic::get_parser() noexcept {
    return *m_parser;
}

const Boxed_Value Carbon_Basic::eval(const AST_Node &t_ast) {
    try {
        return t_ast.eval(Carbon::detail::Dispatch_State(m_engine));
    } catch (const exception::eval_error &t_ee) {
        throw Boxed_Value(t_ee);
    }
}

AST_NodePtr Carbon_Basic::parse(const std::string &t_input,
                                const bool t_debug_print) {
    auto ast = m_parser->parse(t_input, "PARSE");
    if (t_debug_print) {
        m_parser->debug_print(*ast);
    }
    return ast;
}

std::string Carbon_Basic::get_type_name(const atom::meta::Type_Info &ti) const {
    return m_engine.get_type_name(ti);
}

Boxed_Value Carbon_Basic::use(const std::string &t_filename) {
    for (const auto &path : m_use_paths) {
        const auto appendedpath = path + t_filename;
        try {
            Carbon::detail::threading::unique_lock<
                Carbon::detail::threading::recursive_mutex>
                l(m_use_mutex);
            Carbon::detail::threading::unique_lock<
                Carbon::detail::threading::shared_mutex>
                l2(m_mutex);

            Boxed_Value retval;

            if (m_used_files.count(appendedpath) == 0) {
                l2.unlock();
                retval = eval_file(appendedpath);
                l2.lock();
                m_used_files.insert(appendedpath);
            }

            return retval;  // return, we loaded it, or it was already
                            // loaded
        } catch (const exception::file_not_found_error &e) {
            if (e.filename != appendedpath) {
                // a nested file include failed
                throw;
            }
            // failed to load, try the next path
        }
    }

    // failed to load by any name
    throw exception::file_not_found_error(t_filename);
}

Carbon_Basic &Carbon_Basic::add_global_const(const Boxed_Value &t_bv,
                                             const std::string &t_name) {
    Name_Validator::validate_object_name(t_name);
    m_engine.add_global_const(t_bv, t_name);
    return *this;
}

Carbon_Basic &Carbon_Basic::add_global(const Boxed_Value &t_bv,
                                       const std::string &t_name) {
    Name_Validator::validate_object_name(t_name);
    m_engine.add_global(t_bv, t_name);
    return *this;
}

Carbon_Basic &Carbon_Basic::set_global(const Boxed_Value &t_bv,
                                       const std::string &t_name) {
    Name_Validator::validate_object_name(t_name);
    m_engine.set_global(t_bv, t_name);
    return *this;
}

Carbon_Basic::State Carbon_Basic::get_state() const {
    Carbon::detail::threading::lock_guard<
        Carbon::detail::threading::recursive_mutex>
        l(m_use_mutex);
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l2(m_mutex);

    State s;
    s.used_files = m_used_files;
    s.engine_state = m_engine.get_state();
    s.active_loaded_modules = m_active_loaded_modules;
    return s;
}

void Carbon_Basic::set_state(const State &t_state) {
    Carbon::detail::threading::lock_guard<
        Carbon::detail::threading::recursive_mutex>
        l(m_use_mutex);
    Carbon::detail::threading::shared_lock<
        Carbon::detail::threading::shared_mutex>
        l2(m_mutex);

    m_used_files = t_state.used_files;
    m_active_loaded_modules = t_state.active_loaded_modules;
    m_engine.set_state(t_state.engine_state);
}

std::map<std::string, Boxed_Value> Carbon_Basic::get_locals() const {
    return m_engine.get_locals();
}

void Carbon_Basic::set_locals(
    const std::map<std::string, Boxed_Value> &t_locals) {
    m_engine.set_locals(t_locals);
}

Carbon_Basic &Carbon_Basic::add(const Type_Conversion &d) {
    m_engine.add(d);
    return *this;
}

Carbon_Basic &Carbon_Basic::add(const ModulePtr &t_p) {
    t_p->apply(*this, this->get_eval_engine());
    return *this;
}

std::string Carbon_Basic::load_module(const std::string &t_module_name) {
#ifdef CARBON_NO_DYNLOAD
    throw Carbon::exception::load_module_error(
        "Loadable module support was disabled (CARBON_NO_DYNLOAD)");
#else
    std::vector<exception::load_module_error> errors;
    std::string version_stripped_name = t_module_name;
    size_t version_pos =
        version_stripped_name.find("-" + Build_Info::version());
    if (version_pos != std::string::npos) {
        version_stripped_name.erase(version_pos);
    }

    std::vector<std::string> prefixes{"lib", "cyg", ""};

    std::vector<std::string> postfixes{".dll", ".so", ".bundle", ""};

    for (auto &elem : m_module_paths) {
        for (auto &prefix : prefixes) {
            for (auto &postfix : postfixes) {
                try {
                    const auto name = elem + prefix + t_module_name + postfix;
                    // std::cerr << "trying location: " << name << '\n';
                    load_module(version_stripped_name, name);
                    return name;
                } catch (const Carbon::exception::load_module_error &e) {
                    // std::cerr << "error: " << e.what() << '\n';
                    errors.push_back(e);
                    // Try next set
                }
            }
        }
    }

    throw Carbon::exception::load_module_error(t_module_name, errors);
#endif
}

void Carbon_Basic::load_module(const std::string &t_module_name,
                               const std::string &t_filename) {
    Carbon::detail::threading::lock_guard<
        Carbon::detail::threading::recursive_mutex>
        l(m_use_mutex);

    if (m_loaded_modules.count(t_module_name) == 0) {
        detail::Loadable_Module_Ptr lm(
            new detail::Loadable_Module(t_module_name, t_filename));
        m_loaded_modules[t_module_name] = lm;
        m_active_loaded_modules.insert(t_module_name);
        add(lm->m_moduleptr);
    } else if (m_active_loaded_modules.count(t_module_name) == 0) {
        m_active_loaded_modules.insert(t_module_name);
        add(m_loaded_modules[t_module_name]->m_moduleptr);
    }
}

Boxed_Value Carbon_Basic::operator()(
    const std::string &t_script,
    const Exception_Handler &t_handler = Exception_Handler()) {
    return eval(t_script, t_handler);
}

Boxed_Value Carbon_Basic::eval(
    const std::string &t_input,
    const Exception_Handler &t_handler,
    const std::string &t_filename) {
    try {
        return do_eval(t_input, t_filename);
    } catch (Boxed_Value &bv) {
        if (t_handler) {
            t_handler->handle(bv, m_engine);
        }
        throw;
    }
}

Boxed_Value Carbon_Basic::eval_file(
    const std::string &t_filename,
    const Exception_Handler &t_handler) {
    return eval(load_file(t_filename), t_handler, t_filename);
}

void Carbon_Basic::import(const std::string &t_namespace_name) {
    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::recursive_mutex>
        l(m_use_mutex);

    if (m_engine.get_scripting_objects().count(t_namespace_name)) {
        throw std::runtime_error("Namespace: " + t_namespace_name +
                                 " was already defined");
    } else if (m_namespace_generators.count(t_namespace_name)) {
        m_engine.add_global(
            var(std::ref(m_namespace_generators[t_namespace_name]())),
            t_namespace_name);
    } else {
        throw std::runtime_error("No registered namespace: " +
                                 t_namespace_name);
    }
}

void Carbon_Basic::register_namespace(
    const std::function<void(Namespace &)> &t_namespace_generator,
    const std::string &t_namespace_name) {
    Carbon::detail::threading::unique_lock<
        Carbon::detail::threading::recursive_mutex>
        l(m_use_mutex);

    if (!m_namespace_generators.count(t_namespace_name)) {
        // contain the namespace object memory within the
        // m_namespace_generators map
        m_namespace_generators.emplace(
            std::make_pair(t_namespace_name,
                           [=, space = Namespace()]() mutable -> Namespace & {
                               t_namespace_generator(space);
                               return space;
                           }));
    } else {
        throw std::runtime_error("Namespace: " + t_namespace_name +
                                 " was already registered.");
    }
}

}  // namespace Carbon
