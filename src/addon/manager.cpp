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

#include "addons.hpp"
#include "compiler.hpp"
// #include "finder.hpp"
#include "loader.hpp"
#include "sandbox.hpp"
#include "sort.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/ranges.hpp"
#include "atom/utils/string.hpp"

#include "utils/constant.hpp"
#include "utils/marco.hpp"

#include <fstream>

#define IS_ARGUMENT_EMPTY() \
    if (params.is_null()) { \
        return false;       \
    }

#define GET_ARGUMENT_C(name, type)                                  \
    if (!params.contains(#name)) {                                  \
        LOG_F(ERROR, "{}: Missing arguments: {}", __func__, #name); \
        return false;                                               \
    }                                                               \
    type name = params[#name].get<type>();

namespace lithium {
ComponentManager::ComponentManager() : m_Sandbox(nullptr), m_Compiler(nullptr) {
    m_ModuleLoader =
        GetWeakPtr<lithium::ModuleLoader>(constants::LITHIUM_MODULE_LOADER);
    CHECK_WEAK_PTR_EXPIRED(m_ModuleLoader,
                           "load module loader from gpm: lithium.addon.loader");
    m_Env = GetWeakPtr<atom::utils::Env>(constants::LITHIUM_UTILS_ENV);
    CHECK_WEAK_PTR_EXPIRED(m_Env, "load env from gpm: lithium.utils.env");
    m_AddonManager =
        GetWeakPtr<lithium::AddonManager>(constants::LITHIUM_ADDON_MANAGER);
    CHECK_WEAK_PTR_EXPIRED(m_AddonManager,
                           "load addon manager from gpm: lithium.addon.addon");
    // Initialize sandbox and compiler, these are not shared objects
    m_Sandbox = std::make_unique<Sandbox>();
    m_Compiler = std::make_unique<Compiler>();

    if (!Initialize()) {
        LOG_F(ERROR, "Failed to initialize component manager");
        THROW_EXCEPTION("Failed to initialize component manager");
    }
    LOG_F(INFO, "Component manager initialized");
}

ComponentManager::~ComponentManager() {}

bool ComponentManager::Initialize() {
    auto env_lock = m_Env.lock();
    if (!env_lock) {
        LOG_F(ERROR, "Failed to lock environment");
        return false;
    }

    m_module_path = env_lock->getEnv(constants::ENV_VAR_MODULE_PATH,
                                     constants::MODULE_FOLDER);

    const auto &qualified_subdirs =
        resolveDependencies(getQualifiedSubDirs(m_module_path));
    if (qualified_subdirs.empty()) {
        LOG_F(INFO, "No modules found, just skip loading modules");
        return true;
    }

    LOG_F(INFO, "Loading modules from: {}", m_module_path);

#if ENABLE_DEBUG
    LOG_F(INFO, "Available modules:");
    for (const auto &dir : qualified_subdirs) {
        LOG_F(INFO, "{}", dir);
    }
#endif

    auto addon_manager_lock = m_AddonManager.lock();
    if (!addon_manager_lock) {
        LOG_F(ERROR, "Failed to lock addon manager");
        return false;
    }

    for (const auto &dir : qualified_subdirs) {
        std::filesystem::path path = std::filesystem::path(m_module_path) / dir;

        if (!addon_manager_lock->addModule(path, dir)) {
            LOG_F(ERROR, "Failed to load module: {}", path.string());
            continue;
        }

        const auto &addon_info = addon_manager_lock->getModule(dir);
        if (!addon_info.is_object() || !addon_info.contains("name") ||
            !addon_info["name"].is_string()) {
            LOG_F(ERROR, "Invalid module name: {}", path.string());
            continue;
        }

        auto addon_name = addon_info["name"].get<std::string>();
        LOG_F(INFO, "Start loading addon: {}", addon_name);

        if (!addon_info.contains("modules") ||
            !addon_info["modules"].is_array()) {
            LOG_F(ERROR,
                  "Failed to load module {}: Missing or invalid modules field",
                  path.string());
            addon_manager_lock->removeModule(dir);
            continue;
        }

        for (const auto &component_info : addon_info["modules"]) {
            if (!component_info.is_object() ||
                !component_info.contains("name") ||
                !component_info.contains("entry") ||
                !component_info["name"].is_string() ||
                !component_info["entry"].is_string()) {
                LOG_F(ERROR,
                      "Failed to load module {}/{}: Invalid component info",
                      path.string(), component_info.dump());
                continue;
            }

            auto component_name = component_info["name"].get<std::string>();
            auto entry = component_info["entry"].get<std::string>();
            auto dependencies = component_info.value(
                "dependencies", std::vector<std::string>{});
            auto module_path =
                path / (component_name + std::string(constants::LIB_EXTENSION));
            auto component_full_name = addon_name + "." + component_name;

            if (!loadComponentInfo(path.string(), component_full_name)) {
                LOG_F(ERROR, "Failed to load addon package.json {}/{}",
                      path.string(), component_full_name);
                return false;
            }

            if (!loadSharedComponent(component_name, addon_name,
                                     module_path.string(), entry,
                                     dependencies)) {
                LOG_F(ERROR, "Failed to load module {}/{}", path.string(),
                      component_name);
                throw std::runtime_error("Failed to load module: " +
                                         component_name);
            }
        }
    }
    return true;
}

bool ComponentManager::Destroy() { return true; }

std::shared_ptr<ComponentManager> ComponentManager::createShared() {
    return std::make_shared<ComponentManager>();
}

std::vector<std::string> ComponentManager::getFilesInDir(
    const std::string &path) {
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_directory()) {
            files.push_back(entry.path().filename().string());
        }
    }
    return files;
}

std::vector<std::string> ComponentManager::getQualifiedSubDirs(
    const std::string &path) {
    std::vector<std::string> qualifiedSubDirs;
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            bool hasJson = false, hasLib = false;
            std::vector<std::string> files =
                getFilesInDir(entry.path().string());
            for (const auto &fileName : files) {
                if (fileName == constants::PACKAGE_NAME) {
                    hasJson = true;
                } else if (fileName.size() > 4 &&
                           fileName.substr(fileName.size() - 4) ==
                               constants::LIB_EXTENSION) {
                    hasLib = true;
                }
                if (hasJson && hasLib) {
                    break;
                }
            }
            if (hasJson && hasLib) {
                qualifiedSubDirs.push_back(entry.path().string());
            }
        }
    }
    return qualifiedSubDirs;
}

