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

#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/server/global_ptr.hpp"
#include "atom/utils/string.hpp"

#include "utils/constant.hpp"
#include "utils/marco.hpp"

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

namespace Lithium {
ComponentManager::ComponentManager() : m_Sandbox(nullptr), m_Compiler(nullptr) {
    m_ModuleLoader =
        GetWeakPtr<Lithium::ModuleLoader>(constants::LITHIUM_MODULE_LOADER);
    CHECK_WEAK_PTR_EXPIRED(m_ModuleLoader,
                           "load module loader from gpm: lithium.addon.loader");
    m_Env = GetWeakPtr<atom::utils::Env>(constants::LITHIUM_UTILS_ENV);
    CHECK_WEAK_PTR_EXPIRED(m_Env, "load env from gpm: lithium.utils.env");
    m_AddonManager =
        GetWeakPtr<Lithium::AddonManager>(constants::LITHIUM_ADDON_MANAGER);
    CHECK_WEAK_PTR_EXPIRED(m_AddonManager,
                           "load addon manager from gpm: lithium.addon.addon");

    // m_ComponentFinder = std::make_unique<AddonFinder>(
    //     m_Env.lock()->getEnv("LITHIUM_ADDON_PATH", "./modules"), checkFunc);
    // NOTE: AddonFinder is not supported yet

    // Initialize sandbox and compiler, these are not shared objects
    m_Sandbox = std::make_unique<Sandbox>();
    m_Compiler = std::make_unique<Compiler>();

    if (!Initialize()) {
        LOG_F(ERROR, "Failed to initialize component manager");
        throw std::runtime_error("Failed to initialize component manager");
    }
    LOG_F(INFO, "Component manager initialized");
}

ComponentManager::~ComponentManager() {
    m_ModuleLoader.reset();
    m_Env.reset();
    m_AddonManager.reset();
    // m_ComponentFinder.reset();
    m_Sandbox.reset();
    m_Compiler.reset();
}

bool ComponentManager::Initialize() {
    // Check if the module path is valid or reset by the user
    // Default path is ./modules
    // TODO: Windows support
    const std::string &module_path = m_Env.lock()->getEnv(
        constants::ENV_VAR_MODULE_PATH, constants::MODULE_FOLDER);
    // Get all of the available addon path
    /*
    /if (!m_ComponentFinder->traverseDir(std::filesystem::path(module_path))) {
        LOG_F(ERROR, "Failed to traversing module path");
        return false;
    }
    */

    // make a loading list of modules
    std::vector<std::string> qualified_subdirs =
        resolveDependencies(getQualifiedSubDirs(module_path));
    if (qualified_subdirs.empty()) {
        LOG_F(WARNING, "No modules found");
        return true;
    }

    for (const auto &dir : qualified_subdirs) {
        std::filesystem::path path = std::filesystem::path(module_path) / dir;

        if (!m_AddonManager.lock()->addModule(path, dir)) {
            LOG_F(ERROR, "Failed to load module: {}", path.string());
            continue;
        }
        const json &addon_info = m_AddonManager.lock()->getModule(dir);
        if (!addon_info.is_object() || !addon_info.contains("name") ||
            !addon_info["name"].is_string()) {
            LOG_F(ERROR, "Invalid module name: {}", path.string());
            continue;
        }
        auto addon_name = addon_info["name"].get<std::string>();
        // Check if the addon info is valid
        if (!addon_info.contains("modules") || addon_info.is_null()) {
            LOG_F(ERROR, "Failed to load module: {}", path.string());
            LOG_F(ERROR, "Missing modules field in addon info");
            m_AddonManager.lock()->removeModule(dir);
            continue;
        }
        // loading
        for (const auto &module_info :
             addon_info["modules"].get<json::array_t>()) {
            if (module_info.is_null() || !module_info.contains("name") ||
                !module_info.contains("entry")) {
                LOG_F(ERROR, "Failed to load module: {}/{}", path.string(),
                      module_info.dump());
                continue;
            }
            auto module_name =
                addon_name + "." + module_info["name"].get<std::string>();
            std::filesystem::path module_path =
                path / (module_info["name"].get<std::string>() +
                        std::string(constants::LIB_EXTENSION));

            DLOG_F(INFO, "Loading module: {}", module_path.string());
#ifdef _WIN32
            // This is to pass file name check
            auto module_path_str =
                atom::utils::replaceString(module_path.string(), "/", "\\");
#else
            auto module_path_str =
                atom::utils::replaceString(module_path.string(), "\\", "/");
#endif

            // This step is to load the dynamic library
            if (!m_ModuleLoader.lock()->LoadModule(module_path_str,
                                                   module_name)) {
                LOG_F(ERROR, "Failed to load module: {}", module_path_str);
                continue;
            }
            DLOG_F(INFO, "Loaded module: {}/{}", path.string(),
                   module_info.dump());
            auto component_entry = module_info["entry"].get<std::string>();
            if (component_entry.empty()) {
                LOG_F(ERROR, "Failed to load module: {}/{}", path.string(),
                      module_name);
                continue;
            }
            auto component_identifier =
                addon_name + module_name + component_entry;
            if (auto component =
                    m_ModuleLoader.lock()->GetInstance<SharedComponent>(
                        module_name, {}, component_entry);
                component) {
                LOG_F(INFO, "Loaded shared component: {}",
                      component_identifier);
                try {
                    if (component->initialize()) {
                        m_SharedComponents[addon_name + module_name] =
                            component;
                        LOG_F(INFO, "Loaded shared component: {}",
                              component_identifier);
                    } else {
                        LOG_F(ERROR,
                              "Failed to initialize shared component: {}",
                              component_identifier);
                    }
                } catch (const std::exception &e) {
                    LOG_F(ERROR, "Failed to initialize shared component: {}",
                          component_identifier);
                }

            } else {
                LOG_F(ERROR, "Failed to load shared component: {}",
                      component_identifier);
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
    if (!loadComponentInfo(module_path)) {
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
        if (!loadSharedComponent(component_name)) {
            LOG_F(ERROR, "Failed to load shared component: {}", component_name);
            return false;
        }
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
    if (!Atom::IO::isFolderExists(module_path)) {
        LOG_F(ERROR, "Component path {} does not exist", module_path);
        return false;
    }
    // Check component package.json file, this is for the first time loading
    // And we need to know how to load component's ptr from this file
    if (!Atom::IO::isFileExists(module_path + constants::PATH_SEPARATOR +
                                constants::PACKAGE_NAME)) {
        LOG_F(ERROR, "Component path {} does not contain package.json",
              module_path);
        return false;
    }
    // Check component library files
    std::vector<std::string> files = Atom::IO::checkFileTypeInFolder(
        module_path, constants::LIB_EXTENSION, Atom::IO::FileOption::Name);

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

bool ComponentManager::loadComponentInfo(const std::string &module_path) {
    // Load the Package.json
    // Max: We will only load the root package.json
    std::string file_path =
        module_path + constants::PATH_SEPARATOR + constants::PACKAGE_NAME;
    if (!Atom::IO::isFileExists(file_path)) {
        LOG_F(ERROR, "Component path {} does not contain package.json",
              module_path);
        return false;
    }
    std::string module_name = module_path.substr(
        module_path.find_last_of(constants::PATH_SEPARATOR) + 1);
    try {
        m_ComponentInfos[module_name] = json::parse(std::ifstream(file_path));
    } catch (const json::parse_error &e) {
        LOG_F(ERROR, "Failed to load package.json file: {}", e.what());
        return false;
    }
    return true;
}

bool ComponentManager::checkComponentInfo(const std::string &module_name,
                                          const std::string &component_name) {
    auto it = m_ComponentInfos.find(module_name);
    if (it == m_ComponentInfos.end()) {
        LOG_F(ERROR, "Component {} does not contain package.json file",
              module_name);
        return false;
    }
    auto component_info = m_ComponentInfos[module_name];

    if (component_info["main"].contains(component_name)) {
        LOG_F(ERROR, "Could not found the main entry to load {}",
              component_name);
        return false;
    }
    if (component_info["main"][component_name].contains("m_func_name")) {
        LOG_F(ERROR, "Component {} does not contain main function",
              component_name);
        return false;
    }
    // Check if the function exists in the module
    std::string func_name =
        component_info["main"][component_name]["m_func_name"];
    std::string component_type =
        component_info["main"][component_name]["m_component_type"];
    if (!m_ModuleLoader.lock()->HasFunction(module_name, func_name)) {
        LOG_F(ERROR, "Failed to load module: {}'s function {}", component_name,
              func_name);
        return false;
    }
    std::shared_ptr<ComponentEntry> entry = std::make_shared<ComponentEntry>(
        component_name, func_name, component_type, module_name);
    // TODO: There needs a dependency check. Check if the component is dependent
    // on other components
    m_ComponentEntries[module_name + "." + component_name] = entry;
    return true;
}

bool ComponentManager::unloadComponent(ComponentType component_type,
                                       const json &params) {
    return true;
}

bool ComponentManager::reloadComponent(ComponentType component_type,
                                       const json &params) {
    return true;
}

bool ComponentManager::reloadAllComponents() { return true; }

bool ComponentManager::reloadAllComponents(const json &params) { return true; }

std::shared_ptr<Component> ComponentManager::getComponent(
    ComponentType component_type, const std::string &component_name) const {
    return nullptr;
}

std::shared_ptr<Component> ComponentManager::getComponent(
    ComponentType component_type, const json &params) const {
    return nullptr;
}

bool ComponentManager::loadSharedComponent(const std::string &component_name) {
    // Classis parameter check
    if (component_name.empty()) {
        LOG_F(ERROR, "Component name is empty");
        return false;
    }
    if (m_SharedComponents.find(component_name) != m_SharedComponents.end()) {
        LOG_F(ERROR, "Component {} has been loaded", component_name);
        return false;
    }
    // Load the component entry from the component entries
    auto it = m_ComponentEntries.find(component_name);
    if (it == m_ComponentEntries.end()) {
        LOG_F(ERROR, "Component {} does not has an entry", component_name);
        return false;
    }
    // There we need some json parameters support for better get the component
    // instance
    if (std::shared_ptr<SharedComponent> component =
            m_ModuleLoader.lock()->GetInstance<SharedComponent>(
                it->second->m_name, {}, it->second->m_func_name);
        component) {
        try {
            // Initialize the component
            component->initialize();
        } catch (const std::exception &e) {
            LOG_F(ERROR, "Failed to initialize component: {}", e.what());
            return false;
        }
        m_SharedComponents[component_name] = component;
    } else {
        LOG_F(ERROR, "Failed to load module: {}'s library", component_name);
        return false;
    }
    DLOG_F(INFO, "Successfully loaded shared component: {}", component_name);
    return true;
}

bool ComponentManager::unloadSharedComponent(const json &params) {
    IS_ARGUMENT_EMPTY();
    GET_ARGUMENT_C(component_name, std::string);
    if (!m_ModuleLoader.lock()->UnloadModule(component_name)) {
        LOG_F(ERROR, "Failed to unload module: {}", component_name);
        return false;
    }
    DLOG_F(INFO, "Successfully unloaded module: {}", component_name);
    return true;
}

bool ComponentManager::reloadSharedComponent(const json &params) {
    return true;
}

bool ComponentManager::loadScriptComponent(const json &params) { return true; }

bool ComponentManager::unloadScriptComponent(const json &params) {
    return true;
}

bool ComponentManager::reloadScriptComponent(const json &params) {
    return true;
}

}  // namespace Lithium
