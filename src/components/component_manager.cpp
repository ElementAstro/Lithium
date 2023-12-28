#include "component_manager.hpp"

#include "atom/server/global_ptr.hpp"

namespace Lithium
{
    ComponentManager::ComponentManager()
    {
        m_ModuleLoader = GetPtr<Atom::Module::ModuleLoader>("ModuleLoader");
    }

    ComponentManager::~ComponentManager()
    {

    }

    bool ComponentManager::Initialize()
    {
        return true;
    }

    bool ComponentManager::Destroy()
    {
        return true;
    }

    std::shared_ptr<ComponentManager> ComponentManager::createShared()
    {
        return std::make_shared<ComponentManager>();
    }

    std::unique_ptr<ComponentManager> ComponentManager::createUnique()
    {
        return std::make_unique<ComponentManager>();
    }

    bool ComponentManager::loadComponent(ComponentType component_type, const std::shared_ptr<ArgumentContainer> args)
    {

        return true;
    }

    bool ComponentManager::unloadComponent(ComponentType component_type, const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }

    bool ComponentManager::reloadComponent(ComponentType component_type, const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }

    bool ComponentManager::reloadAllComponents()
    {
        return true;
    }

    bool ComponentManager::reloadAllComponents(const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }

    std::shared_ptr<Component> ComponentManager::getComponent(ComponentType component_type, const std::string &component_name) const
    {
        return nullptr;
    }

    std::shared_ptr<Component> ComponentManager::getComponent(ComponentType component_type, const std::shared_ptr<ArgumentContainer> args) const
    {
        return nullptr;
    }

    bool ComponentManager::loadSharedComponent(const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }

    bool ComponentManager::unloadSharedComponent(const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }

    bool ComponentManager::reloadSharedComponent(const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }

    bool ComponentManager::loadScriptComponent(const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }

    bool ComponentManager::unloadScriptComponent(const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }

    bool ComponentManager::reloadScriptComponent(const std::shared_ptr<ArgumentContainer> args)
    {
        return true;
    }


} // namespace Lithium