bool ComponentManager::loadComponent(ComponentType component_type,
                                     const json &params) {
    IS_ARGUMENT_EMPTY();
    // Args:
    // module_name: the name of the module(.dll or.so)
    // module_path: the path of the module(.dll or.so)
    // component_name: the name of the component, the name of the ptr will be
    // gotten from the module Others will be load from the package.json
    GET_ARGUMENT_C(module_name, std::string);
    GET_ARGUMENT_C(module_path, std::string);
    GET_ARGUMENT_C(component_name, std::string);

    if (!checkComponent(module_name, module_path)) {
        LOG_F(ERROR, "Failed to load component library: {}", module_name);
        return false;
    }
    if (!loadComponentInfo(module_path, component_name)) {
        LOG_F(ERROR, "Failed to load component info: {}", module_path);
        return false;
    }
    if (!checkComponentInfo(module_name, component_name)) {
        LOG_F(ERROR, "Failed to load component info: {}", module_path);
        return false;
    }

    auto it = m_ComponentEntries.find(module_name + "." + component_name);
    if (it == m_ComponentEntries.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", component_name);
        return false;
    }
    if (it->second->m_component_type == "shared") {
        /*
        if (!loadSharedComponent(component_name)) {
            LOG_F(ERROR, "Failed to load shared component: {}", component_name);
            return false;
        }
        */

    } else if (it->second->m_component_type == "alone") {
    } else if (it->second->m_component_type == "executable") {
    }
    return true;
}

bool ComponentManager::checkComponent(const std::string &module_name,
                                      const std::string &module_path) {
    // Check if the module has been loaded
    if (m_ModuleLoader.lock()->HasModule(module_name)) {
        LOG_F(WARNING, "Module {} has been loaded, please do not load again",
              module_name);
        return true;
    }
    // If not, load the module
    // Check component path
    if (!atom::io::isFolderExists(module_path)) {
        LOG_F(ERROR, "Component path {} does not exist", module_path);
        return false;
    }
    // Check component package.json file, this is for the first time loading
    // And we need to know how to load component's ptr from this file
    if (!atom::io::isFileExists(module_path + constants::PATH_SEPARATOR +
                                constants::PACKAGE_NAME)) {
        LOG_F(ERROR, "Component path {} does not contain package.json",
              module_path);
        return false;
    }
    // Check component library files
    std::vector<std::string> files = atom::io::checkFileTypeInFolder(
        module_path, constants::LIB_EXTENSION, atom::io::FileOption::NAME);

    if (files.empty()) {
        LOG_F(ERROR, "Component path {} does not contain dll or so file",
              module_path);
        return false;
    }
    auto it = std::find(files.begin(), files.end(),
                        module_name + constants::LIB_EXTENSION);
    if (it != files.end()) {
        if (!m_ModuleLoader.lock()->LoadModule(
                module_path + constants::PATH_SEPARATOR + module_name +
                    constants::LIB_EXTENSION,
                module_name)) {
            LOG_F(ERROR, "Failed to load module: {}'s library {}", module_name,
                  module_path);
            return false;
        }
    } else {
        LOG_F(ERROR,
              "Component path {} does not contain specified dll or so file",
              module_path);
        return false;
    }
    return true;
}

