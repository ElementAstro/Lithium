#ifndef LITHIUM_ADDON_LOADER_INL
#define LITHIUM_ADDON_LOADER_INL

#include "loader.hpp"

namespace lithium {

template <typename T>
T ModuleLoader::GetFunction(const std::string &name,
                            const std::string &function_name) {
    std::shared_lock lock(m_SharedMutex);
    auto handle_it = modules_.find(name);
    if (handle_it == modules_.end()) {
        LOG_F(ERROR, "Failed to find module {}", name);
        return nullptr;
    }

    auto func_ptr = reinterpret_cast<T>(
        LOAD_FUNCTION(handle_it->second->handle, function_name.c_str()));

    if (!func_ptr) {
        LOG_F(ERROR, "Failed to get symbol {} from module {}: {}",
              function_name, name, dlerror());
        return nullptr;
    }

    return func_ptr;
}

template <typename T>
std::shared_ptr<T> ModuleLoader::GetInstance(const std::string &name,
                                             const json &config,
                                             const std::string &symbol_name) {
    std::shared_lock lock(m_SharedMutex);
    if (!HasModule(name)) {
        LOG_F(ERROR, "Failed to find module {}", name);
        return nullptr;
    }
    auto get_instance_func =
        GetFunction<std::shared_ptr<T> (*)(const json &)>(name, symbol_name);
    if (!get_instance_func) {
        LOG_F(ERROR, "Failed to get symbol {} from module {}: {}", symbol_name,
              name, dlerror());
        return nullptr;
    }

    return get_instance_func(config);
}

template <typename T>
std::unique_ptr<T> ModuleLoader::GetUniqueInstance(
    const std::string &name, const json &config,
    const std::string &instance_function_name) {
    std::shared_lock lock(m_SharedMutex);
    if (!HasModule(name)) {
        LOG_F(ERROR, "Failed to find module {}", name);
        return nullptr;
    }
    auto get_instance_func = GetFunction<std::unique_ptr<T> (*)(const json &)>(
        name, instance_function_name);
    if (!get_instance_func) {
        LOG_F(ERROR, "Failed to get symbol {} from module {}: {}",
              instance_function_name, name, dlerror());
        return nullptr;
    }

    return get_instance_func(config);
}

template <typename T>
std::shared_ptr<T> ModuleLoader::GetInstancePointer(
    const std::string &name, const json &config,
    const std::string &instance_function_name) {
    return GetInstance<T>(name, config, instance_function_name);
}

}  // namespace lithium

#endif