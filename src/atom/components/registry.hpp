/*
 * var.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Registry Pattern

**************************************************/

#ifndef ATOM_COMPONENT_REGISTRY_HPP
#define ATOM_COMPONENT_REGISTRY_HPP

#include <algorithm>  // for std::reverse
#include <memory>     // for std::shared_ptr
#include <memory>
#include <mutex>          // for std::scoped_lock
#include <ranges>         // for std::ranges
#include <shared_mutex>   // for std::shared_mutex
#include <string>         // for std::string
#include <unordered_map>  // for std::unordered_map
#include <unordered_set>  // for std::unordered_set
#include <utility>
#include <vector>  // for std::vector

#include "atom/error/exception.hpp"
#include "component.hpp"
#include "macro.hpp"

/**
 * @class Registry
 * @brief Manages initialization and cleanup of components in a registry
 * pattern.
 */
class Registry {
public:
    /**
     * @brief Gets the singleton instance of the Registry.
     * @return Reference to the singleton instance of the Registry.
     */
    static inline auto instance() -> Registry& {
        static Registry instance;
        return instance;
    }

    /**
     * @brief Registers a module's initialization function.
     * @param name The name of the module.
     * @param init_func The initialization function for the module.
     */
    void registerModule(const std::string& name, Component::InitFunc init_func);

    /**
     * @brief Adds an initializer function for a component to the registry.
     * @param name The name of the component.
     * @param init_func The initialization function for the component.
     * @param cleanup_func The cleanup function for the component (optional).
     */
    void addInitializer(const std::string& name, Component::InitFunc init_func,
                        Component::CleanupFunc cleanup_func = nullptr);

    /**
     * @brief Adds a dependency between two components.
     * @param name The name of the component.
     * @param dependency The name of the component's dependency.
     */
    void addDependency(const std::string& name, const std::string& dependency);

    /**
     * @brief Initializes all components in the registry.
     */
    void initializeAll();

    /**
     * @brief Cleans up all components in the registry.
     */
    void cleanupAll();

    /**
     * @brief Checks if a component is initialized.
     * @param name The name of the component to check.
     * @return True if the component is initialized, false otherwise.
     */
    auto isInitialized(const std::string& name) const -> bool;

    /**
     * @brief Reinitializes a component in the registry.
     * @param name The name of the component to reinitialize.
     */
    void reinitializeComponent(const std::string& name);

    auto getComponent(const std::string& name) const
        -> std::shared_ptr<Component>;

    auto getAllComponents() const -> std::vector<std::shared_ptr<Component>>;

    auto getAllComponentNames() const -> std::vector<std::string>;

private:
    /**
     * @brief Private constructor to prevent instantiation.
     */
    Registry() = default;

    std::unordered_map<std::string, std::shared_ptr<Component>>
        initializers_; /**< Map of component names to their initialization and
                          cleanup functions. */
    std::unordered_map<std::string, std::unordered_set<std::string>>
        dependencies_; /**< Map of component names to their dependencies. */
    std::unordered_map<std::string, bool>
        initialized_; /**< Map of component names to their initialization
                         status. */
    std::vector<std::string>
        initializationOrder_; /**< List of component names in initialization
                                 order. */
    std::unordered_map<std::string, Component::InitFunc>
        module_initializers_; /**< Map of module names to their initialization
                                 functions. */
    mutable std::shared_mutex
        mutex_; /**< Mutex for thread-safe access to the registry. */

    /**
     * @brief Checks if adding a dependency creates a circular dependency.
     * @param name The name of the component.
     * @param dependency The name of the dependency.
     * @return True if adding the dependency creates a circular dependency,
     * false otherwise.
     */
    bool hasCircularDependency(const std::string& name,
                               const std::string& dependency);

    /**
     * @brief Initializes a component and its dependencies recursively.
     * @param name The name of the component to initialize.
     * @param init_stack Stack to keep track of components being initialized to
     * detect circular dependencies.
     */
    void initializeComponent(const std::string& name,
                             std::unordered_set<std::string>& init_stack);