bool ComponentManager::loadComponentInfo(const std::string &module_path,
                                         const std::string &component_name) {
    // Load the Package.json
    // Max: We will only load the root package.json
    std::string file_path =
        module_path + constants::PATH_SEPARATOR + constants::PACKAGE_NAME;

    if (!fs::exists(file_path)) {
        LOG_F(ERROR, "Component path {} does not contain package.json",
              module_path);
        return false;
    }
    try {
        m_ComponentInfos[component_name] =
            json::parse(std::ifstream(file_path));
    } catch (const json::parse_error &e) {
        LOG_F(ERROR, "Failed to load package.json file: {}", e.what());
        return false;
    }
    return true;
}

bool ComponentManager::checkComponentInfo(const std::string &module_name,
                                          const std::string &component_name) {
    if (auto it = m_ComponentInfos.find(module_name);
        it == m_ComponentInfos.end()) {
        LOG_F(ERROR, "Component {} does not contain package.json file",
              module_name);
        return false;
    } else {
        const auto &component_info = it->second;

        if (!component_info.contains("modules") ||
            !component_info["modules"].is_array()) {
            LOG_F(ERROR, "Component {} does not contain modules", module_name);
            return false;
        }

        for (const auto &module : component_info["modules"]) {
            if (!module.contains("name") || !module["name"].is_string() ||
                !module.contains("entry") || !module["entry"].is_string()) {
                LOG_F(ERROR, "Component {} does not contain name or entry",
                      module_name);
                return false;
            }

            if (module["name"] == component_name) {
                auto func_name = module["entry"].get<std::string>();

                if (auto module_loader_lock = m_ModuleLoader.lock();
                    module_loader_lock &&
                    !module_loader_lock->HasFunction(module_name, func_name)) {
                    LOG_F(ERROR, "Failed to load module: {}'s function {}",
                          component_name, func_name);
                    return false;
                }

                m_ComponentEntries[component_name] =
                    std::make_shared<ComponentEntry>(component_name, func_name,
                                                     "shared", module_name);
                return true;
            }
        }
        return false;
    }
}

bool ComponentManager::unloadComponent(ComponentType component_type,
                                       const json &params) {
    IS_ARGUMENT_EMPTY();
    // Args:
    // component_name: the name of the component
    GET_ARGUMENT_C(component_name, std::string);
    GET_ARGUMENT_C(forced, bool)

    auto it = m_ComponentEntries.find(component_name);
    if (it == m_ComponentEntries.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", component_name);
        return false;
    }
    if (it->second->m_component_type == "shared") [[likely]] {
        if (!unloadSharedComponent(component_name, forced)) {
            LOG_F(ERROR, "Failed to unload component: {}", component_name);
            return false;
        }
    } else if (it->second->m_component_type == "alone") {
    } else if (it->second->m_component_type == "executable") {
    }
    return true;
}

bool ComponentManager::reloadComponent(ComponentType component_type,
                                       const json &params) {
    IS_ARGUMENT_EMPTY();
    // Args:
    // component_name: the name of the component
    GET_ARGUMENT_C(component_name, std::string);

    auto it = m_ComponentEntries.find(component_name);
    if (it == m_ComponentEntries.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", component_name);
        return false;
    }
    if (it->second->m_component_type == "shared") [[likely]] {
        if (!reloadSharedComponent(component_name)) {
            LOG_F(ERROR, "Failed to unload component: {}", component_name);
            return false;
        }
    } else if (it->second->m_component_type == "alone") {
    } else if (it->second->m_component_type == "executable") {
    }
    return true;
    return true;
}

bool ComponentManager::reloadAllComponents() {
    LOG_F(INFO, "Reloading all components");
    for (auto &[name, component] : m_Components) {
        if (!reloadComponent(ComponentType::SHREAD_INJECTED,
                             json::object({{"component_name", name}}))) {
            LOG_F(ERROR, "Failed to reload component: {}", name);
            return false;
        }
    }
    return true;
}

std::optional<std::weak_ptr<Component>> ComponentManager::getComponent(
    const std::string &component_name) {
    if (!m_ComponentEntries.contains(component_name)) {
        LOG_F(ERROR, "Could not found the component: {}", component_name);
        return std::nullopt;
    }
    return std::optional(m_Components[component_name]);
}

