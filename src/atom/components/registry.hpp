/*
 * registry.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Registry Pattern

**************************************************/

#ifndef ATOM_COMPONENT_REGISTRY_HPP
#define ATOM_COMPONENT_REGISTRY_HPP

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "component.hpp"

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
    static auto instance() -> Registry&;

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

    /**
     * @brief Gets a component by name.
     * @param name The name of the component.
     * @return Shared pointer to the component.
     */
    auto getComponent(const std::string& name) const
        -> std::shared_ptr<Component>;

    /**
     * @brief Gets all components.
     * @return Vector of shared pointers to all components.
     */
    auto getAllComponents() const -> std::vector<std::shared_ptr<Component>>;

    /**
     * @brief Gets the names of all components.
     * @return Vector of all component names.
     */
    auto getAllComponentNames() const -> std::vector<std::string>;

    /**
     * @brief Removes a component from the registry.
     * @param name The name of the component to remove.
     */
    void removeComponent(const std::string& name);

private:
    /**
     * @brief Private constructor to prevent instantiation.
     */
    Registry() = default;

    std::unordered_map<std::string, std::shared_ptr<Component>> initializers_;
    std::unordered_map<std::string, std::unordered_set<std::string>>
        dependencies_;
    std::unordered_map<std::string, bool> initialized_;
    std::vector<std::string> initializationOrder_;
    std::unordered_map<std::string, Component::InitFunc> module_initializers_;
    mutable std::shared_mutex mutex_;

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

#endif  // ATOM_COMPONENT_REGISTRY_HPP