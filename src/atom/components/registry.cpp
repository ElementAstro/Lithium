#include "registry.hpp"

#include <algorithm>
#include <mutex>
#include <vector>

#include "atom/error/exception.hpp"

void Registry::addInitializer(const std::string& name,
                              Component::InitFunc init_func,
                              Component::CleanupFunc cleanup_func) {
    std::scoped_lock lock(mutex_);
    if (initializers_.find(name) != initializers_.end()) {
        THROW_OBJ_ALREADY_INITIALIZED("Initializer already registered: " +
                                      name);
    }
    initializers_[name]->initFunc = std::move(init_func);
    initializers_[name]->cleanupFunc = std::move(cleanup_func);
    initialized_[name] = false;
}

void Registry::addDependency(const std::string& name,
                             const std::string& dependency) {
    std::unique_lock lock(mutex_);
    if (hasCircularDependency(name, dependency)) {
        THROW_RUNTIME_ERROR("Circular dependency detected: " + name + " -> " +
                            dependency);
    }
    dependencies_[name].insert(dependency);
}

void Registry::initializeAll() {
    std::unique_lock lock(mutex_);
    std::unordered_set<std::string> initStack;
    for (const auto& pair : initializers_) {
        initializeComponent(pair.first, initStack);
    }
}

void Registry::cleanupAll() {
    std::unique_lock lock(mutex_);
    std::vector<std::string> keys;
    keys.reserve(initializers_.size());
    for (const auto& pair : initializers_) {
        keys.push_back(pair.first);
    }
    std::reverse(keys.begin(), keys.end());

    for (const auto& key : keys) {
        if (initializers_[key]->cleanupFunc && initialized_[key]) {
            initializers_[key]->cleanupFunc();
            initialized_[key] = false;
        }
    }
}

bool Registry::isInitialized(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = initialized_.find(name);
    return it != initialized_.end() && it->second;
}

void Registry::reinitializeComponent(const std::string& name) {
    std::unique_lock lock(mutex_);
    if (initializers_.find(name) == initializers_.end()) {
        THROW_OBJ_NOT_EXIST("Component not registered: " + name);
    }
    std::unordered_set<std::string> initStack;
    initializeComponent(name, initStack);
}

auto Registry::hasCircularDependency(const std::string& name,
                                     const std::string& dependency) -> bool {
    if (dependencies_[dependency].contains(name)) {
        return true;
    }
    for (const auto& dep : dependencies_[dependency]) {
        if (hasCircularDependency(name, dep)) {
            return true;
        }
    }
    return false;
}

void Registry::initializeComponent(
    const std::string& name, std::unordered_set<std::string>& init_stack) {
    if (initialized_[name]) {
        return;
    }
    if (init_stack.contains(name)) {
        THROW_RUNTIME_ERROR(
            "Circular dependency detected while initializing: " + name);
    }
    init_stack.insert(name);
    for (const auto& dep : dependencies_[name]) {
        initializeComponent(dep, init_stack);
    }
    initializers_[name]->initFunc();
    initialized_[name] = true;
    init_stack.erase(name);
}
