// Helper macro to register initializers with dependencies and cleanup
#ifndef REGISTER_INITIALIZER
#define REGISTER_INITIALIZER(name, init_func, cleanup_func)        \
    namespace {                                                    \
    struct Initializer_##name {                                    \
        Initializer_##name() {                                     \
            Registry::instance().add_initializer(#name, init_func, \
                                                 cleanup_func);    \
        }                                                          \
    };                                                             \
    static Initializer_##name initializer_##name;                  \
    }

#endif

#ifndef REGISTER_DEPENDENCY
#define REGISTER_DEPENDENCY(name, dependency)                        \
    namespace {                                                      \
    struct Dependency_##name {                                       \
        Dependency_##name() {                                        \
            Registry::instance().add_dependency(#name, #dependency); \
        }                                                            \
    };                                                               \
    static Dependency_##name dependency_##name;                      \
    }
#endif

// Macro for dynamic library module
#ifndef ATOM_MODULE
#define ATOM_MODULE(module_name, init_func)                               \
    namespace module_name {                                               \
    struct ModuleManager {                                                \
        static void init() {                                              \
            static std::once_flag flag;                                   \
            std::call_once(flag, []() {                                   \
                init_func();                                              \
                Registry::instance().initialize_all();                    \
            });                                                           \
        }                                                                 \
        static void cleanup() {                                           \
            static std::once_flag flag;                                   \
            std::call_once(flag,                                          \
                           []() { Registry::instance().cleanup_all(); }); \
        }                                                                 \
    };                                                                    \
    }                                                                     \
    extern "C" void initialize_registry() {                               \
        module_name::ModuleManager::init();                               \
    }                                                                     \
    extern "C" void cleanup_registry() {                                  \
        module_name::ModuleManager::cleanup();                            \
    }
#endif

// Macro for embedded module
#ifndef ATOM_EMBED_MODULE
#define ATOM_EMBED_MODULE(module_name, init_func)         \
    namespace module_name {                               \
    inline std::optional<std::once_flag> init_flag;       \
    struct ModuleInitializer {                            \
        ModuleInitializer() {                             \
            std::call_once(init_flag.value(), init_func); \
            Registry::instance().initialize_all();        \
        }                                                 \
        ~ModuleInitializer() {                            \
            if (init_flag) {                              \
                Registry::instance().cleanup_all();       \
                init_flag.reset();                        \
            }                                             \
        }                                                 \
    };                                                    \
    static ModuleInitializer module_initializer;          \
    }
#endif