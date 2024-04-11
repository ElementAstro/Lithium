

#ifndef CARBON_POSIX_HPP
#define CARBON_POSIX_HPP

namespace Carbon::detail {
struct Loadable_Module {
    struct DLModule {
        explicit DLModule(const std::string &t_filename)
            : m_data(dlopen(t_filename.c_str(), RTLD_NOW)) {
            if (m_data == nullptr) {
                throw Carbon::exception::load_module_error(dlerror());
            }
        }

        DLModule(DLModule &&) = default;
        DLModule &operator=(DLModule &&) = default;
        DLModule(const DLModule &) = delete;
        DLModule &operator=(const DLModule &) = delete;

        ~DLModule() { dlclose(m_data); }

        void *m_data;
    };

    template <typename T>
    struct DLSym {
        DLSym(DLModule &t_mod, const std::string &t_symbol)
            : m_symbol(
                  reinterpret_cast<T>(dlsym(t_mod.m_data, t_symbol.c_str()))) {
            if (!m_symbol) {
                throw Carbon::exception::load_module_error(dlerror());
            }
        }

        T m_symbol;
    };

    Loadable_Module(const std::string &t_module_name,
                    const std::string &t_filename)
        : m_dlmodule(t_filename),
          m_func(m_dlmodule, "create_module_" + t_module_name),
          m_moduleptr(m_func.m_symbol()) {}

    DLModule m_dlmodule;
    DLSym<Create_Module_Func> m_func;
    ModulePtr m_moduleptr;
};
}  // namespace Carbon::detail
#endif
