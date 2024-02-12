/*
 * component_manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Component Manager (the core of the plugin system)

**************************************************/

#include "manager.hpp"

#include "atom/server/global_ptr.hpp"

#include "atom/log/loguru.hpp"

#include "atom/io/io.hpp"

#include <memory>

#ifdef _WIN32
constexpr std::string PATH_SEPARATOR = "\\";
constexpr std::string DYNAMIC_LIBRARY_EXTENSION = ".dll";
#else
constexpr std::string PATH_SEPARATOR = "/";
constexpr std::string DYNAMIC_LIBRARY_EXTENSION = ".so";
#endif

#define IS_ARGUMENT_EMPTY() \
    if (params.is_null())   \
    {                       \
        return false;       \
    }

#define GET_ARGUMENT_C(name, type)                                  \
    if (!params.contains(#name))                                    \
    {                                                               \
        LOG_F(ERROR, "{}: Missing arguments: {}", __func__, #name); \
        return false;                                               \
    }                                                               \
    type name = params[#name].get<type>();

namespace Lithium
{
    ComponentManager::ComponentManager() : m_ModuleLoader(nullptr), m_Env(nullptr), m_ComponentFinder(nullptr), m_Sandbox(nullptr), m_Compiler(nullptr)
    {
        m_ModuleLoader = GetPtr<Lithium::ModuleLoader>("lithium.addon.loader");
        m_Env = GetPtr<Atom::Utils::Env>("lithium.utils.env");

        m_ComponentFinder = std::make_unique<AddonFinder>(m_Env->getEnv("LITHIUM_ADDON_PATH", "./modules"));
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
            LOG_F(ERROR, "Failed to load component manager: {}", "lithium.addon.loader");
            return false;
        }
        // Check if the module path is valid or reset by the user
        // Default path is ./modules
        const std::string &module_path = m_Env->getEnv("LITHIUM_ADDON_PATH", "./modules");
        // Get all of the available addon path
        if (!m_ComponentFinder->traverseDir(std::filesystem::path(module_path)))
        {
            LOG_F(ERROR, "Failed to traversing module path");
            return false;
        }
        for (const auto &dir : m_ComponentFinder->getAvailableDirs())
        {
            std::filesystem::path path = std::filesystem::path(module_path) / dir;

            if (!m_AddonManager->addModule(path, dir))
            {
                LOG_F(ERROR, "Failed to load module: {}", path.string());
                continue;
            }
            const json &addon_info = m_AddonManager->getModule(dir);

            for (const auto &module_name : addon_info["modules"].get<std::vector<std::string>>())
            {
                std::filesystem::path module_path = path / module_name;
                if (!m_ModuleLoader->LoadModule(module_path.string(), module_name))
                {
                    LOG_F(ERROR, "Failed to load module: {}/{}", path.string(), module_name);
                    continue;
                }
                DLOG_F(INFO, "Loaded module: {}/{}", path.string(), module_name);
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

    bool ComponentManager::loadComponent(ComponentType component_type, const json &params)
    {
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
            LOG_F(ERROR, "Failed to load component library: {}", module_name);
            return false;
        }
        if (!loadComponentInfo(module_path))
        {
            LOG_F(ERROR, "Failed to load component info: {}", module_path);
            return false;
        }
        if (!checkComponentInfo(module_name, component_name))
        {
            LOG_F(ERROR, "Failed to load component info: {}", module_path);
            return false;
        }

        auto it = m_ComponentEntries.find(module_name + "." + component_name);
        if (it == m_ComponentEntries.end())
        {
            LOG_F(ERROR, "Failed to load component entry: {}", component_name);
            return false;
        }
        if (it->second->m_component_type == "shared")
        {
            if (!loadSharedComponent(component_name))
            {
                LOG_F(ERROR, "Failed to load shared component: {}", component_name);
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
        // Check if the module has been loaded
        if (m_ModuleLoader->HasModule(module_name))
        {
            LOG_F(WARNING, "Module {} has been loaded, please do not load again", module_name);
            return true;
        }
        // If not, load the module
        // Check component path
        if (!Atom::IO::isFolderExists(module_path))
        {
            LOG_F(ERROR, "Component path {} does not exist", module_path);
            return false;
        }
        // Check component package.json file, this is for the first time loading
        // And we need to know how to load component's ptr from this file
        if (!Atom::IO::isFileExists(module_path + PATH_SEPARATOR + "package.json"))
        {
            LOG_F(ERROR, "Component path {} does not contain package.json", module_path);
            return false;
        }
        // Check component library files
        std::vector<std::string> files = Atom::IO::checkFileTypeInFolder(module_path, DYNAMIC_LIBRARY_EXTENSION, Atom::IO::FileOption::Name);

        if (files.empty())
        {
            LOG_F(ERROR, "Component path {} does not contain dll or so file", module_path);
            return false;
        }
        auto it = std::find(files.begin(), files.end(), module_name + DYNAMIC_LIBRARY_EXTENSION);
        if (it != files.end())
        {
            if (!m_ModuleLoader->LoadModule(module_path + PATH_SEPARATOR + module_name + DYNAMIC_LIBRARY_EXTENSION, module_name))
            {
                LOG_F(ERROR, "Failed to load module: {}'s library {}", module_name, module_path);
                return false;
            }
        }
        else
        {
            LOG_F(ERROR, "Component path {} does not contain specified dll or so file", module_path);
            return false;
        }
        return true;
    }

    bool ComponentManager::loadComponentInfo(const std::string &module_path)
    {
        // Load the Package.json
        // Max: We will only load the root package.json
        std::string file_path = module_path + PATH_SEPARATOR + "package.json";
        if (!Atom::IO::isFileExists(file_path))
        {
            LOG_F(ERROR, "Component path {} does not contain package.json", module_path);
            return false;
        }
        std::string module_name = module_path.substr(module_path.find_last_of(PATH_SEPARATOR) + 1);
        try
        {
            m_ComponentInfos[module_name] = json::parse(std::ifstream(file_path));
        }
        catch (const json::parse_error &e)
        {
            LOG_F(ERROR, "Failed to load package.json file: {}", e.what());
            return false;
        }
        return true;
    }

    bool ComponentManager::checkComponentInfo(const std::string &module_name, const std::string &component_name)
    {
        auto it = m_ComponentInfos.find(module_name);
        if (it == m_ComponentInfos.end())
        {
            LOG_F(ERROR, "Component {} does not contain package.json file", module_name);
            return false;
        }
        auto component_info = m_ComponentInfos[module_name];

        if (component_info["main"].contains(component_name))
        {
            LOG_F(ERROR, "Could not found the main entry to load {}", component_name);
            return false;
        }
        if (component_info["main"][component_name].contains("m_func_name"))
        {
            LOG_F(ERROR, "Component {} does not contain main function", component_name);
            return false;
        }
        // Check if the function exists in the module
        std::string func_name = component_info["main"][component_name]["m_func_name"];
        std::string component_type = component_info["main"][component_name]["m_component_type"];
        if (!m_ModuleLoader->HasFunction(module_name, func_name))
        {
            LOG_F(ERROR, "Failed to load module: {}'s function {}", component_name, func_name);
            return false;
        }
        std::shared_ptr<ComponentEntry> entry = std::make_shared<ComponentEntry>(component_name, func_name, component_type, module_name);
        // TODO: There needs a dependency check. Check if the component is dependent on other components
        m_ComponentEntries[module_name + "." + component_name] = entry;
        return true;
    }

    bool ComponentManager::unloadComponent(ComponentType component_type, const json &params)
    {
        return true;
    }

    bool ComponentManager::reloadComponent(ComponentType component_type, const json &params)
    {
        return true;
    }

    bool ComponentManager::reloadAllComponents()
    {
        return true;
    }

    bool ComponentManager::reloadAllComponents(const json &params)
    {
        return true;
    }

    std::shared_ptr<Component> ComponentManager::getComponent(ComponentType component_type, const std::string &component_name) const
    {
        return nullptr;
    }

    std::shared_ptr<Component> ComponentManager::getComponent(ComponentType component_type, const json &params) const
    {
        return nullptr;
    }

    bool ComponentManager::loadSharedComponent(const std::string &component_name)
    {
        // Classis parameter check
        if (component_name.empty())
        {
            LOG_F(ERROR, "Component name is empty");
            return false;
        }
        if (m_SharedComponents.find(component_name) != m_SharedComponents.end())
        {
            LOG_F(ERROR, "Component {} has been loaded", component_name);
            return false;
        }
        // Load the component entry from the component entries
        auto it = m_ComponentEntries.find(component_name);
        if (it == m_ComponentEntries.end())
        {
            LOG_F(ERROR, "Component {} does not has an entry", component_name);
            return false;
        }
        // There we need some json parameters support for better get the component instance
        if (std::shared_ptr<SharedComponent> component = m_ModuleLoader->GetInstance<SharedComponent>(it->second->m_name, {}, it->second->m_func_name); component)
        {
            try
            {
                // Initialize the component
                component->Initialize();
            }
            catch (const std::exception &e)
            {
                LOG_F(ERROR, "Failed to initialize component: {}", e.what());
                return false;
            }
            m_SharedComponents[component_name] = component;
        }
        else
        {
            LOG_F(ERROR, "Failed to load module: {}'s library", component_name);
            return false;
        }
        DLOG_F(INFO, "Successfully loaded shared component: {}", component_name);
        return true;
    }

    bool ComponentManager::unloadSharedComponent(const json &params)
    {
        IS_ARGUMENT_EMPTY();
        GET_ARGUMENT_C(component_name, std::string);
        if (!m_ModuleLoader->UnloadModule(component_name))
        {
            LOG_F(ERROR, "Failed to unload module: {}", component_name);
            return false;
        }
        DLOG_F(INFO, "Successfully unloaded module: {}", component_name);
        return true;
    }

    bool ComponentManager::reloadSharedComponent(const json &params)
    {
        return true;
    }

    bool ComponentManager::loadScriptComponent(const json &params)
    {
        return true;
    }

    bool ComponentManager::unloadScriptComponent(const json &params)
    {
        return true;
    }

    bool ComponentManager::reloadScriptComponent(const json &params)
    {
        return true;
    }

} // namespace Lithium
