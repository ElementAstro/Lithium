// Helper macro to register initializers with dependencies and cleanup
#define REGISTER_INITIALIZER(name, init_func, cleanup_func)       \
    namespace {                                                   \
    struct Initializer {                                          \
        Initializer() {                                           \
            Registry::instance().add_initializer(name, init_func, \
                                                 cleanup_func);   \
        }                                                         \
    };                                                            \
    static Initializer initializer;                               \
    }

#define REGISTER_DEPENDENCY(name, dependency)                      \
    namespace {                                                    \
    struct Dependency {                                            \
        Dependency() {                                             \
            Registry::instance().add_dependency(name, dependency); \
        }                                                          \
    };                                                             \
    static Dependency dependency;                                  \
    }

// Macro for dynamic library module
#define ATOM_MODULE(module_name, init_func)                                    \
    extern "C" void initialize_registry() {                                    \
        init_func();                                                           \
        Registry::instance().initialize_all();                                 \
    }                                                                          \
    extern "C" void cleanup_registry() { Registry::instance().cleanup_all(); } \
    namespace module_name {                                                    \
    struct ModuleInitializer {                                                 \
        ModuleInitializer() { init_func(); }                                   \
    };                                                                         \
    static ModuleInitializer module_initializer;                               \
    }

// Macro for embedded module
#define ATOM_EMBED_MODULE(module_name, init_func)                    \
    namespace module_name {                                          \
    struct ModuleInitializer {                                       \
        ModuleInitializer() {                                        \
            init_func();                                             \
            Registry::instance().initialize_all();                   \
        }                                                            \
        ~ModuleInitializer() { Registry::instance().cleanup_all(); } \
    };                                                               \
    static ModuleInitializer module_initializer;                     \
    }