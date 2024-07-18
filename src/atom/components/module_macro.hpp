// Helper macro to register initializers with dependencies and cleanup
#ifndef REGISTER_INITIALIZER
#define REGISTER_INITIALIZER(name, init_func, cleanup_func)       \
    namespace {                                                   \
    struct Initializer_##name {                                   \
        Initializer_##name() {                                    \
            Registry::instance().addInitializer(#name, init_func, \
                                                cleanup_func);    \
        }                                                         \
    };                                                            \
    static Initializer_##name initializer_##name;                 \
    }
#endif

#ifndef REGISTER_DEPENDENCY
#define REGISTER_DEPENDENCY(name, dependency)                       \
    namespace {                                                     \
    struct Dependency_##name {                                      \
        Dependency_##name() {                                       \
            Registry::instance().addDependency(#name, #dependency); \
        }                                                           \
    };                                                              \
    static Dependency_##name dependency_##name;                     \
    }
#endif

// Macro for dynamic library module
#ifndef ATOM_MODULE
#define ATOM_MODULE(module_name, init_func)                                    \
    namespace module_name {                                                    \
    struct ModuleManager {                                                     \
        static void init() {                                                   \
            static std::once_flag flag;                                        \
            std::call_once(flag, []() {                                        \
                init_func();                                                   \
                Registry::instance().initializeAll();                          \
            });                                                                \
        }                                                                      \
        static void cleanup() {                                                \
            static std::once_flag flag;                                        \
            std::call_once(flag, []() { Registry::instance().cleanupAll(); }); \
        }                                                                      \
    };                                                                         \
    }                                                                          \
    extern "C" void initialize_registry() {                                    \
        module_name::ModuleManager::init();                                    \
    }                                                                          \
    extern "C" void cleanup_registry() {                                       \
        module_name::ModuleManager::cleanup();                                 \
    }
#endif

// Macro for embedded module
#ifndef ATOM_EMBED_MODULE
#define ATOM_EMBED_MODULE(module_name, init_func)      \
    namespace module_name {                            \
    inline std::optional<std::once_flag> init_flag;    \
    struct ModuleInitializer {                         \
        ModuleInitializer() {                          \
            if (!init_flag.has_value()) {              \
                init_flag.emplace();                   \
                std::call_once(*init_flag, init_func); \
                Registry::instance().initializeAll();  \
            }                                          \
        }                                              \
        ~ModuleInitializer() {                         \
            if (init_flag.has_value()) {               \
                Registry::instance().cleanupAll();     \
                init_flag.reset();                     \
            }                                          \
        }                                              \
    };                                                 \
    static ModuleInitializer module_initializer;       \
    }
#endif
