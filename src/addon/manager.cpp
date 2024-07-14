#include "manager.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "addons.hpp"
#include "compiler.hpp"
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

namespace lithium {

ComponentManager::ComponentManager() : sandbox_(nullptr), compiler_(nullptr) {
    module_loader_ = GetWeakPtr<ModuleLoader>(constants::LITHIUM_MODULE_LOADER);
    env_ = GetWeakPtr<atom::utils::Env>(constants::LITHIUM_UTILS_ENV);
    addon_manager_ = GetWeakPtr<AddonManager>(constants::LITHIUM_ADDON_MANAGER);
    sandbox_ = std::make_unique<Sandbox>();
    compiler_ = std::make_unique<Compiler>();

    if (!Initialize()) {
        LOG_F(ERROR, "Failed to initialize component manager");
        THROW_EXCEPTION("Failed to initialize component manager");
    }
    LOG_F(INFO, "Component manager initialized");
}

ComponentManager::~ComponentManager() {}

bool ComponentManager::Initialize() {
    auto env_lock = env_.lock();
    if (!env_lock) {
        LOG_F(ERROR, "Failed to lock environment");
        return false;
    }

    module_path_ = env_lock->getEnv(constants::ENV_VAR_MODULE_PATH, constants::MODULE_FOLDER);

    auto qualified_subdirs = resolveDependencies(getQualifiedSubDirs(module_path_));
    if (qualified_subdirs.empty()) {
        LOG_F(INFO, "No modules found, just skip loading modules");
        return true;
    }

    LOG_F(INFO, "Loading modules from: {}", module_path_);

    auto addon_manager_lock = addon_manager_.lock();
    if (!addon_manager_lock) {
        LOG_F(ERROR, "Failed to lock addon manager");
        return false;
    }

    for (const auto& dir : qualified_subdirs) {
        std::filesystem::path path = std::filesystem::path(module_path_) / dir;

        if (!addon_manager_lock->addModule(path, dir)) {
            LOG_F(ERROR, "Failed to load module: {}", path.string());
            continue;
        }

        const auto& addon_info = addon_manager_lock->getModule(dir);
        if (!addon_info.is_object() || !addon_info.contains("name") || !addon_info["name"].is_string()) {
            LOG_F(ERROR, "Invalid module name: {}", path.string());
            continue;
        }

        auto addon_name = addon_info["name"].get<std::string>();
        LOG_F(INFO, "Start loading addon: {}", addon_name);

        if (!addon_info.contains("modules") || !addon_info["modules"].is_array()) {
            LOG_F(ERROR, "Failed to load module {}: Missing or invalid modules field", path.string());
            addon_manager_lock->removeModule(dir);
            continue;
        }

        for (const auto& component_info : addon_info["modules"]) {
            if (!component_info.is_object() || !component_info.contains("name") || !component_info.contains("entry") || !component_info["name"].is_string() || !component_info["entry"].is_string()) {
                LOG_F(ERROR, "Failed to load module {}/{}: Invalid component info", path.string(), component_info.dump());
                continue;
            }

            auto component_name = component_info["name"].get<std::string>();
            auto entry = component_info["entry"].get<std::string>();
            auto dependencies = component_info.value("dependencies", std::vector<std::string>{});
            auto module_path = path / (component_name + std::string(constants::LIB_EXTENSION));
            auto component_full_name = addon_name + "." + component_name;

            if (!loadComponentInfo(path.string(), component_full_name)) {
                LOG_F(ERROR, "Failed to load addon package.json {}/{}", path.string(), component_full_name);
                return false;
            }

            if (!loadSharedComponent(component_name, addon_name, module_path.string(), entry, dependencies)) {
                LOG_F(ERROR, "Failed to load module {}/{}", path.string(), component_name);
                throw std::runtime_error("Failed to load module: " + component_name);
            }
        }
    }
    return true;
}

bool ComponentManager::Destroy() {
    return true;
}

std::shared_ptr<ComponentManager> ComponentManager::createShared() {
    return std::make_shared<ComponentManager>();
}

std::vector<std::string> ComponentManager::getFilesInDir(const std::string& path) {
    std::vector<std::string> files;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (!entry.is_directory()) {
            files.push_back(entry.path().filename().string());
        }
    }
    return files;
}

