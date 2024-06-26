/*
 * component_manager.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Component Manager (the core of the plugin system)

**************************************************/

#pragma once

#include "atom/components/component.hpp"
#include "atom/components/types.hpp"

#include "atom/type/args.hpp"
#include "atom/system/env.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {
class AddonManager;
class Compiler;
class ModuleLoader;
class Sandbox;

class ComponentEntry {
public:
    std::string m_name;
    std::string m_func_name;
    std::string m_component_type;
    std::string m_module_name;

    std::vector<std::string> m_dependencies;

    ComponentEntry(const std::string& name, const std::string& func_name,
                   const std::string& component_type,
                   const std::string& module_name)
        : m_name(name),
          m_func_name(func_name),
          m_component_type(component_type),
          m_module_name(module_name) {}
};

class ComponentManager {
public:
    explicit ComponentManager();
    ~ComponentManager();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    /**
     * @brief Initializes the component manager
     * @return true if the component manager was initialized successfully, false
     * otherwise
     * @note We will try to load all of the components in the components
     * directory.
     * @note And the directory structure should be like this:
     *     - components / modules
     *         - component1
     *             - package.json
     *             - component.dll
     *         - component2
     *             - package.json
     *             - component.dll
     *         -...
     * @warning This method will not load the components in the debug mode.
     * @note The `addon` is the highest level of the component system.
     */
    bool Initialize();

    /**
     * @brief Destroys the component manager
     * @return true if the component manager was destroyed successfully, false
     * otherwise
     */
    bool Destroy();

    /**
     * @brief Creates a shared pointer to the component manager
     * @return A shared pointer to the component manager
     */
    static std::shared_ptr<ComponentManager> createShared();

    // -------------------------------------------------------------------
    // Components methods (main entry)
    // -------------------------------------------------------------------

    /*
        Max: Though we provide the ways to load and unload components,
             we will only used load method in the release mode.
             It seems that the logic of loading and unloading components is not
       as simple as we thought. Only the developer can use unload and reload
       methods for debugging.

             Also, the module means the dynamic library, which is a kind of
       shared library. The components means the shared_ptr involved by the
       module. So do not confuse the module and the component.
    */

    /**
     * @brief Load a component
     * @param component_type The type of the component to load
     * @param args The arguments to pass to the component
     * @return true if the component was loaded successfully, false otherwise
     * @note The component will be loaded in the main thread
     */
    bool loadComponent(ComponentType component_type, const json& params);

    /**
     * @brief Unload a component
     * @param component_type The type of the component to unload
     * @param args The arguments to pass to the component
     * @return true if the component was unloaded successfully, false otherwise
     * @note The component will be unloaded in the main thread
     * @note This method is not supposed to be called in release mode
     * @warning This method will alse unload the component if it is still in use
     * @warning Also, will cause Segmentation fault
     */
    bool unloadComponent(ComponentType component_type, const json& params);

    /**
     * @brief Reload a component
     * @param component_type The type of the component to reload
     * @param args The arguments to pass to the component
     * @return true if the component was reloaded successfully, false otherwise
     * @note The component will be reloaded in the main thread
     * @note This method is not supposed to be called in release mode
     * @warning This method will alse reload the component if it is still in use
     * @warning Also, will cause Segmentation fault
     */
    bool reloadComponent(ComponentType component_type, const json& params);

    /**
     * @brief Reload all components
     * @return true if the components were reloaded successfully, false
     * otherwise
     * @note The components will be reloaded in the main thread
     * @note This method is not supposed to be called in release mode
     * @warning This method will alse reload the components if they are still in
     * use
     * @warning Also, will cause Segmentation fault
     */
    bool reloadAllComponents();

    // -------------------------------------------------------------------
    // Components methods (getters)
    // -------------------------------------------------------------------

    /**
     * @brief Get a component
     * @param component_type The type of the component to get
     * @param component_name The name of the component to get
     * @return The component if it exists, nullptr otherwise
     */
    std::optional<std::weak_ptr<Component>> getComponent(
        const std::string& component_name);

    std::optional<json> getComponentInfo(const std::string& component_name);

    std::vector<std::string> getComponentList();

    // -------------------------------------------------------------------
    // Load Components Steppers methods
    // -------------------------------------------------------------------

    /**
     * @brief Check if a component is loaded
     * @param module_path The path of the module
     * @param module_name The name of the module
     * @return true if the component is loaded, false otherwise
     * @note We will check if the module is loaded, if not we will load it
     */
    bool checkComponent(const std::string& module_path,
                        const std::string& module_name);

    /**
     * @brief Load the component info
     * @param module_path The path of the module
     * @return true if the component info was loaded successfully, false
     * otherwise
     * @note This method is used to load the component info.
     *       Just to check if the component info was loaded, if not we will load
     * it
     */
    bool loadComponentInfo(const std::string& module_path,
                           const std::string& name);

    /**
     * @brief Check if a component info is loaded
     * @param module_name The name of the module
     * @param component_name The name of the component
     * @return true if the component info is loaded, false otherwise
     * @note We will check if the component info is loaded, if not we will load
     * it
     */
    bool checkComponentInfo(const std::string& module_name,
                            const std::string& component_name);

    // -------------------------------------------------------------------
    // Components methods (for shared components)
    // -------------------------------------------------------------------

    /**
     * @brief Load a shared component
     * @param component_name The name of the component to load
     * @return true if the component was loaded successfully, false otherwise
     * @note The component will be loaded in the main thread
     * @note This method is not supposed to be called in release mode
     * @warning This method will alse load the component if it is still in use
     * @warning Also, will cause Segmentation fault
     */
    bool loadSharedComponent(const std::string& component_name,
                             const std::string& addon_name,
                             const std::string& module_path,
                             const std::string& entry,
                             const std::vector<std::string>& dependencies);
    bool unloadSharedComponent(const std::string& component_name, bool forced);
    bool reloadSharedComponent(const std::string& component_name);

private:
    /**
     * @brief Get all files in a directory
     * @param path The path of the directory
     * @return The files in the directory
     */
    std::vector<std::string> getFilesInDir(const std::string& path);

    /**
     * @brief Get all sub directories in a directory
     * @param path The path of the directory
     * @return The sub directories in the directory
     */
    std::vector<std::string> getQualifiedSubDirs(const std::string& path);

private:
    std::weak_ptr<ModuleLoader> m_ModuleLoader;
    std::weak_ptr<atom::utils::Env> m_Env;

    // The finder used to find the components
    // std::unique_ptr<AddonFinder> m_ComponentFinder;

    // The sandbox used to run the components in a safe way
    std::unique_ptr<Sandbox> m_Sandbox;

    // The compiler used to compile the shared components just in time
    std::unique_ptr<Compiler> m_Compiler;

    // This is used to solve the circular dependency problem
    // And make sure we can unload the components in the correct order
    std::weak_ptr<AddonManager> m_AddonManager;

    std::unordered_map<std::string, std::shared_ptr<ComponentEntry>>
        m_ComponentEntries;
    std::unordered_map<std::string, json> m_ComponentInfos;

    // Components map of different types
    // Max: Why not just use a single map of std::shared_ptr<Component>?
    //      Maybe it is because the dynamic_cast will be slow
    //      And we are surely about what the component is
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::weak_ptr<Component>> m_Components;
#else
    std::unordered_map<std::string, std::weak_ptr<Component>> m_Components;
#endif

    std::string m_module_path;
};
}  // namespace lithium
