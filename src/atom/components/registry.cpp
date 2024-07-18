#include "registry.hpp"

#include <iostream>

void Registry::addInitializer(const std::string& name,
                              Component::InitFunc init_func,
                              Component::CleanupFunc cleanup_func) {
    std::scoped_lock lock(mutex_);
    if (initializers_.contains(name)) {
        return;
    }
    initializers_[name] = std::make_shared<Component>(name);
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
    determineInitializationOrder();
    for (const auto& name : initializationOrder_) {
        std::unordered_set<std::string> initStack;
        initializeComponent(name, initStack);
    }
}

void Registry::cleanupAll() {
    std::unique_lock lock(mutex_);
    for (const auto& name : std::ranges::reverse_view(initializationOrder_)) {
        if (initializers_[name]->cleanupFunc && initialized_[name]) {
            initializers_[name]->cleanupFunc();
            initialized_[name] = false;
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
    if (!initializers_.contains(name)) {
        THROW_OBJ_NOT_EXIST("Component not registered: " + name);
    }
    std::unordered_set<std::string> initStack;
    initializeComponent(name, initStack);
}

auto Registry::getComponent(const std::string& name) const
    -> std::shared_ptr<Component> {
    std::shared_lock lock(mutex_);
    if (!initializers_.contains(name)) {
        THROW_OBJ_NOT_EXIST("Component not registered: " + name);
    }
    return initializers_.at(name);
}

bool Registry::hasCircularDependency(const std::string& name,
                                     const std::string& dependency) {
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
        if (init_stack.contains(name)) {
            THROW_RUNTIME_ERROR(
                "Circular dependency detected while initializing component "
                "'{}'",
                name);
        }
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
    if (initializers_[name]->initFunc) {
        initializers_[name]->initFunc();
    }
    initialized_[name] = true;
    init_stack.erase(name);
}

void Registry::determineInitializationOrder() {
    std::unordered_set<std::string> visited;
    std::function<void(const std::string&)> visit =
        [&](const std::string& name) {
            if (!visited.contains(name)) {
                visited.insert(name);
                for (const auto& dep : dependencies_[name]) {
                    visit(dep);
                }
                initializationOrder_.push_back(name);
            }
        };
    for (const auto& pair : initializers_) {
        visit(pair.first);
    }
}