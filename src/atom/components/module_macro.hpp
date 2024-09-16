// Helper macros for registering initializers, dependencies, and modules
#ifndef REGISTER_INITIALIZER
#define REGISTER_INITIALIZER(name, init_func, cleanup_func)       \
    namespace {                                                   \
    struct Initializer_##name {                                   \
        Initializer_##name() {                                    \
            LOG_F(INFO, "Registering initializer: %s", #name);    \
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
            LOG_F(INFO, "Registering dependency: %s -> %s", #name,  \
                  #dependency);                                     \
            Registry::instance().addDependency(#name, #dependency); \
        }                                                           \
    };                                                              \
    static Dependency_##name dependency_##name;                     \
    }
#endif

// Nested macro for module initialization
#ifndef ATOM_MODULE_INIT
#define ATOM_MODULE_INIT(module_name, init_func)                          \
    namespace module_name {                                               \
    struct ModuleManager {                                                \
        static void init() {                                              \
            LOG_F(INFO, "Initializing module: %s", #module_name);         \
            Registry::instance().registerModule(#module_name, init_func); \
            Registry::instance().addInitializer(#module_name, init_func); \
            Registry::instance().initializeAll();                         \
        }                                                                 \
        static void cleanup() {                                           \
            static std::once_flag flag;                                   \
            std::call_once(flag, []() {                                   \
                LOG_F(INFO, "Cleaning up module: %s", #module_name);      \
                Registry::instance().cleanupAll();                        \
            });                                                           \
        }                                                                 \
    };                                                                    \
    }
#endif

// Macro for dynamic library module
#ifndef ATOM_MODULE
#define ATOM_MODULE(module_name, init_func)                                \
    ATOM_MODULE_INIT(module_name, init_func)                               \
    extern "C" void initialize_registry() {                                \
        LOG_F(INFO, "Initializing registry for module: %s", #module_name); \
        module_name::ModuleManager::init();                                \
    }                                                                      \
    extern "C" void cleanup_registry() {                                   \
        LOG_F(INFO, "Cleaning up registry for module: %s", #module_name);  \
        module_name::ModuleManager::cleanup();                             \
    }                                                                      \
    extern "C" auto getInstance() -> std::shared_ptr<Component> {          \
        LOG_F(INFO, "Getting instance of module: %s", #module_name);       \
        return Registry::instance().getComponent(#module_name);            \
    }
#endif

// Macro for embedded module
#ifndef ATOM_EMBED_MODULE
#define ATOM_EMBED_MODULE(module_name, init_func)                             \
    ATOM_MODULE_INIT(module_name, init_func)                                  \
    namespace module_name {                                                   \
    inline std::optional<std::once_flag> init_flag;                           \
    struct ModuleInitializer {                                                \
        ModuleInitializer() {                                                 \
            if (!init_flag.has_value()) {                                     \
                LOG_F(INFO, "Embedding module: %s", #module_name);            \
                init_flag.emplace();                                          \
                Registry::instance().registerModule(#module_name, init_func); \
                Registry::instance().addInitializer(#module_name, init_func); \
            }                                                                 \
        }                                                                     \
        ~ModuleInitializer() {                                                \
            if (init_flag.has_value()) {                                      \
                LOG_F(INFO, "Cleaning up embedded module: %s", #module_name); \
                init_flag.reset();                                            \
            }                                                                 \
        }                                                                     \
    };                                                                        \
    static ModuleInitializer module_initializer;                              \
    }
#endif