    /**
     * @brief Determines the order of initialization based on dependencies.
     */
    void determineInitializationOrder();
};

ATOM_INLINE void Registry::registerModule(const std::string& name,
                                          Component::InitFunc init_func) {
    std::scoped_lock lock(mutex_);
    LOG_F(INFO, "Registering module: {}", name);
    module_initializers_[name] = std::move(init_func);
}

ATOM_INLINE void Registry::addInitializer(const std::string& name,
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

ATOM_INLINE void Registry::addDependency(const std::string& name,
                                         const std::string& dependency) {
    std::unique_lock lock(mutex_);
    if (hasCircularDependency(name, dependency)) {
        THROW_RUNTIME_ERROR("Circular dependency detected: " + name + " -> " +
                            dependency);
    }
    dependencies_[name].insert(dependency);
}

ATOM_INLINE void Registry::initializeAll() {
    std::unique_lock lock(mutex_);
    LOG_F(INFO, "Initializing all components");
    determineInitializationOrder();
    for (const auto& name : initializationOrder_) {
        std::unordered_set<std::string> initStack;
        LOG_F(INFO, "Initializing component: {}", name);
        initializeComponent(name, initStack);
    }
}

ATOM_INLINE void Registry::cleanupAll() {
    std::unique_lock lock(mutex_);
    for (const auto& name : std::ranges::reverse_view(initializationOrder_)) {
        if (initializers_[name]->cleanupFunc && initialized_[name]) {
            initializers_[name]->cleanupFunc();
            initialized_[name] = false;
        }
    }
}

ATOM_INLINE auto Registry::isInitialized(const std::string& name) const
    -> bool {
    std::shared_lock lock(mutex_);
    auto it = initialized_.find(name);
    return it != initialized_.end() && it->second;
}

ATOM_INLINE void Registry::reinitializeComponent(const std::string& name) {
    std::scoped_lock lock(mutex_);
    if (initialized_[name]) {
        if (auto it = initializers_.find(name);
            it != initializers_.end() && it->second->cleanupFunc) {
            it->second->cleanupFunc();
        }
    }
    auto it = module_initializers_.find(name);
    if (it != module_initializers_.end()) {
        auto component = std::make_shared<Component>(name);
        it->second(*component);
        initializers_[name] = component;
        initialized_[name] = true;
    }
}

ATOM_INLINE auto Registry::getComponent(const std::string& name) const
    -> std::shared_ptr<Component> {
    std::shared_lock lock(mutex_);
    if (!initializers_.contains(name)) {
        THROW_OBJ_NOT_EXIST("Component not registered: " + name);
    }
    return initializers_.at(name);
}

ATOM_INLINE auto Registry::getAllComponents() const
    -> std::vector<std::shared_ptr<Component>> {
    std::shared_lock lock(mutex_);
    std::vector<std::shared_ptr<Component>> components;
    for (const auto& pair : initializers_) {
        if (pair.second) {
            components.push_back(pair.second);
        }
    }
    return components;
}

ATOM_INLINE auto Registry::getAllComponentNames() const
    -> std::vector<std::string> {
    std::shared_lock lock(mutex_);
    std::vector<std::string> names;
    names.reserve(initializers_.size());
    for (const auto& pair : initializers_) {
        names.push_back(pair.first);
    }
    return names;
}

ATOM_INLINE auto Registry::hasCircularDependency(
    const std::string& name, const std::string& dependency) -> bool {
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

ATOM_INLINE void Registry::initializeComponent(
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
        initializers_[name]->initFunc(*initializers_[name]);
    }
    initialized_[name] = true;
    init_stack.erase(name);
}

ATOM_INLINE void Registry::determineInitializationOrder() {
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

#endif  // ATOM_COMPONENT_REGISTRY_HPP
