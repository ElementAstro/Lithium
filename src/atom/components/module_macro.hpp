// Helper macros for registering initializers, dependencies, and modules
#ifndef REGISTER_INITIALIZER
#define REGISTER_INITIALIZER(name, init_func, cleanup_func)       \
    namespace {                                                   \
    struct Initializer_##name {                                   \
        Initializer_##name() {                                    \
            LOG_F(INFO, "Registering initializer: {}", #name);    \
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
            LOG_F(INFO, "Registering dependency: {} -> {}", #name,  \
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
            LOG_F(INFO, "Initializing module: {}", #module_name);         \
            Registry::instance().registerModule(#module_name, init_func); \
            Registry::instance().addInitializer(#module_name, init_func); \
            Registry::instance().initializeAll();                         \
        }                                                                 \
        static void cleanup() {                                           \
            static std::once_flag flag;                                   \
            std::call_once(flag, []() {                                   \
                LOG_F(INFO, "Cleaning up module: {}", #module_name);      \
                Registry::instance().cleanupAll();                        \
            });                                                           \
        }                                                                 \
    };                                                                    \
    }
#endif

// Macro for dynamic library module
#ifndef ATOM_MODULE
#define ATOM_MODULE(module_name, init_func)                                   \
    ATOM_MODULE_INIT(module_name, init_func)                                  \
    extern "C" void module_name##_initialize_registry() {                     \
        LOG_F(INFO, "Initializing registry for module: {}", #module_name);    \
        module_name::ModuleManager::init();                                   \
        LOG_F(INFO, "Initialized registry for module: {}", #module_name);     \
    }                                                                         \
    extern "C" void module_name##_cleanup_registry() {                        \
        LOG_F(INFO, "Cleaning up registry for module: {}", #module_name);     \
        module_name::ModuleManager::cleanup();                                \
        LOG_F(INFO, "Cleaned up registry for module: {}", #module_name);      \
    }                                                                         \
    extern "C" auto module_name##_getInstance()->std::shared_ptr<Component> { \
        LOG_F(INFO, "Getting instance of module: {}", #module_name);          \
        return Registry::instance().getComponent(#module_name);               \
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
                LOG_F(INFO, "Embedding module: {}", #module_name);            \
                init_flag.emplace();                                          \
                Registry::instance().registerModule(#module_name, init_func); \
                Registry::instance().addInitializer(#module_name, init_func); \
            }                                                                 \
        }                                                                     \
        ~ModuleInitializer() {                                                \
            if (init_flag.has_value()) {                                      \
                LOG_F(INFO, "Cleaning up embedded module: {}", #module_name); \
                init_flag.reset();                                            \
            }                                                                 \
        }                                                                     \
    };                                                                        \
    static ModuleInitializer module_initializer;                              \
    }
#endif

// Macro for dynamic library module with test support
// Max: This means that the module is a dynamic library that can be loaded at
// runtime.
//      And the test function should hava a signature like void
//      test_func(std::shared_ptr<Component> instance).
#ifndef ATOM_MODULE_TEST
#define ATOM_MODULE_TEST(module_name, test_func)                 \
    extern "C" void module_name##_run_tests() {                             \
        LOG_F(INFO, "Running tests for module: {}", #module_name);          \
        try {                                                               \
            test_func(module_name##_getInstance());                                       \
        } catch (const atom::error::ObjectNotExist& e) {                    \
            LOG_F(ERROR, "{} not found", #module_name);                     \
        } catch (const std::exception& e) {                                 \
            LOG_F(ERROR, "Exception thrown: {} in {}'s tests", e.what(),    \
                  #module_name);                                            \
        }                                                                   \
        LOG_F(INFO, "Finished running tests for module: {}", #module_name); \
    }
#endif

// Macro for embedded module with test support
#ifndef ATOM_EMBED_MODULE_TEST
#define ATOM_EMBED_MODULE_TEST(module_name, init_func, test_func)             \
    ATOM_MODULE_INIT(module_name, init_func)                                  \
    namespace module_name {                                                   \
    inline std::optional<std::once_flag> init_flag;                           \
    struct ModuleInitializer {                                                \
        ModuleInitializer() {                                                 \
            if (!init_flag.has_value()) {                                     \
                LOG_F(INFO, "Embedding module: {}", #module_name);            \
                init_flag.emplace();                                          \
                Registry::instance().registerModule(#module_name, init_func); \
                Registry::instance().addInitializer(#module_name, init_func); \
            }                                                                 \
        }                                                                     \
        ~ModuleInitializer() {                                                \
            if (init_flag.has_value()) {                                      \
                LOG_F(INFO, "Cleaning up embedded module: {}", #module_name); \
                init_flag.reset();                                            \
            }                                                                 \
        }                                                                     \
    };                                                                        \
    static ModuleInitializer module_initializer;                              \
    }                                                                         \
    extern "C" void run_tests() {                                             \
        LOG_F(INFO, "Running tests for module: {}", #module_name);            \
        try {                                                                 \
            test_func(module_name::getInstance());                            \
        } catch (const atom::error::ObjectNotExist& e) {                      \
            LOG_F(ERROR, "{} not found", #module_name);                       \
        } catch (const std::exception& e) {                                   \
            LOG_F(ERROR, "Exception thrown: {} in {}'s tests", e.what(),      \
                  #module_name);                                              \
        }                                                                     \
        LOG_F(INFO, "Finished running tests for module: {}", #module_name);   \
    }
#endif
