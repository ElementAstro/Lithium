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

#include <memory>         // for std::shared_ptr
#include <shared_mutex>   // for std::shared_mutex
#include <string>         // for std::string
#include <unordered_map>  // for std::unordered_map
#include <unordered_set>  // for std::unordered_set

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
    static inline auto instance() -> Registry& {
        static Registry instance;
        return instance;
    }

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

    auto getComponent(const std::string& name) const -> std::shared_ptr<Component>;

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
                       * status.
                       */
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
};

#endif  // ATOM_COMPONENT_REGISTRY_HPP
