#include "manager.hpp"

#include <filesystem>
#include <fstream>

#include "addons.hpp"
#include "compiler.hpp"
#include "loader.hpp"
#include "sandbox.hpp"
#include "sort.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/ranges.hpp"
#include "atom/utils/string.hpp"

#include "utils/constant.hpp"
#include "utils/marco.hpp"

namespace lithium {

ComponentManager::ComponentManager() : sandbox_(nullptr), compiler_(nullptr) {
    module_loader_ = GetWeakPtr<ModuleLoader>(constants::LITHIUM_MODULE_LOADER);
    env_ = GetWeakPtr<atom::utils::Env>(constants::LITHIUM_UTILS_ENV);
    addon_manager_ = GetWeakPtr<AddonManager>(constants::LITHIUM_ADDON_MANAGER);
    sandbox_ = std::make_unique<Sandbox>();
    compiler_ = std::make_unique<Compiler>();

    if (!initialize()) {
        LOG_F(ERROR, "Failed to initialize component manager");
        THROW_EXCEPTION("Failed to initialize component manager");
    }
    LOG_F(INFO, "Component manager initialized");
}

ComponentManager::~ComponentManager() {}

auto ComponentManager::initialize() -> bool {
    auto envLock = env_.lock();
    if (!envLock) {
        LOG_F(ERROR, "Failed to lock environment");
        return false;
    }

    module_path_ = envLock->getEnv(constants::ENV_VAR_MODULE_PATH,
                                   constants::MODULE_FOLDER);

    auto qualifiedSubdirs =
        resolveDependencies(getQualifiedSubDirs(module_path_));
    if (qualifiedSubdirs.empty()) {
        LOG_F(INFO, "No modules found, just skip loading modules");
        return true;
    }

    LOG_F(INFO, "Loading modules from: {}", module_path_);

    auto addonManagerLock = addon_manager_.lock();
    if (!addonManagerLock) {
        LOG_F(ERROR, "Failed to lock addon manager");
        return false;
    }

    for (const auto& dir : qualifiedSubdirs) {
        std::filesystem::path path = std::filesystem::path(module_path_) / dir;

        if (!addonManagerLock->addModule(path, dir)) {
            LOG_F(ERROR, "Failed to load module: {}", path.string());
            continue;
        }

        const auto& addonInfo = addonManagerLock->getModule(dir);
        if (!addonInfo.is_object() || !addonInfo.contains("name") ||
            !addonInfo["name"].is_string()) {
            LOG_F(ERROR, "Invalid module name: {}", path.string());
            continue;
        }

        auto addonName = addonInfo["name"].get<std::string>();
        LOG_F(INFO, "Start loading addon: {}", addonName);

        if (!addonInfo.contains("modules") ||
            !addonInfo["modules"].is_array()) {
            LOG_F(ERROR,
                  "Failed to load module {}: Missing or invalid modules field",
                  path.string());
            addonManagerLock->removeModule(dir);
            continue;
        }

        for (const auto& componentInfo : addonInfo["modules"]) {
            if (!componentInfo.is_object() || !componentInfo.contains("name") ||
                !componentInfo.contains("entry") ||
                !componentInfo["name"].is_string() ||
                !componentInfo["entry"].is_string()) {
                LOG_F(ERROR,
                      "Failed to load module {}/{}: Invalid component info",
                      path.string(), componentInfo.dump());
                continue;
            }

            auto componentName = componentInfo["name"].get<std::string>();
            auto entry = componentInfo["entry"].get<std::string>();
            auto dependencies =
                componentInfo.value("dependencies", std::vector<std::string>{});
            auto modulePath =
                path / (componentName + std::string(constants::LIB_EXTENSION));
            std::string componentFullName;
            componentFullName.reserve(addonName.length() +
                                      componentName.length() +
                                      1);  // 预留足够空间
            componentFullName.append(addonName).append(".").append(
                componentName);

            if (!loadComponentInfo(path.string(), componentFullName)) {
                LOG_F(ERROR, "Failed to load addon package.json {}/{}",
                      path.string(), componentFullName);
                return false;
            }

            if (!loadSharedComponent(componentName, addonName,
                                     modulePath.string(), entry,
                                     dependencies)) {
                LOG_F(ERROR, "Failed to load module {}/{}", path.string(),
                      componentName);
                throw std::runtime_error("Failed to load module: " +
                                         componentName);
            }
        }
    }
    return true;
}

auto ComponentManager::destroy() -> bool { return true; }

auto ComponentManager::createShared() -> std::shared_ptr<ComponentManager> {
    return std::make_shared<ComponentManager>();
}

auto ComponentManager::getFilesInDir(const std::string& path)
    -> std::vector<std::string> {
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_directory()) {
            files.push_back(entry.path().filename().string());
        }
    }
    return files;
}

