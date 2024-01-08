/*
 * component_manager.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2024-1-4

Description: Component Manager (the core of the plugin system)

**************************************************/

#pragma once

#include "atom/components/component.hpp"
#include "atom/components/templates/shared_component.hpp"
#include "atom/components/templates/exe_component.hpp"
#include "atom/components/templates/alone_component.hpp"
#include "atom/components/types.hpp"

#include "atom/module/module_loader.hpp"

#include "atom/type/args.hpp"

// headers of the components that are provided by the plugin system
#include "component_info.hpp"
#include "component_finder.hpp"

// headers about the package system and the project system
// note: we will use the package system to manage the plugins
#include "package_manager.hpp"
#include "project_info.hpp"
#include "project_manager.hpp"

// headers about the sandbox system, specifically for the alone components
#include "sandbox.hpp"

namespace Lithium
{
    class ComponentEntry
    {
    public:
        std::string m_name;
        std::string m_func_name;
        std::string m_component_type;
        std::string m_module_name;

        std::vector<std::string> m_dependencies;

        ComponentEntry(const std::string &name, const std::string &func_name, const std::string &component_type, const std::string &module_name)
            : m_name(name), m_func_name(func_name), m_component_type(component_type), m_module_name(module_name) {}
    };

    class ComponentManager
    {
    public:
        explicit ComponentManager();
        ~ComponentManager();

        // -------------------------------------------------------------------
        // Common methods
        // -------------------------------------------------------------------

        /**
         * @brief Initializes the component manager
         * @return true if the component manager was initialized successfully, false otherwise
         * @note We will try to load all of the components in the components directory.
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
         */
        bool Initialize();

        /**
         * @brief Destroys the component manager
         * @return true if the component manager was destroyed successfully, false otherwise
         */
        bool Destroy();

        /**
         * @brief Creates a shared pointer to the component manager
         * @return A shared pointer to the component manager
         */
        std::shared_ptr<ComponentManager> createShared();

        /**
         * @brief Creates a unique pointer to the component manager
         * @return A unique pointer to the component manager
         */
        std::unique_ptr<ComponentManager> createUnique();

        // -------------------------------------------------------------------
        // Components methods (main entry)
        // -------------------------------------------------------------------

        /*
            Max: Though we provide the ways to load and unload components,
                 we will only used load method in the release mode.
                 It seems that the logic of loading and unloading components is not as simple as we thought.
                 Only the developer can use unload and reload methods for debugging.

                 Also, the module means the dynamic library, which is a kind of shared library.
                 The components means the shared_ptr involved by the module.
                 So do not confuse the module and the component.
        */

        /**
         * @brief Load a component
         * @param component_type The type of the component to load
         * @param args The arguments to pass to the component
         * @return true if the component was loaded successfully, false otherwise
         * @note The component will be loaded in the main thread
         */
        bool loadComponent(ComponentType component_type, const std::shared_ptr<Args> args);

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
        bool unloadComponent(ComponentType component_type, const std::shared_ptr<Args> args);

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
        bool reloadComponent(ComponentType component_type, const std::shared_ptr<Args> args);

        /**
         * @brief Reload all components
         * @return true if the components were reloaded successfully, false otherwise
         * @note The components will be reloaded in the main thread
         * @note This method is not supposed to be called in release mode
         * @warning This method will alse reload the components if they are still in use
         * @warning Also, will cause Segmentation fault
         */
        bool reloadAllComponents();

        /**
         * @brief Reload all components
         * @param args The arguments to pass to the components
         * @return true if the components were reloaded successfully, false otherwise
         * @note The components will be reloaded in the main thread
         * @note This method is not supposed to be called in release mode
         * @warning This method will alse reload the components if they are still in use
         * @warning Also, will cause Segmentation fault
         */
        bool reloadAllComponents(const std::shared_ptr<Args> args);

        // -------------------------------------------------------------------
        // Components methods (getters)
        // -------------------------------------------------------------------

        /**
         * @brief Get a component
         * @param component_type The type of the component to get
         * @param component_name The name of the component to get
         * @return The component if it exists, nullptr otherwise
         */
        std::shared_ptr<Component> getComponent(ComponentType component_type, const std::string &component_name) const;

        /**
         * @brief Get a component
         * @param component_type The type of the component to get
         * @param args The arguments to pass to the component
         * @return The component if it exists, nullptr otherwise
         */
        std::shared_ptr<Component> getComponent(ComponentType component_type, const std::shared_ptr<Args> args) const;

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
        bool checkComponent(const std::string &module_path, const std::string &module_name);

        /**
         * @brief Load the component info
         * @param module_path The path of the module
         * @return true if the component info was loaded successfully, false otherwise
         * @note This method is used to load the component info.
         *       Just to check if the component info was loaded, if not we will load it
         */
        bool loadComponentInfo(const std::string &module_path);

        /**
         * @brief Check if a component info is loaded
         * @param module_name The name of the module
         * @param component_name The name of the component
         * @return true if the component info is loaded, false otherwise
         * @note We will check if the component info is loaded, if not we will load it
         */
        bool checkComponentInfo(const std::string &module_name, const std::string &component_name);

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
        bool loadSharedComponent(const std::string &component_name);
        bool unloadSharedComponent(const std::shared_ptr<Args> args);
        bool reloadSharedComponent(const std::shared_ptr<Args> args);

        // -------------------------------------------------------------------
        // Components methods (for alone components)
        // -------------------------------------------------------------------

        bool loadAloneComponent(const std::shared_ptr<Args> args);
        bool unloadAloneComponent(const std::shared_ptr<Args> args);
        bool reloadAloneComponent(const std::shared_ptr<Args> args);

        // -------------------------------------------------------------------
        // Components methods (for script components)
        // -------------------------------------------------------------------

        bool loadScriptComponent(const std::shared_ptr<Args> args);
        bool unloadScriptComponent(const std::shared_ptr<Args> args);
        bool reloadScriptComponent(const std::shared_ptr<Args> args);

    private:
        std::shared_ptr<Atom::Module::ModuleLoader> m_ModuleLoader;

        std::shared_ptr<ComponentFinder> m_ComponentFinder;

        // This is used to solve the circular dependency problem
        // And make sure we can unload the components in the correct order
        std::shared_ptr<PackageManager> m_PackageManager;
        std::shared_ptr<ProjectManager> m_ProjectManager;

        std::unordered_map<std::string, std::shared_ptr<ComponentInfo>> m_ComponentInfos;
        std::unordered_map<std::string, std::shared_ptr<ComponentEntry>> m_ComponentEntries;

        // Components map of different types
        // Max: Why not just use a single map of std::shared_ptr<Component>?
        //      Maybe it is because the dynamic_cast will be slow
        //      And we are surely about what the component is
        std::unordered_map<std::string, std::shared_ptr<SharedComponent>> m_SharedComponents;
        std::unordered_map<std::string, std::shared_ptr<AloneComponent>> m_AloneComponents;
        std::unordered_map<std::string, std::shared_ptr<ExecutableComponent>> m_ExecutableComponents;
    };
} // namespace Lithium
