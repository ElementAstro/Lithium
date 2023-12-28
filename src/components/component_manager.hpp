#pragma once

#include "atom/components/component.hpp"
#include "atom/components/types.hpp"

#include "atom/module/module_loader.hpp"

#include "atom/type/args.hpp"

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

        bool loadComponent(ComponentType component_type, const std::shared_ptr<ArgumentContainer> args);
        bool unloadComponent(ComponentType component_type, const std::shared_ptr<ArgumentContainer> args);
        bool reloadComponent(ComponentType component_type, const std::shared_ptr<ArgumentContainer> args);
        bool reloadAllComponents();
        bool reloadAllComponents(const std::shared_ptr<ArgumentContainer> args);

        std::shared_ptr<Component> getComponent(ComponentType component_type, const std::string &component_name) const;
        std::shared_ptr<Component> getComponent(ComponentType component_type, const std::shared_ptr<ArgumentContainer> args) const;

        // -------------------------------------------------------------------
        // Components methods (for shared components)
        // -------------------------------------------------------------------

        bool loadSharedComponent(const std::shared_ptr<ArgumentContainer> args);
        bool unloadSharedComponent(const std::shared_ptr<ArgumentContainer> args);
        bool reloadSharedComponent(const std::shared_ptr<ArgumentContainer> args);

        // -------------------------------------------------------------------
        // Components methods (for script components)
        // -------------------------------------------------------------------

        bool loadScriptComponent(const std::shared_ptr<ArgumentContainer> args);
        bool unloadScriptComponent(const std::shared_ptr<ArgumentContainer> args);
        bool reloadScriptComponent(const std::shared_ptr<ArgumentContainer> args);

    private:

        

        std::shared_ptr<Atom::Module::ModuleLoader> m_ModuleLoader;
        
    };
} // namespace Lithium
