#pragma once

#include "atom/components/component.hpp"
#include "atom/components/types.hpp"

#include "atom/module/module_loader.hpp"

#include "atom/type/args.hpp"

#include "component_info.hpp"

namespace Lithium
{
    class ComponentManager
    {
    public:
        explicit ComponentManager();
        ~ComponentManager();

        // -------------------------------------------------------------------
        // Common methods
        // -------------------------------------------------------------------

        bool Initialize();
        bool Destroy();

        std::shared_ptr<ComponentManager> createShared();
        std::unique_ptr<ComponentManager> createUnique();

        // -------------------------------------------------------------------
        // Components methods (main entry)
        // -------------------------------------------------------------------

        /*
            Max: Though we provide the ways to load and unload components,
                 we will only used load method in the release mode.
                 It seems that the logic of loading and unloading components is not as simple as we thought.
                 Only the developer can use unload and reload methods for debugging.
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
        // Components methods (for shared components)
        // -------------------------------------------------------------------

        bool loadSharedComponent(const std::shared_ptr<Args> args);
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

        std::unordered_map<ComponentType, std::unordered_map<std::string, std::shared_ptr<ComponentInfo>>> m_ComponentInfos;
    };
} // namespace Lithium
