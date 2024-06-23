#include "registry.hpp"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <vector>

#include "atom/error/exception.hpp"

void Registry::add_initializer(const std::string& name, InitFunc init_func,
                               CleanupFunc cleanup_func) {
    std::scoped_lock lock(mutex_);
    if (initializers.find(name) != initializers.end()) {
        THROW_OBJ_ALREADY_INITIALIZED("Initializer already registered: " +
                                      name);
    }
    initializers[name] = {std::move(init_func), std::move(cleanup_func)};
    initialized[name] = false;
}

void Registry::add_dependency(const std::string& name,
                              const std::string& dependency) {
    std::unique_lock lock(mutex_);
    if (has_circular_dependency(name, dependency)) {
        THROW_RUNTIME_ERROR("Circular dependency detected: " + name + " -> " +
                            dependency);
    }
    dependencies[name].insert(dependency);
}

void Registry::initialize_all() {
    std::unique_lock lock(mutex_);
    std::unordered_set<std::string> init_stack;
    for (const auto& pair : initializers) {
        initialize_component(pair.first, init_stack);
    }
}

void Registry::cleanup_all() {
    std::unique_lock lock(mutex_);
    std::vector<std::string> keys;
    for (const auto& pair : initializers) {
        keys.push_back(pair.first);
    }
    std::reverse(keys.begin(), keys.end());

    for (const auto& key : keys) {
        if (initializers[key].cleanup_func && initialized[key]) {
            initializers[key].cleanup_func();
            initialized[key] = false;
        }
    }
}

bool Registry::is_initialized(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = initialized.find(name);
    return it != initialized.end() && it->second;
}

void Registry::reinitialize_component(const std::string& name) {
    std::unique_lock lock(mutex_);
    if (initializers.find(name) == initializers.end()) {
        THROW_OBJ_NOT_EXIST("Component not registered: " + name);
    }
    std::unordered_set<std::string> init_stack;
    initialize_component(name, init_stack);
}

bool Registry::has_circular_dependency(const std::string& name,
                                       const std::string& dependency) {
    if (dependencies[dependency].count(name)) {
        return true;
    }
    for (const auto& dep : dependencies[dependency]) {
        if (has_circular_dependency(name, dep)) {
            return true;
        }
    }
    return false;
}

void Registry::initialize_component(
    const std::string& name, std::unordered_set<std::string>& init_stack) {
    if (initialized[name]) {
        return;
    }
    if (init_stack.count(name)) {
        THROW_RUNTIME_ERROR(
            "Circular dependency detected while initializing: " + name);
    }
    init_stack.insert(name);
    for (const auto& dep : dependencies[name]) {
        initialize_component(dep, init_stack);
    }
    initializers[name].init_func();
    initialized[name] = true;
    init_stack.erase(name);
}