std::optional<json> ComponentManager::getComponentInfo(
    const std::string &component_name) {
    if (!m_ComponentEntries.contains(component_name)) {
        LOG_F(ERROR, "Could not found the component: {}", component_name);
        return std::nullopt;
    }
    return std::optional(m_ComponentInfos[component_name]);
}

std::vector<std::string> ComponentManager::getComponentList() {
    std::vector<std::string> list;
    for (const auto &[name, component] : m_Components) {
        list.push_back(name);
    }
    std::sort(list.begin(), list.end());
    return list;
}

bool ComponentManager::loadSharedComponent(
    const std::string &component_name, const std::string &addon_name,
    const std::string &module_path, const std::string &entry,
    const std::vector<std::string> &dependencies) {
    auto component_full_name = addon_name + "." + component_name;
    DLOG_F(INFO, "Loading module: {}", component_full_name);

#ifdef _WIN32
    auto module_path_str = atom::utils::replaceString(module_path, "/", "\\");
#else
    auto module_path_str = atom::utils::replaceString(module_path, "\\", "/");
#endif

    auto module_loader = m_ModuleLoader.lock();
    if (!module_loader) {
        LOG_F(ERROR, "Failed to lock module loader");
        return false;
    }

    if (!module_loader->LoadModule(module_path_str, component_full_name)) {
        LOG_F(ERROR, "Failed to load module: {}", module_path_str);
        return false;
    }

    if (entry.empty()) {
        LOG_F(ERROR, "Failed to load module: {}/{}", module_path,
              component_name);
        return false;
    }

    if (auto component = module_loader->GetInstance<Component>(
            component_full_name, {}, entry);
        component) {
        LOG_F(INFO, "Loaded shared component: {}", component_full_name);

        try {
            for (const auto &dependency : dependencies) {
                if (!dependency.empty()) {
                    component->addOtherComponent(
                        dependency, GetWeakPtr<Component>(dependency));
                } else {
                    LOG_F(WARNING, "Empty dependency detected");
                }
            }
        } catch (const json::exception &e) {
            LOG_F(ERROR, "Failed to load shared component: {} {}",
                  component_full_name, e.what());
            return false;
        }

        try {
            if (component->initialize()) {
                m_Components[component_full_name] = component;
                AddPtr(component_full_name, component);
                LOG_F(INFO, "Loaded shared component: {}", component_full_name);

                m_ComponentEntries[component_full_name] =
                    std::make_shared<ComponentEntry>(component_name, entry,
                                                     "shared", module_path);
                return true;
            }
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Failed to initialize shared component: {} {}",
                  component_full_name, e.what());
        }
    } else {
        LOG_F(ERROR, "Failed to load shared component: {}",
              component_full_name);
    }

    return false;
}

auto ComponentManager::unloadSharedComponent(const std::string &component_name,
                                             bool forced) -> bool {
    LOG_F(WARNING,
          "Unload a component is very dangerous, you should make sure "
          "everything proper");
    // CHeck if the compoent is loaded
    if (!m_Components.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return false;
    }
    std::vector<std::string> dependencies;  // record all of the components
                                            // which depend on this component
    for (const auto &entry : m_ComponentEntries) {
        if (atom::utils::findElement(entry.second->m_dependencies, component_name)) {
            dependencies.push_back(entry.first);
        }
    }
    if (!dependencies.empty()) {
        if (!forced) {
            return false;
        }
        for (const auto &dependency : dependencies) {
            unloadSharedComponent(dependency, forced);
        }
    }
    // explicit destroy the component
    if (!m_Components[component_name].lock()->destroy()) {
        LOG_F(ERROR, "Failed to destroy component: {}", component_name);
        return false;
    }
    m_Components.erase(component_name);
    RemovePtr(component_name);
    LOG_F(INFO, "Unloaded shared component: {}", component_name);
    return true;
}

bool ComponentManager::reloadSharedComponent(
    const std::string &component_name) {
    if (!m_Components.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return false;
    }
    if (!unloadSharedComponent(component_name, false)) {
        LOG_F(ERROR, "Failed to unload component: {}", component_name);
        return false;
    }
    if (!loadSharedComponent(
            component_name, m_ComponentEntries[component_name]->m_module_name,
            m_ComponentEntries[component_name]->m_module_name,
            m_ComponentEntries[component_name]->m_func_name,
            m_ComponentEntries[component_name]->m_dependencies)) {
        LOG_F(ERROR, "Failed to reload component: {}", component_name);
        return false;
    }
    return true;
}

}  // namespace lithium
