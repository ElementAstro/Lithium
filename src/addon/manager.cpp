/*
 * component_manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

#ifdef _WIN32
constexpr std::string PATH_SEPARATOR = '\\';
constexpr std::string DYNAMIC_LIBRARY_EXTENSION = ".dll";
#else
constexpr std::string PATH_SEPARATOR = '/';
constexpr std::string DYNAMIC_LIBRARY_EXTENSION = ".so";
#endif

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
        m_ModuleLoader = GetPtr<Atom::Module::ModuleLoader>("lithium.addon.loader");
        m_Env = GetPtr<Atom::Utils::Env>("lithium.utils.env");

        m_ComponentFinder = std::make_unique<AddonFinder>();
        m_Sandbox = std::make_unique<Sandbox>();
        m_Compiler = std::make_unique<Compiler>();
    }

    ComponentManager::~ComponentManager()
    {
    }

    bool ComponentManager::Initialize()
    {
        if (!m_ModuleLoader)
        {
            LOG_F(ERROR, _("Failed to load component manager: {}"), "lithium.addon.loader");
            return false;
        }
        // Check if the module path is valid or reset by the user
        // Default path is ./modules
        const std::string &module_path = m_Env->getEnv("LITHIUM_ADDON_PATH", "./modules");
        // Get all of the available addon path
        if (!m_ComponentFinder->traverseDir(std::filesystem::path(module_path)))
        {
            LOG_F(ERROR, _("Failed to traversing module path"));
            return false;
        }
        for (const auto &dir : m_ComponentFinder->getAvailableDirs())
        {
            std::filesystem::path path = module_path / dir;

            if (!m_AddonManager->addModule(path, dir))
            {
                LOG_F(ERROR, _("Failed to load module: {}"), path.string());
                continue;
            }
            const json &addon_info = m_AddonManager->getModule(dir);

            for (const auto &module_name : addon_info["modules"].get<std::vector<std::string>>())
            {
                std::filesystem::path module_path = path / module_name;
                if (!m_ModuleLoader->loadModule(module_path.string(), module_name))
                {
                    LOG_F(ERROR, _("Failed to load module: {}/{}"), path.string(), module_name);
                    continue;
                }
                DLOG_F(INFO, _("Loaded module: {}/{}"), path.string(), module_name);
            }
        }
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

        if (!checkComponent(module_name, module_path))
        {
            DVLOG_F(ERROR, _("Failed to load component library: {}"), component_library);
            return false;
        }
        if (!loadComponentInfo(module_path))
        {
            DVLOG_F(ERROR, _("Failed to load component info: {}"), component_library);
            return false;
        }
        if (!checkComponentInfo(module_name, component_name))
        {
            DVLOG_F(ERROR, _("Failed to load component info: {}"), component_library);
            return false;
        }

        auto it = m_ComponentEntries.find(module_name + "." + component_name);
        if (it == m_ComponentEntries.end())
        {
            LOG_F(ERROR, _("Failed to load component entry: {}"), component_name);
            return false;
        }
        if (it->second->m_component_type == "shared")
        {
            if (!loadSharedComponent(component_name))
            {
                DVLOG_F(ERROR, _("Failed to load shared component: {}"), component_name);
                return false;
            }
        }
        else if (it->second->m_component_type == "alone")
        {
        }
        else if (it->second->m_component_type == "executable")
        {
        }
        return true;
    }

    bool ComponentManager::checkComponent(const std::string &module_name, const std::string &module_path)
    {
        DLOG_SCOPE_FUNCTION(INFO);
        // Check if the module has been loaded
        if (m_ModuleLoader->HasModule(module_name))
        {
            LOG_F(WARNING, _("Module {} has been loaded, please do not load again"), module_name);
            return true;
        }
        // If not, load the module
        // Check component path
        if (!Atom::IO::isFolderExists(module_path))
        {
            LOG_F(ERROR, _("Component path {} does not exist"), module_path);
            return false;
        }
        // Check component package.json file, this is for the first time loading
        // And we need to know how to load component's ptr from this file
        if (!Atom::IO::isFileExists(module_path + PATH_SEPARATOR + "package.json"))
        {
            LOG_F(ERROR, _("Component path {} does not contain package.json"), module_path);
            return false;
        }
        // Check component library files
        std::vector<std::string> files = Atom::IO::checkFileTypeInFolder(module_path, DYNAMIC_LIBRARY_EXTENSION, Atom::IO::fileType::Name);

        if (files.empty())
        {
            LOG_F(ERROR, _("Component path {} does not contain dll or so file"), module_path);
            return false;
        }
        auto it = std::find(files.begin(), files.end(), module_name + DYNAMIC_LIBRARY_EXTENSION);
        if (it != files.end())
        {
            if (!m_ModuleLoader->loadModule(module_path + PATH_SEPARATOR + module_name + DYNAMIC_LIBRARY_EXTENSION, module_name))
            {
                LOG_F(ERROR, _("Failed to load module: {}'s library {}"), component_name, component_library);
                return false;
            }
        }
        else
        {
            LOG_F(ERROR, _("Component path {} does not contain specified dll or so file"), module_path);
            return false;
        }
        return true;
    }

    bool ComponentManager::loadComponentInfo(const std::string &module_path)
    {
        DLOG_SCOPE_FUNCTION(INFO);
        // Load the Package.json
        // Max: We will only load the root package.json
        std::shared_ptr<ComponentInfo> component_info = std::make_shared<ComponentInfo>(module_path + PATH_SEPARATOR + "package.json");
        try
        {
            component_info->loadPackageJson();
            if (!component_info->isLoaded())
            {
                LOG_F(ERROR, _("Failed to load package.json file: {}"), module_path);
                return false;
            }
            DLOG_F(INFO, _("Successfully loaded package.json file: {}"), module_path);
        }
        catch (const Atom::Utils::Exception::FileNotReadable_Error &e)
        {
            LOG_F(ERROR, _("Failed to load package.json file: {}"), e.what());
            return false;
        }
    }

    bool ComponentManager::checkComponentInfo(const std::string &module_name, const std::string &component_name)
    {
        DLOG_SCOPE_FUNCTION(INFO);
        auto it = m_ComponentInfos.find(module_name);
        if (it == m_ComponentInfos.end())
        {
            LOG_F(ERROR, _("Component {} does not contain package.json file"), module_name);
            return false;
        }
        auto component_info = m_ComponentInfos[module_name];
        // Then we will parse the package.json file and get what we need for the component loading
        _ComponentInfo m = component_info->toStruct();
        if (m.main.find(component_name) == m.main.end())
        {
            LOG_F(ERROR, _("Could not found the main entry to load {}"), component_name);
            return false;
        }
        // Check the main function
        _ComponentMain main = m.main[component_name];
        if (main.m_func_name.empty())
        {
            LOG_F(ERROR, _("Component {} does not contain main function"), component_name);
            return false;
        }
        // Check if the function exists in the module
        if (!m_ModuleLoader->HasFunction(module_name, main.m_func_name))
        {
            LOG_F(ERROR, _("Failed to load module: {}'s function {}"), component_name, main.m_func_name);
            return false;
        }
        std::shared_ptr<ComponentEntry> entry = std::make_shared<ComponentEntry>(main.m_component_name, main.m_func_name, main.m_component_type, module_name);
        // Check the dependencies
        if (!m.dependencies.empty())
        {
            entry->m_dependencies = m.dependencies;
            for (auto &dependency : m.dependencies)
            {
                auto it = m_ComponentEntries.find(dependency);
                if (it == m_ComponentEntries.end())
                {
                    LOG_F(ERROR, _("Component {}'s dependency {} does not exist"), component_name, dependency);
                    return false;
                }
            }
            DLOG_F(INFO, _("Component {} dependencies check passed"), component_name);
        }
        m_ComponentEntries[module_name + "." + component_name] = entry;
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

    bool ComponentManager::loadSharedComponent(const std::string &component_name)
    {
        DLOG_SCOPE_FUNCTION(INFO);
        // Classis parameter check
        if (component_name.empty())
        {
            LOG_F(ERROR, _("Component name is empty"));
            return false;
        }
        if (m_SharedComponents.find(component_name) != m_SharedComponents.end())
        {
            LOG_F(ERROR, _("Component {} has been loaded"), component_name);
            return false;
        }
        // Load the component entry from the component entries
        auto it = m_ComponentEntries.find(component_name);
        if (it == m_ComponentEntries.end())
        {
            LOG_F(ERROR, _("Component {} does not has an entry"), component_name);
            return false;
        }
        // There we need some json parameters support for better get the component instance
        if (std::shared_ptr<SharedComponent> component = m_ModuleLoader->GetInstance<Component>(it->second->m_name, {}, it->second->m_func_name); component)
        {
            try
            {
                // Initialize the component
                component->Initialize();
            }
            catch (const std::exception &e)
            {
                LOG_F(ERROR, _("Failed to initialize component: {}"), e.what());
                return false;
            }
            m_SharedComponents[component_name] = component;
        }
        else
        {
            LOG_F(ERROR, _("Failed to load module: {}'s library {}"), component_name, component_func);
            return false;
        }
        DLOG_F(INFO, _("Successfully loaded shared component: {}"), component_name);
        return true;
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
