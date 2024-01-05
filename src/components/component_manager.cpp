/*
 * component_manager.cpp
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

#include "component_manager.hpp"

#include "atom/server/global_ptr.hpp"
#include "atom/log/loguru.hpp"

#include "atom/io/io.hpp"

#include <memory>

#define IS_ARGUMENT_EMPTY() \
    if (args->size() < 1)   \
    {                       \
        return false;       \
    }

#define GET_ARGUMENT_C(name, type)                                     \
    std::optional<type> name = args[#name];                            \
    if (!name)                                                         \
    {                                                                  \
        LOG_F(ERROR, _("{}: Missing arguments: {}"), __func__, #name); \
        return false;                                                  \
    }

namespace Lithium
{
    ComponentManager::ComponentManager()
    {
        m_ModuleLoader = GetPtr<Atom::Module::ModuleLoader>("ModuleLoader");

        m_ComponentFinder = std::make_shared<ComponentFinder>();
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

    bool ComponentManager::loadComponent(ComponentType component_type, const std::shared_ptr<Args> args)
    {
        DLOG_SCOPE_FUNCTION(INFO);
        IS_ARGUMENT_EMPTY();
        // Args:
        // module_name: the name of the module(.dll or.so)
        // module_path: the path of the module(.dll or.so)
        // component_name: the name of the component, the name of the ptr will be gotten from the module
        // Others will be load from the package.json
        GET_ARGUMENT_C(module_name, std::string);
        GET_ARGUMENT_C(module_path, std::string);
        GET_ARGUMENT_C(component_name, std::string);
        // Check component path
        if (!Atom::IO::isFolderExists(module_path))
        {
            LOG_F(ERROR, _("Component path {} does not exist"), component_path);
            return false;
        }
        // Check component package.json file, this is for the first time loading
        // And we need to know how to load component's ptr from this file
#ifdef _WIN32
        if (!Atom::IO::isFileExists(component_path + "\\package.json"))
#else
        if (!Atom::IO::isFileExists(component_path + "/package.json"))
#endif
        {
            LOG_F(ERROR, _("Component path {} does not contain package.json"), component_path);
            return false;
        }
        // Load the Package.json
        std::shared_ptr<ComponentInfo> component_info = std::make_shared<ComponentInfo>(component_path + "/package.json");
        try
        {
            component_info->loadPackageJson();
            if (!component_info->isLoaded())
            {
                LOG_F(ERROR, _("Failed to load package.json file: {}"), component_path);
                return false;
            }
            DLOG_F(INFO, _("Successfully loaded package.json file: {}"), component_path);
            _ComponentInfo m = component_info->toStruct();
            if (m.m_type != "shared")
            {
                LOG_F(ERROR, _("Component {} is not a shared component"), component_name);
                return false;
            }
            if (m.main.find(component_name) == m.main.end())
            {
                LOG_F(ERROR, _("Component {} does not contain main function"), component_name);
                return false;
            }
            _ComponentMain main = m.main[component_name];
            if (main.m_func_name.empty())
            {
                LOG_F(ERROR, _("Component {} does not contain main function"), component_name);
                return false;
            }
            if (!m_ModuleLoader->loadModule(component_library, component_path))
            {
                LOG_F(ERROR, _("Failed to load module: {}'s library {}"), component_name, component_library);
                return false;
            }
            if (!m_ModuleLoader->HasFunction(component_name, main.m_func_name))
            {
                LOG_F(ERROR, _("Failed to load module: {}'s function {}"), component_name, main.m_func_name);
                return false;
            }
        }
    }
    catch (const Atom::Utils::Exception::FileNotReadable_Error &e)
    {
        LOG_F(ERROR, _("Failed to load package.json file: {}"), e.what());
        return false;
    }

    DLOG_F(INFO, _("Successfully loaded component library: {}"), component_library);

    return true;
    if (component_type == ComponentType::Shared)
    {
        return loadSharedComponent(args);
    }
    else if (commponent_type == ComponentType::Alone)
    {
        return loadAloneComponent(args);
    }
    return true;
}

bool ComponentManager::unloadComponent(ComponentType component_type, const std::shared_ptr<Args> args)
{
    return true;
}

bool ComponentManager::reloadComponent(ComponentType component_type, const std::shared_ptr<Args> args)
{
    return true;
}

bool ComponentManager::reloadAllComponents()
{
    return true;
}

bool ComponentManager::reloadAllComponents(const std::shared_ptr<Args> args)
{
    return true;
}

std::shared_ptr<Component> ComponentManager::getComponent(ComponentType component_type, const std::string &component_name) const
{
    return nullptr;
}

std::shared_ptr<Component> ComponentManager::getComponent(ComponentType component_type, const std::shared_ptr<Args> args) const
{
    return nullptr;
}

bool ComponentManager::loadSharedComponent(const std::shared_ptr<Args> args)
{
}

bool ComponentManager::unloadSharedComponent(const std::shared_ptr<Args> args)
{
    if (args->size() < 1)
    {
        return false;
    }
    GET_ARGUMENT_C(component_name, std::string);
    if (!m_ModuleLoader->unloadModule(component_name))
    {
        LOG_F(ERROR, _("Failed to unload module: {}"), component_name);
        return false;
    }
    DLOG_F(INFO, _("Successfully unloaded module: {}"), component_name);
    return true;
}

bool ComponentManager::reloadSharedComponent(const std::shared_ptr<Args> args)
{
    return true;
}

bool ComponentManager::loadScriptComponent(const std::shared_ptr<Args> args)
{
    return true;
}

bool ComponentManager::unloadScriptComponent(const std::shared_ptr<Args> args)
{
    return true;
}

bool ComponentManager::reloadScriptComponent(const std::shared_ptr<Args> args)
{
    return true;
}

} // namespace Lithium