std::vector<std::string> ComponentManager::getQualifiedSubDirs(const std::string& path) {
    std::vector<std::string> qualifiedSubDirs;
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_directory()) {
            bool hasJson = false, hasLib = false;
            std::vector<std::string> files = getFilesInDir(entry.path().string());
            for (const auto& fileName : files) {
                if (fileName == constants::PACKAGE_NAME) {
                    hasJson = true;
                } else if (fileName.size() > 4 && fileName.substr(fileName.size() - 4) == constants::LIB_EXTENSION) {
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

bool ComponentManager::loadComponent(ComponentType component_type, const json& params) {
    if (params.is_null()) {
        return false;
    }

    if (!params.contains("module_name") || !params.contains("module_path") || !params.contains("component_name")) {
        LOG_F(ERROR, "{}: Missing arguments", __func__);
        return false;
    }

    auto module_name = params["module_name"].get<std::string>();
    auto module_path = params["module_path"].get<std::string>();
    auto component_name = params["component_name"].get<std::string>();

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

    auto it = component_entries_.find(module_name + "." + component_name);
    if (it == component_entries_.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", component_name);
        return false;
    }
    if (it->second->component_type == "shared") {
        // 仅示例：实际使用时需要实现组件加载逻辑
    }
    return true;
}

bool ComponentManager::checkComponent(const std::string& module_name, const std::string& module_path) {
    if (module_loader_.lock()->HasModule(module_name)) {
        LOG_F(WARNING, "Module {} has been loaded, please do not load again", module_name);
        return true;
    }

    if (!atom::io::isFolderExists(module_path)) {
        LOG_F(ERROR, "Component path {} does not exist", module_path);
        return false;
    }

    if (!atom::io::isFileExists(module_path + constants::PATH_SEPARATOR + constants::PACKAGE_NAME)) {
        LOG_F(ERROR, "Component path {} does not contain package.json", module_path);
        return false;
    }

    auto files = atom::io::checkFileTypeInFolder(module_path, constants::LIB_EXTENSION, atom::io::FileOption::NAME);
    if (files.empty()) {
        LOG_F(ERROR, "Component path {} does not contain dll or so file", module_path);
        return false;
    }

    auto it = std::find(files.begin(), files.end(), module_name + constants::LIB_EXTENSION);
    if (it != files.end()) {
        if (!module_loader_.lock()->LoadModule(module_path + constants::PATH_SEPARATOR + module_name + constants::LIB_EXTENSION, module_name)) {
            LOG_F(ERROR, "Failed to load module: {}'s library {}", module_name, module_path);
            return false;
        }
    } else {
        LOG_F(ERROR, "Component path {} does not contain specified dll or so file", module_path);
        return false;
    }
    return true;
}

bool ComponentManager::loadComponentInfo(const std::string& module_path, const std::string& component_name) {
    std::string file_path = module_path + constants::PATH_SEPARATOR + constants::PACKAGE_NAME;

    if (!std::filesystem::exists(file_path)) {
        LOG_F(ERROR, "Component path {} does not contain package.json", module_path);
        return false;
    }
    try {
        component_infos_[component_name] = json::parse(std::ifstream(file_path));
    } catch (const json::parse_error& e) {
        LOG_F(ERROR, "Failed to load package.json file: {}", e.what());
        return false;
    }
    return true;
}

bool ComponentManager::checkComponentInfo(const std::string& module_name, const std::string& component_name) {
    if (auto it = component_infos_.find(module_name); it == component_infos_.end()) {
        LOG_F(ERROR, "Component {} does not contain package.json file", module_name);
        return false;
    } else {
        const auto& component_info = it->second;

        if (!component_info.contains("modules") || !component_info["modules"].is_array()) {
            LOG_F(ERROR, "Component {} does not contain modules", module_name);
            return false;
        }

        for (const auto& module : component_info["modules"]) {
            if (!module.contains("name") || !module["name"].is_string() || !module.contains("entry") || !module["entry"].is_string()) {
                LOG_F(ERROR, "Component {} does not contain name or entry", module_name);
                return false;
            }

            if (module["name"] == component_name) {
                auto func_name = module["entry"].get<std::string>();

                if (auto module_loader_lock = module_loader_.lock(); module_loader_lock && !module_loader_lock->HasFunction(module_name, func_name)) {
                    LOG_F(ERROR, "Failed to load module: {}'s function {}", component_name, func_name);
                    return false;
                }

                component_entries_[component_name] = std::make_shared<ComponentEntry>(component_name, func_name, "shared", module_name);
                return true;
            }
        }
        return false;
    }
}

bool ComponentManager::unloadComponent(ComponentType component_type, const json& params) {
    if (params.is_null()) {
        return false;
    }

    if (!params.contains("component_name") || !params.contains("forced")) {
        LOG_F(ERROR, "{}: Missing arguments", __func__);
        return false;
    }

    auto component_name = params["component_name"].get<std::string>();
    auto forced = params["forced"].get<bool>();

    auto it = component_entries_.find(component_name);
    if (it == component_entries_.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", component_name);
        return false;
    }
    if (it->second->component_type == "shared") {
        if (!unloadSharedComponent(component_name, forced)) {
            LOG_F(ERROR, "Failed to unload component: {}", component_name);
            return false;
        }
    }
    return true;
}

bool ComponentManager::reloadComponent(ComponentType component_type, const json& params) {
    if (params.is_null()) {
        return false;
    }

    if (!params.contains("component_name")) {
        LOG_F(ERROR, "{}: Missing arguments", __func__);
        return false;
    }

    auto component_name = params["component_name"].get<std::string>();

    auto it = component_entries_.find(component_name);
    if (it == component_entries_.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", component_name);
        return false;
    }
    if (it->second->component_type == "shared") {
        if (!reloadSharedComponent(component_name)) {
            LOG_F(ERROR, "Failed to unload component: {}", component_name);
            return false;
        }
    }
    return true;
}

bool ComponentManager::reloadAllComponents() {
    LOG_F(INFO, "Reloading all components");
    for (auto& [name, component] : components_) {
        if (!reloadComponent(ComponentType::SHARED, json::object({{"component_name", name}}))) {
            LOG_F(ERROR, "Failed to reload component: {}", name);
            return false;
        }
    }
    return true;
}

std::optional<std::weak_ptr<Component>> ComponentManager::getComponent(const std::string& component_name) {
    if (!component_entries_.contains(component_name)) {
        LOG_F(ERROR, "Could not found the component: {}", component_name);
        return std::nullopt;
    }
    return std::optional(components_[component_name]);
}

std::optional<json> ComponentManager::getComponentInfo(const std::string& component_name) {
    if (!component_entries_.contains(component_name)) {
        LOG_F(ERROR, "Could not found the component: {}", component_name);
        return std::nullopt;
    }
    return std::optional(component_infos_[component_name]);
}

std::vector<std::string> ComponentManager::getComponentList() {
    std::vector<std::string> list;
    for (const auto& [name, component] : components_) {
        list.push_back(name);
    }
    std::sort(list.begin(), list.end());
    return list;
}

bool ComponentManager::loadSharedComponent(const std::string& component_name, const std::string& addon_name, const std::string& module_path, const std::string& entry, const std::vector<std::string>& dependencies) {
    auto component_full_name = addon_name + "." + component_name;
    DLOG_F(INFO, "Loading module: {}", component_full_name);

#ifdef _WIN32
    auto module_path_str = atom::utils::replaceString(module_path, "/", "\\");
#else
    auto module_path_str = atom::utils::replaceString(module_path, "\\", "/");
#endif

    auto module_loader = module_loader_.lock();
    if (!module_loader) {
        LOG_F(ERROR, "Failed to lock module loader");
        return false;
    }

    if (!module_loader->LoadModule(module_path_str, component_full_name)) {
        LOG_F(ERROR, "Failed to load module: {}", module_path_str);
        return false;
    }

    if (entry.empty()) {
        LOG_F(ERROR, "Failed to load module: {}/{}", module_path, component_name);
        return false;
    }

    if (auto component = module_loader->GetInstance<Component>(component_full_name, {}, entry); component) {
        LOG_F(INFO, "Loaded shared component: {}", component_full_name);

        try {
            for (const auto& dependency : dependencies) {
                if (!dependency.empty()) {
                    component->addOtherComponent(dependency, GetWeakPtr<Component>(dependency));
                } else {
                    LOG_F(WARNING, "Empty dependency detected");
                }
            }
        } catch (const json::exception& e) {
            LOG_F(ERROR, "Failed to load shared component: {} {}", component_full_name, e.what());
            return false;
        }

        try {
            if (component->initialize()) {
                components_[component_full_name] = component;
                AddPtr(component_full_name, component);
                LOG_F(INFO, "Loaded shared component: {}", component_full_name);

                component_entries_[component_full_name] = std::make_shared<ComponentEntry>(component_name, entry, "shared", module_path);
                return true;
            }
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to initialize shared component: {} {}", component_full_name, e.what());
        }
    } else {
        LOG_F(ERROR, "Failed to load shared component: {}", component_full_name);
    }

    return false;
}

bool ComponentManager::unloadSharedComponent(const std::string& component_name, bool forced) {
    LOG_F(WARNING, "Unload a component is very dangerous, you should make sure everything proper");

    if (!components_.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return false;
    }

    std::vector<std::string> dependencies;
    for (const auto& entry : component_entries_) {
        if (atom::utils::findElement(entry.second->dependencies, component_name)) {
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

bool ComponentManager::reloadSharedComponent(const std::string& component_name) {
    if (!components_.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return false;
    }
    if (!unloadSharedComponent(component_name, false)) {
        LOG_F(ERROR, "Failed to unload component: {}", component_name);
        return false;
    }
    if (!loadSharedComponent(component_name, component_entries_[component_name]->module_name, component_entries_[component_name]->module_name, component_entries_[component_name]->func_name, component_entries_[component_name]->dependencies)) {
        LOG_F(ERROR, "Failed to reload component: {}", component_name);
        return false;
    }
    return true;
}

}  // namespace lithium