auto ComponentManager::getQualifiedSubDirs(const std::string& path)
    -> std::vector<std::string> {
    std::vector<std::string> qualifiedSubDirs;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            bool hasJson = false;
            bool hasLib = false;
            std::vector<std::string> files =
                getFilesInDir(entry.path().string());
            for (const auto& fileName : files) {
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

auto ComponentManager::loadComponent(ComponentType component_type,
                                     const json& params) -> bool {
    if (params.is_null()) {
        return false;
    }

    if (!params.contains("module_name") || !params.contains("module_path") ||
        !params.contains("component_name")) {
        LOG_F(ERROR, "{}: Missing arguments", __func__);
        return false;
    }

    auto moduleName = params["module_name"].get<std::string>();
    auto modulePath = params["module_path"].get<std::string>();
    auto componentName = params["component_name"].get<std::string>();

    if (!checkComponent(moduleName, modulePath)) {
        LOG_F(ERROR, "Failed to load component library: {}", moduleName);
        return false;
    }
    if (!loadComponentInfo(modulePath, componentName)) {
        LOG_F(ERROR, "Failed to load component info: {}", modulePath);
        return false;
    }
    if (!checkComponentInfo(moduleName, componentName)) {
        LOG_F(ERROR, "Failed to load component info: {}", modulePath);
        return false;
    }

    auto it = component_entries_.find(moduleName + "." + componentName);
    if (it == component_entries_.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", componentName);
        return false;
    }
    if (it->second->component_type == "shared") {
        // 仅示例：实际使用时需要实现组件加载逻辑
    }
    return true;
}

auto ComponentManager::checkComponent(const std::string& module_name,
                                      const std::string& module_path) -> bool {
    if (module_loader_.lock()->hasModule(module_name)) {
        LOG_F(WARNING, "Module {} has been loaded, please do not load again",
              module_name);
        return true;
    }

    if (!atom::io::isFolderExists(module_path)) {
        LOG_F(ERROR, "Component path {} does not exist", module_path);
        return false;
    }

    if (!atom::io::isFileExists(module_path + constants::PATH_SEPARATOR +
                                constants::PACKAGE_NAME)) {
        LOG_F(ERROR, "Component path {} does not contain package.json",
              module_path);
        return false;
    }

    auto files = atom::io::checkFileTypeInFolder(
        module_path, constants::LIB_EXTENSION, atom::io::FileOption::NAME);
    if (files.empty()) {
        LOG_F(ERROR, "Component path {} does not contain dll or so file",
              module_path);
        return false;
    }

    auto it = std::find(files.begin(), files.end(),
                        module_name + constants::LIB_EXTENSION);
    if (it != files.end()) {
        if (!module_loader_.lock()->loadModule(
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

auto ComponentManager::loadComponentInfo(
    const std::string& module_path, const std::string& component_name) -> bool {
    std::string file_path =
        module_path + constants::PATH_SEPARATOR + constants::PACKAGE_NAME;

    if (!std::filesystem::exists(file_path)) {
        LOG_F(ERROR, "Component path {} does not contain package.json",
              module_path);
        return false;
    }
    try {
        component_infos_[component_name] =
            json::parse(std::ifstream(file_path));
    } catch (const json::parse_error& e) {
        LOG_F(ERROR, "Failed to load package.json file: {}", e.what());
        return false;
    }
    return true;
}

auto ComponentManager::checkComponentInfo(
    const std::string& module_name, const std::string& component_name) -> bool {
    auto it = component_infos_.find(module_name);
    if (it == component_infos_.end()) {
        LOG_F(ERROR, "Component {} does not contain package.json file",
              module_name);
        return false;
    }
    const auto& componentInfo = it->second;

    if (!componentInfo.contains("modules") ||
        !componentInfo["modules"].is_array()) {
        LOG_F(ERROR, "Component {} does not contain modules", module_name);
        return false;
    }

    for (const auto& module : componentInfo["modules"]) {
        if (!module.contains("name") || !module["name"].is_string() ||
            !module.contains("entry") || !module["entry"].is_string()) {
            LOG_F(ERROR, "Component {} does not contain name or entry",
                  module_name);
            return false;
        }

        if (module["name"] == component_name) {
            auto funcName = module["entry"].get<std::string>();

            if (auto moduleLoaderLock = module_loader_.lock();
                moduleLoaderLock &&
                !moduleLoaderLock->hasFunction(module_name, funcName)) {
                LOG_F(ERROR, "Failed to load module: {}'s function {}",
                      component_name, funcName);
                return false;
            }

            component_entries_[component_name] =
                std::make_shared<ComponentEntry>(component_name, funcName,
                                                 "shared", module_name);
            return true;
        }
    }
    return false;
}

auto ComponentManager::unloadComponent(ComponentType component_type,
                                       const json& params) -> bool {
    if (params.is_null()) {
        return false;
    }

    if (!params.contains("component_name") || !params.contains("forced")) {
        LOG_F(ERROR, "{}: Missing arguments", __func__);
        return false;
    }

    auto componentName = params["component_name"].get<std::string>();
    auto forced = params["forced"].get<bool>();

    auto it = component_entries_.find(componentName);
    if (it == component_entries_.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", componentName);
        return false;
    }
    if (it->second->component_type == "shared") {
        if (!unloadSharedComponent(componentName, forced)) {
            LOG_F(ERROR, "Failed to unload component: {}", componentName);
            return false;
        }
    }
    return true;
}

auto ComponentManager::reloadComponent(ComponentType component_type,
                                       const json& params) -> bool {
    if (params.is_null()) {
        return false;
    }

    if (!params.contains("component_name")) {
        LOG_F(ERROR, "{}: Missing arguments", __func__);
        return false;
    }

    auto componentName = params["component_name"].get<std::string>();

    auto it = component_entries_.find(componentName);
    if (it == component_entries_.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", componentName);
        return false;
    }
    if (it->second->component_type == "shared") {
        if (!reloadSharedComponent(componentName)) {
            LOG_F(ERROR, "Failed to unload component: {}", componentName);
            return false;
        }
    }
    return true;
}

auto ComponentManager::reloadAllComponents() -> bool {
    LOG_F(INFO, "Reloading all components");
    for (auto& [name, component] : components_) {
        if (!reloadComponent(ComponentType::SHARED,
                             json::object({{"component_name", name}}))) {
            LOG_F(ERROR, "Failed to reload component: {}", name);
            return false;
        }
    }
    return true;
}

auto ComponentManager::getComponent(const std::string& component_name)
    -> std::optional<std::weak_ptr<Component>> {
    if (!component_entries_.contains(component_name)) {
        LOG_F(ERROR, "Could not found the component: {}", component_name);
        return std::nullopt;
    }
    return components_[component_name];
}

auto ComponentManager::getComponentInfo(const std::string& component_name)
    -> std::optional<json> {
    if (!component_entries_.contains(component_name)) {
        LOG_F(ERROR, "Could not found the component: {}", component_name);
        return std::nullopt;
    }
    return component_infos_[component_name];
}

auto ComponentManager::getComponentList() -> std::vector<std::string> {
    std::vector<std::string> list;
    for (const auto& [name, component] : components_) {
        list.push_back(name);
    }
    std::sort(list.begin(), list.end());
    return list;
}

auto ComponentManager::loadSharedComponent(
    const std::string& component_name, const std::string& addon_name,
    const std::string& module_path, const std::string& entry,
    const std::vector<std::string>& dependencies) -> bool {
    auto componentFullName = addon_name + "." + component_name;
    DLOG_F(INFO, "Loading module: {}", componentFullName);

#ifdef _WIN32
    auto modulePathStr = atom::utils::replaceString(module_path, "/", "\\");
#else
    auto modulePathStr = atom::utils::replaceString(module_path, "\\", "/");
#endif

    auto moduleLoader = module_loader_.lock();
    if (!moduleLoader) {
        LOG_F(ERROR, "Failed to lock module loader");
        return false;
    }

    if (!moduleLoader->loadModule(modulePathStr, componentFullName)) {
        LOG_F(ERROR, "Failed to load module: {}", modulePathStr);
        return false;
    }

    if (entry.empty()) {
        LOG_F(ERROR, "Failed to load module: {}/{}", module_path,
              component_name);
        return false;
    }

    if (auto component = moduleLoader->getInstance<Component>(
            componentFullName, {}, entry);
        component) {
        LOG_F(INFO, "Loaded shared component: {}", componentFullName);

        try {
            for (const auto& dependency : dependencies) {
                if (!dependency.empty()) {
                    component->addOtherComponent(
                        dependency, GetWeakPtr<Component>(dependency));
                } else {
                    LOG_F(WARNING, "Empty dependency detected");
                }
            }
        } catch (const json::exception& e) {
            LOG_F(ERROR, "Failed to load shared component: {} {}",
                  componentFullName, e.what());
            return false;
        }

        try {
            if (component->initialize()) {
                components_[componentFullName] = component;
                AddPtr(componentFullName, component);
                LOG_F(INFO, "Loaded shared component: {}", componentFullName);

                component_entries_[componentFullName] =
                    std::make_shared<ComponentEntry>(component_name, entry,
                                                     "shared", module_path);
                return true;
            }
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to initialize shared component: {} {}",
                  componentFullName, e.what());
        }
    } else {
        LOG_F(ERROR, "Failed to load shared component: {}", componentFullName);
    }

    return false;
}

auto ComponentManager::unloadSharedComponent(const std::string& component_name,
                                             bool forced) -> bool {
    LOG_F(WARNING,
          "Unload a component is very dangerous, you should make sure "
          "everything proper");

    if (!components_.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return false;
    }

    std::vector<std::string> dependencies;
    for (const auto& entry : component_entries_) {
        if (atom::utils::findElement(entry.second->dependencies,
                                     component_name)) {
            dependencies.push_back(entry.first);
        }
    }

    if (!dependencies.empty()) {
        if (!forced) {
            return false;
        }
        for (const auto& dependency : dependencies) {
            unloadSharedComponent(dependency, forced);
        }
    }

    if (!components_[component_name].lock()->destroy()) {
        LOG_F(ERROR, "Failed to destroy component: {}", component_name);
        return false;
    }
    components_.erase(component_name);
    RemovePtr(component_name);
    LOG_F(INFO, "Unloaded shared component: {}", component_name);
    return true;
}

auto ComponentManager::reloadSharedComponent(const std::string& component_name)
    -> bool {
    if (!components_.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return false;
    }
    if (!unloadSharedComponent(component_name, false)) {
        LOG_F(ERROR, "Failed to unload component: {}", component_name);
        return false;
    }
    if (!loadSharedComponent(
            component_name, component_entries_[component_name]->module_name,
            component_entries_[component_name]->module_name,
            component_entries_[component_name]->func_name,
            component_entries_[component_name]->dependencies)) {
        LOG_F(ERROR, "Failed to reload component: {}", component_name);
        return false;
    }
    return true;
}

}  // namespace lithium
