/*
 * component_manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "manager.hpp"

#include <algorithm>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <future>
#include <memory>
#include <mutex>
#include <string>

#include "addon/dependency.hpp"
#include "addons.hpp"
#include "compiler.hpp"
#include "component.hpp"
#include "loader.hpp"
#include "sandbox.hpp"
#include "system_dependency.hpp"
#include "tracker.hpp"

#include "config/configor.hpp"

#include "template/remote.hpp"
#include "template/standalone.hpp"

#include "atom/components/registry.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/io/io.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"
#include "atom/system/env.hpp"
#include "atom/system/process.hpp"
#include "atom/system/process_manager.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"

#include "utils/constant.hpp"
#include "utils/marco.hpp"

#include "atom/macro.hpp"

#if defined(_WIN32) || defined(_WIN64)
// clang-format off
// #include <windows.h>
#include <io.h>
#include <process.h>
#define pipe _pipe
#define popen _popen
#define pclose _pclose
#undef XMLDocument
// clang-format on
#else
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace lithium {

class ComponentManagerImpl {
public:
    std::weak_ptr<ModuleLoader> moduleLoader;
    std::weak_ptr<atom::utils::Env> env;
    std::shared_ptr<Sandbox> sandbox;
    std::shared_ptr<Compiler> compiler;
    std::shared_ptr<FileTracker> fileTracker;
    std::weak_ptr<AddonManager> addonManager;
    std::shared_ptr<DependencyManager> dependencyManager;
    std::unordered_map<std::string, std::shared_ptr<ComponentEntry>>
        componentEntries;
    std::weak_ptr<atom::system::ProcessManager> processManager;
    std::weak_ptr<ConfigManager> configManager;
    std::unordered_map<std::string, json> componentInfos;
    std::unordered_map<std::string, std::weak_ptr<Component>> components;
    std::string modulePath;
    DependencyGraph dependencyGraph;
    std::mutex mutex;
    std::future<void> fileTrackerFuture;
};

ComponentManager::ComponentManager()
    : impl_(std::make_unique<ComponentManagerImpl>()) {
    impl_->moduleLoader = GetWeakPtr<ModuleLoader>(Constants::MODULE_LOADER);
    impl_->env = GetWeakPtr<atom::utils::Env>(Constants::ENVIRONMENT);
    impl_->addonManager = GetWeakPtr<AddonManager>(Constants::ADDON_MANAGER);
    impl_->processManager =
        GetWeakPtr<atom::system::ProcessManager>(Constants::PROCESS_MANAGER);
    impl_->sandbox = std::make_shared<Sandbox>();
    impl_->compiler = std::make_shared<Compiler>();
    impl_->dependencyManager = std::make_shared<DependencyManager>();

    GET_OR_CREATE_WEAK_PTR(impl_->configManager, ConfigManager,
                           Constants::CONFIG_MANAGER);

    if (!initialize()) {
        LOG_F(ERROR, "Failed to initialize component manager");
        THROW_EXCEPTION("Failed to initialize component manager");
    }
    LOG_F(INFO, "Component manager initialized");
}

ComponentManager::~ComponentManager() = default;

auto ComponentManager::initialize() -> bool {
    LOG_F(INFO, "Initializing component manager");

    if (!lockEnvironment()) {
        return false;
    }

    startFileTracker();

    if (!loadComponentDirectory()) {
        return false;
    }

    initializeRegistryComponents();
    try {
        impl_->fileTrackerFuture.get();
    } catch (const std::future_error& e) {
        LOG_F(ERROR, "Failed to get file tracker future: {}", e.what());
        return false;
    }

    if (!loadModules()) {
        return false;
    }

    LOG_F(INFO, "Component manager initialized successfully");
    return true;
}

bool ComponentManager::lockEnvironment() {
    auto envLock = impl_->env.lock();
    if (!envLock) {
        LOG_F(ERROR, "Failed to lock environment");
        return false;
    }
    LOG_F(INFO, "Environment locked successfully");
    return true;
}

void ComponentManager::startFileTracker() {
    LOG_F(INFO, "Starting file tracker and creating status json file");
    auto statusFile = impl_->env.lock()->getEnv(
        Constants::COMPONENT_STATUS_FILE_ENV, Constants::COMPONENT_STATUS_FILE);
    auto componentDir = impl_->env.lock()->getEnv(Constants::COMPONENT_PATH_ENV,
                                                  Constants::COMPONENT_PATH);
    impl_->fileTracker = std::make_shared<FileTracker>(
        componentDir, statusFile,
        std::vector<std::string>{Constants::LIB_EXTENSION, ".json", ".xml"},
        true);
    impl_->fileTrackerFuture = impl_->fileTracker->asyncScan();
    LOG_F(INFO, "File tracker started");
}

auto ComponentManager::loadComponentDirectory() -> bool {
    auto envLock = impl_->env.lock();
    auto componentDir = envLock->getEnv(Constants::COMPONENT_PATH_ENV,
                                        Constants::COMPONENT_PATH);
    LOG_F(INFO, "Component directory: {}", componentDir);

    if (auto value = impl_->configManager.lock()->getValue("/app/modules/path");
        value.has_value()) {
        try {
            componentDir = value.value();
            LOG_F(INFO, "Component directory from config: {}", componentDir);
            if (atom::io::isFolderExists(value.value())) {
                LOG_F(INFO, "Component directory loaded from config exists");
            } else {
                LOG_F(
                    ERROR,
                    "Component directory loaded from config does not exist: {}",
                    value.value().dump());
                return false;
            }
        } catch (const json::parse_error& e) {
            LOG_F(ERROR, "Failed to parse config: {}", e.what());
        } catch (const json::exception& e) {
            LOG_F(ERROR, "Failed to get module path from config: {}", e.what());
        }
    }

    impl_->modulePath = componentDir;
    LOG_F(INFO, "Module path set to: {}", impl_->modulePath);
    return true;
}

void ComponentManager::initializeRegistryComponents() {
    LOG_F(INFO, "Initializing all registry components");
    Registry::instance().initializeAll();
    for (const auto& name : Registry::instance().getAllComponentNames()) {
        LOG_F(INFO, "Registering component: {}", name);
    }
    for (const auto& component : Registry::instance().getAllComponents()) {
        impl_->components[component->getName()] = component;
        impl_->componentInfos[component->getName()] = json();
        impl_->componentEntries[component->getName()] =
            std::make_shared<ComponentEntry>(component->getName(), "builtin",
                                             "embed", "main");
        LOG_F(INFO, "Component registered: {}", component->getName());
    }
}

auto ComponentManager::loadModules() -> bool {
    // Resolve system dependencies first, then resolve other dependencies
    auto systemDeps = impl_->dependencyGraph.resolveSystemDependencies(
        getQualifiedSubDirs(impl_->modulePath));

    for (const auto& [dep, version] : systemDeps) {
        impl_->dependencyManager->addDependency({dep, version.toString()});
    }
    impl_->dependencyManager->checkAndInstallDependencies();

    auto qualifiedSubdirs = impl_->dependencyGraph.resolveDependencies(
        getQualifiedSubDirs(impl_->modulePath));
    if (qualifiedSubdirs.empty()) {
        LOG_F(INFO, "No modules found, just skip loading modules");
        return true;
    }

    LOG_F(INFO, "Loading modules from: {}", impl_->modulePath);

    auto addonManagerLock = impl_->addonManager.lock();
    if (!addonManagerLock) {
        LOG_F(ERROR, "Failed to lock addon manager");
        return false;
    }
    LOG_F(INFO, "Addon manager locked successfully");

    impl_->fileTracker->asyncScan().get();

    for (const auto& dir : qualifiedSubdirs) {
        if (!loadModule(dir, addonManagerLock)) {
            return false;
        }
    }
    return true;
}

auto ComponentManager::loadModule(
    const std::string& dir,
    const std::shared_ptr<AddonManager>& addonManagerLock) -> bool {
    std::filesystem::path path = std::filesystem::path(impl_->modulePath) / dir;
    LOG_F(INFO, "Loading module: {}", path.string());

    if (!addonManagerLock->addModule(path, dir)) {
        LOG_F(ERROR, "Failed to load module: {}", path.string());
        return false;
    }

    const auto& addonInfo = addonManagerLock->getModule(dir);
    if (!addonInfo.is_object() || !addonInfo.contains("name") ||
        !addonInfo["name"].is_string()) {
        LOG_F(ERROR, "Invalid module name: {}", path.string());
        return false;
    }

    auto addonName = addonInfo["name"].get<std::string>();
    LOG_F(INFO, "Start loading addon: {}", addonName);

    if (!addonInfo.contains("modules") || !addonInfo["modules"].is_array()) {
        LOG_F(ERROR,
              "Failed to load module {}: Missing or invalid modules field",
              path.string());
        addonManagerLock->removeModule(dir);
        return false;
    }

    for (const auto& componentInfo : addonInfo["modules"]) {
        if (!componentInfo.is_object() || !componentInfo.contains("name") ||
            !componentInfo.contains("entry") ||
            !componentInfo["name"].is_string() ||
            !componentInfo["entry"].is_string()) {
            LOG_F(ERROR, "Failed to load module {}/{}: Invalid component info",
                  path.string(), componentInfo.dump());
            continue;
        }

        auto componentName = componentInfo["name"].get<std::string>();
        auto entry = componentInfo["entry"].get<std::string>();
        auto dependencies =
            componentInfo.value("dependencies", std::vector<std::string>{});
        auto modulePath =
            path / (componentName + std::string(Constants::LIB_EXTENSION));
        std::string componentFullName;
        componentFullName.reserve(addonName.length() + componentName.length() +
                                  1);
        componentFullName.append(addonName).append(".").append(componentName);

        LOG_F(INFO, "Loading component info for: {}", componentFullName);
        if (!loadComponentInfo(path.string(), componentFullName)) {
            LOG_F(ERROR, "Failed to load addon package.json {}/{}",
                  path.string(), componentFullName);
            return false;
        }

        LOG_F(INFO, "Loading shared component: {}", componentFullName);
        if (!loadSharedComponent(componentName, addonName, path.string(), entry,
                                 dependencies)) {
            LOG_F(ERROR, "Failed to load module {}/{}", path.string(),
                  componentName);
            THROW_RUNTIME_ERROR("Failed to load module: " + componentName);
        }
    }
    return true;
}

auto ComponentManager::destroy() -> bool {
    std::lock_guard lock(impl_->mutex);
    // 销毁组件管理器的逻辑
    impl_->components.clear();
    impl_->componentInfos.clear();
    impl_->componentEntries.clear();
    return true;
}

auto ComponentManager::createShared() -> std::shared_ptr<ComponentManager> {
    return std::make_shared<ComponentManager>();
}

auto ComponentManager::scanComponents(const std::string& path)
    -> std::vector<std::string> {
    // Once we call scanComponents, we will rerun file tracker
    std::vector<std::string> foundComponents;
    impl_->fileTrackerFuture = impl_->fileTracker->asyncScan();
    try {
        auto subDirs = getQualifiedSubDirs(path);
        for (const auto& subDir : subDirs) {
            auto componentFiles = getFilesInDir(subDir);
            foundComponents.insert(foundComponents.end(),
                                   componentFiles.begin(),
                                   componentFiles.end());
        }
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to scan components: {}", e.what());
    }
    // Wait for file tracker to finish
    impl_->fileTrackerFuture.get();
    return foundComponents;
}

auto ComponentManager::getFilesInDir(const std::string& path)
    -> std::vector<std::string> {
    std::vector<std::string> files;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (!entry.is_directory()) {
                files.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_F(ERROR, "Error accessing directory {}: {}", path, e.what());
    }
    return files;
}

auto ComponentManager::getQualifiedSubDirs(const std::string& path)
    -> std::vector<std::string> {
    std::vector<std::string> qualifiedSubDirs;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                bool hasJson = false;
                bool hasLib = false;
                LOG_F(INFO, "Checking directory: {}", entry.path().string());
                auto files = getFilesInDir(entry.path().string());
                for (const auto& fileName : files) {
                    if (fileName == Constants::PACKAGE_NAME) {
                        hasJson = true;
                    } else if (fileName.size() > 4 &&
#ifdef _WIN32
                               fileName.substr(fileName.size() - 4) ==
                                   Constants::LIB_EXTENSION
#else
                               fileName.substr(fileName.size() - 3) ==
                                   Constants::LIB_EXTENSION
#endif
                    ) {
                        hasLib = true;
                    }
                    LOG_F(INFO, "Dir {} has json: {}, lib: {}",
                          entry.path().string(), hasJson, hasLib);
                    if (hasJson && hasLib) {
                        break;
                    }
                }
                if (hasJson && hasLib) {
                    qualifiedSubDirs.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_F(ERROR, "Error accessing directory {}: {}", path, e.what());
    }
    return qualifiedSubDirs;
}

auto ComponentManager::loadComponent(const json& params) -> bool {
    std::lock_guard lock(impl_->mutex);
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

    auto it = impl_->componentEntries.find(moduleName + "." + componentName);
    if (it == impl_->componentEntries.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", componentName);
        return false;
    }
    std::vector<std::string> dependencyVersions;
    for (const auto& dependency :
         impl_->componentInfos[componentName]["dependencies"]) {
        dependencyVersions.push_back(dependency["version"].get<std::string>());
    }
    for (const auto& dependency : it->second->dependencies) {
        if (!checkComponent(dependency, modulePath)) {
            LOG_F(ERROR, "Failed to load dependency: {}", dependency);
            return false;
        }
    }
    if (it->second->component_type == "shared") {
        if (loadSharedComponent(componentName, moduleName, modulePath,
                                it->second->func_name,
                                it->second->dependencies)) {
            updateDependencyGraph(
                componentName, impl_->componentInfos[componentName]["version"],
                it->second->dependencies, dependencyVersions);
            return true;
        }
    } else if (it->second->component_type == "standalone") {
        if (loadStandaloneComponent(componentName, moduleName, modulePath,
                                    it->second->func_name,
                                    it->second->dependencies)) {
            updateDependencyGraph(
                componentName, impl_->componentInfos[componentName]["version"],
                it->second->dependencies, dependencyVersions);
            return true;
        }
        LOG_F(ERROR, "Unknown component type: {}", componentName);
        return false;
    }
    return true;
}

auto ComponentManager::checkComponent(const std::string& module_name,
                                      const std::string& module_path) -> bool {
    auto moduleLoader = impl_->moduleLoader.lock();
    if (moduleLoader && moduleLoader->hasModule(module_name)) {
        LOG_F(WARNING, "Module {} has been loaded, please do not load again",
              module_name);
        return true;
    }

    if (!std::filesystem::exists(module_path)) {
        LOG_F(ERROR, "Component path {} does not exist", module_path);
        return false;
    }

    if (!std::filesystem::exists(module_path + Constants::PATH_SEPARATOR +
                                 Constants::PACKAGE_NAME)) {
        LOG_F(ERROR, "Component path {} does not contain package.json",
              module_path);
        return false;
    }

    auto files = getFilesInDir(module_path);
    auto it = std::find_if(files.begin(), files.end(),
                           [&](const std::string& fileName) {
                               return fileName.size() > 4 &&
                                      fileName.substr(fileName.size() - 4) ==
                                          Constants::LIB_EXTENSION;
                           });

    if (it == files.end()) {
        LOG_F(ERROR,
              "Component path {} does not contain specified dll or so file",
              module_path);
        return false;
    }

    if (moduleLoader &&
        !moduleLoader->loadModule(module_path + Constants::PATH_SEPARATOR + *it,
                                  module_name)) {
        LOG_F(ERROR, "Failed to load module: {}'s library {}", module_name,
              module_path);
        return false;
    }
    return true;
}

auto ComponentManager::loadComponentInfo(
    const std::string& module_path, const std::string& component_name) -> bool {
    std::string filePath =
        module_path + Constants::PATH_SEPARATOR + Constants::PACKAGE_NAME;

    if (!std::filesystem::exists(filePath)) {
        LOG_F(ERROR, "Component path {} does not contain package.json",
              module_path);
        return false;
    }
    try {
        std::ifstream file(filePath);
        impl_->componentInfos[component_name] = json::parse(file);
    } catch (const json::parse_error& e) {
        LOG_F(ERROR, "Failed to load package.json file: {}", e.what());
        return false;
    }
    return true;
}

auto ComponentManager::checkComponentInfo(
    const std::string& module_name, const std::string& component_name) -> bool {
    auto it = impl_->componentInfos.find(module_name);
    if (it == impl_->componentInfos.end()) {
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

            auto moduleLoaderLock = impl_->moduleLoader.lock();
            if (moduleLoaderLock &&
                !moduleLoaderLock->hasFunction(module_name, funcName)) {
                LOG_F(ERROR, "Failed to load module: {}'s function {}",
                      component_name, funcName);
                return false;
            }

            impl_->componentEntries[component_name] =
                std::make_shared<ComponentEntry>(component_name, funcName,
                                                 "shared", module_name);
            return true;
        }
    }
    return false;
}

auto ComponentManager::unloadComponent(const json& params) -> bool {
    std::lock_guard lock(impl_->mutex);
    if (params.is_null()) {
        return false;
    }

    if (!params.contains("component_name") || !params.contains("forced")) {
        LOG_F(ERROR, "{}: Missing arguments", __func__);
        return false;
    }

    auto componentName = params["component_name"].get<std::string>();
    auto forced = params["forced"].get<bool>();

    auto it = impl_->componentEntries.find(componentName);
    if (it == impl_->componentEntries.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", componentName);
        return false;
    }
    if (it->second->component_type == "shared") {
        if (!unloadSharedComponent(componentName, forced)) {
            LOG_F(ERROR, "Failed to unload component: {}", componentName);
            impl_->dependencyGraph.removeNode(componentName);
            return false;
        }
    } else if (it->second->component_type == "standalone") {
        if (!unloadStandaloneComponent(componentName, forced)) {
            LOG_F(ERROR, "Failed to unload standalone component: {}",
                  componentName);
            impl_->dependencyGraph.removeNode(componentName);
            return false;
        }
    }
    return true;
}

auto ComponentManager::reloadComponent(const json& params) -> bool {
    std::lock_guard lock(impl_->mutex);
    if (params.is_null()) {
        return false;
    }

    if (!params.contains("component_name")) {
        LOG_F(ERROR, "{}: Missing arguments", __func__);
        return false;
    }

    auto componentName = params["component_name"].get<std::string>();

    auto it = impl_->componentEntries.find(componentName);
    if (it == impl_->componentEntries.end()) {
        LOG_F(ERROR, "Failed to load component entry: {}", componentName);
        return false;
    }
    if (it->second->component_type == "shared") {
        if (!reloadSharedComponent(componentName)) {
            LOG_F(ERROR, "Failed to reload component: {}", componentName);
            return false;
        }
    } else if (it->second->component_type == "standalone") {
        if (!reloadStandaloneComponent(componentName)) {
            LOG_F(ERROR, "Failed to reload standalone component: {}",
                  componentName);
            return false;
        }
    }
    return true;
}

auto ComponentManager::reloadAllComponents() -> bool {
    std::lock_guard lock(impl_->mutex);
    LOG_F(INFO, "Reloading all components");
    for (const auto& [name, component] : impl_->components) {
        if (!reloadComponent(json::object({{"component_name", name}}))) {
            LOG_F(ERROR, "Failed to reload component: {}", name);
            return false;
        }
    }
    return true;
}

auto ComponentManager::getComponent(const std::string& component_name)
    -> std::optional<std::weak_ptr<Component>> {
    std::lock_guard lock(impl_->mutex);
    if (!impl_->componentEntries.contains(component_name)) {
        LOG_F(ERROR, "Could not find the component: {}", component_name);
        return std::nullopt;
    }
    return impl_->components[component_name];
}

auto ComponentManager::getComponentInfo(const std::string& component_name)
    -> std::optional<json> {
    std::lock_guard lock(impl_->mutex);
    if (!impl_->componentEntries.contains(component_name)) {
        LOG_F(ERROR, "Could not find the component: {}", component_name);
        return std::nullopt;
    }
    return impl_->componentInfos[component_name];
}

auto ComponentManager::getComponentList() -> std::vector<std::string> {
    std::lock_guard lock(impl_->mutex);
    std::vector<std::string> list;
    for (const auto& [name, component] : impl_->components) {
        list.push_back(name);
    }
    std::sort(list.begin(), list.end());
    return list;
}

auto ComponentManager::hasComponent(const std::string& component_name) -> bool {
    std::lock_guard lock(impl_->mutex);
    return impl_->componentEntries.contains(component_name);
}

auto ComponentManager::loadSharedComponent(
    const std::string& component_name, const std::string& addon_name,
    const std::string& module_path, const std::string& entry,
    const std::vector<std::string>& dependencies) -> bool {
    std::lock_guard lock(impl_->mutex);
    auto componentFullName = addon_name + "." + component_name;
    DLOG_F(INFO, "Loading module: {}", componentFullName);
    auto modulePathStr =
#ifdef _WIN32
        atom::utils::replaceString(module_path, "/", "\\")
#else
        atom::utils::replaceString(module_path, "\\", "/")
#endif
        + Constants::PATH_SEPARATOR + component_name + Constants::LIB_EXTENSION;

    auto moduleLoader = impl_->moduleLoader.lock();
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

    // Max: 对组件进行初始化，如果存在可以使用的初始化函数
    auto moduleInitFunc = moduleLoader->getFunction<void()>(
        componentFullName, "initialize_registry");
    if (moduleInitFunc != nullptr) {
        LOG_F(INFO, "Initializing registry for shared component: {}",
              componentFullName);
        moduleInitFunc();
        LOG_F(INFO, "Initialized registry for shared component: {}",
              componentFullName);
    }

    if (auto component =
            moduleLoader->getInstance<Component>(componentFullName, {}, entry);
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
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Failed to load shared component: {} {}",
                  componentFullName, e.what());
            return false;
        }

        try {
            if (component->initialize()) {
                impl_->components[componentFullName] = component;
                AddPtr(componentFullName, component);
                LOG_F(INFO, "Loaded shared component: {}", componentFullName);

                impl_->componentEntries[componentFullName] =
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
          "Unloading a component is very dangerous, you should make sure "
          "everything is proper");

    if (!impl_->components.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return false;
    }

    std::vector<std::string> dependencies;
    for (const auto& entry : impl_->componentEntries) {
        if (std::find(entry.second->dependencies.begin(),
                      entry.second->dependencies.end(),
                      component_name) != entry.second->dependencies.end()) {
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

    if (!impl_->components[component_name].lock()->destroy()) {
        LOG_F(ERROR, "Failed to destroy component: {}", component_name);
        return false;
    }
    impl_->components.erase(component_name);
    RemovePtr(component_name);
    LOG_F(INFO, "Unloaded shared component: {}", component_name);
    return true;
}

auto ComponentManager::reloadSharedComponent(const std::string& component_name)
    -> bool {
    if (!impl_->components.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return false;
    }
    if (!unloadSharedComponent(component_name, false)) {
        LOG_F(ERROR, "Failed to unload component: {}", component_name);
        return false;
    }
    if (!loadSharedComponent(
            component_name,
            impl_->componentEntries[component_name]->module_name,
            impl_->componentEntries[component_name]->module_name,
            impl_->componentEntries[component_name]->func_name,
            impl_->componentEntries[component_name]->dependencies)) {
        LOG_F(ERROR, "Failed to reload component: {}", component_name);
        return false;
    }
    return true;
}

auto ComponentManager::loadStandaloneComponent(
    const std::string& component_name,
    [[maybe_unused]] const std::string& addon_name,
    const std::string& module_path, const std::string& entry,
    const std::vector<std::string>& dependencies) -> bool {
    std::lock_guard lock(impl_->mutex);
    for (const auto& [name, component] : impl_->components) {
        if (name == component_name) {
            LOG_F(ERROR, "Component {} is already loaded", component_name);
            return false;
        }
    }
    if (atom::system::isProcessRunning(component_name)) {
        LOG_F(ERROR, "Component {} is already running, killing it",
              component_name);
        atom::system::killProcessByName(component_name, SIGTERM);
        LOG_F(INFO, "Killed process {}", component_name);
        if (atom::system::isProcessRunning(component_name)) {
            LOG_F(ERROR, "Failed to kill process {}", component_name);
            return false;
        }
    }
    for (const auto& dependency : dependencies) {
        if (!atom::system::isProcessRunning(dependency)) {
            LOG_F(ERROR, "Dependency {} is not running", dependency);
            return false;
        }
    }
    auto componentFullPath = module_path + Constants::PATH_SEPARATOR +
                             component_name + Constants::EXECUTABLE_EXTENSION;
    auto standaloneComponent =
        std::make_shared<StandAloneComponent>(component_name);
    standaloneComponent->startLocalDriver(componentFullPath,
                                          InteractionMethod::Pipe);
    if (!standaloneComponent->initialize()) {
        LOG_F(ERROR, "Failed to initialize component {}", component_name);
        return false;
    }
    standaloneComponent->toggleDriverListening();
    LOG_F(INFO, "Start listening to driver for component {}", component_name);
    standaloneComponent->monitorDrivers();
    LOG_F(INFO, "Start monitoring drivers for component {}", component_name);

    impl_->components[component_name] = standaloneComponent;
    impl_->componentEntries[component_name] = std::make_shared<ComponentEntry>(
        component_name, entry, "standalone", module_path);
    LOG_F(INFO, "Successfully loaded standalone component {}", component_name);
    return true;
}

auto ComponentManager::unloadStandaloneComponent(
    const std::string& component_name, bool forced) -> bool {
    std::lock_guard lock(impl_->mutex);
    auto it = impl_->components.find(component_name);
    if (it == impl_->components.end()) {
        LOG_F(WARNING, "Component {} is not loaded", component_name);
        return true;
    }
    auto component = it->second.lock();
    if (!component) {
        LOG_F(ERROR, "Component {} is expired", component_name);
        impl_->components.erase(component_name);
        return true;
    }
    auto standaloneComponent =
        std::dynamic_pointer_cast<StandAloneComponent>(component);
    if (forced) {
        LOG_F(INFO, "Forcefully unloading component {}", component_name);
        if (!standaloneComponent->destroy()) {
            LOG_F(ERROR, "Failed to destroy component {}", component_name);
        }
        impl_->components.erase(component_name);
        return true;
    }
    LOG_F(INFO, "Unloaded standalone component {}", component_name);
    return true;
}

auto ComponentManager::reloadStandaloneComponent(
    const std::string& component_name) -> bool {
    std::lock_guard lock(impl_->mutex);
    if (!impl_->components.contains(component_name)) {
        LOG_F(ERROR, "Component {} not found", component_name);
        return false;
    }
    if (!unloadStandaloneComponent(component_name, false)) {
        LOG_F(ERROR, "Failed to unload standalone component: {}",
              component_name);
        return false;
    }
    if (!loadStandaloneComponent(
            component_name,
            impl_->componentEntries[component_name]->module_name,
            impl_->componentEntries[component_name]->module_name,
            impl_->componentEntries[component_name]->func_name,
            impl_->componentEntries[component_name]->dependencies)) {
        LOG_F(ERROR, "Failed to reload standalone component: {}",
              component_name);
        return false;
    }
    return true;
}

auto ComponentManager::loadRemoteComponent(
    const std::string& component_name, const std::string& addon_name,
    const std::string& module_path, const std::string& entry,
    const std::vector<std::string>& dependencies) -> bool {
    std::lock_guard lock(impl_->mutex);
    for (const auto& [name, component] : impl_->components) {
        if (name == component_name) {
            LOG_F(ERROR, "Component {} is already loaded", component_name);
            return false;
        }
    }
    if (atom::system::isProcessRunning(component_name)) {
        LOG_F(ERROR, "Component {} is already running, killing it",
              component_name);
        atom::system::killProcessByName(component_name, SIGTERM);
        LOG_F(INFO, "Killed process {}", component_name);
        if (atom::system::isProcessRunning(component_name)) {
            LOG_F(ERROR, "Failed to kill process {}", component_name);
            return false;
        }
    }
    for (const auto& dependency : dependencies) {
        if (!atom::system::isProcessRunning(dependency)) {
            LOG_F(ERROR, "Dependency {} is not running", dependency);
            return false;
        }
    }
    auto componentFullPath = module_path + Constants::PATH_SEPARATOR +
                             component_name + Constants::EXECUTABLE_EXTENSION;
    auto remoteComponent = std::make_shared<RemoteStandAloneComponent>(
        component_name);

    LOG_F(INFO, "Successfully loaded remote component {}", component_name);
    return true;
}

void ComponentManager::updateDependencyGraph(
    const std::string& component_name, const std::string& version,
    const std::vector<std::string>& dependencies,
    const std::vector<std::string>& dependencies_version) {
    impl_->dependencyGraph.addNode(component_name, Version::parse(version));
    for (auto i = 0; i < static_cast<int>(dependencies.size()); i++) {
        impl_->dependencyGraph.addDependency(
            component_name, dependencies[i],
            Version::parse(dependencies_version[i]));
    }
}

auto ComponentManager::savePackageLock(const std::string& filename) -> bool {
    std::lock_guard lock(impl_->mutex);

    json packageLock;
    packageLock["name"] = "component-manager";
    packageLock["version"] = "1.0.0";
    packageLock["dependencies"] = json::object();

    for (const auto& [component_name, component] : impl_->components) {
        json componentInfo;
        componentInfo["version"] =
            impl_->componentInfos[component_name]["version"];
        componentInfo["resolved"] =
            impl_->componentInfos[component_name]["resolved"];
        componentInfo["dependencies"] = json::object();

        auto dependencies =
            impl_->dependencyGraph.getAllDependencies(component_name);
        for (const auto& dep : dependencies) {
            componentInfo["dependencies"][dep] =
                impl_->componentInfos[dep]["version"];
        }

        packageLock["dependencies"][component_name] = componentInfo;
    }

    try {
        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            LOG_F(ERROR, "Failed to open file: {}", filename);
            return false;
        }
        outFile << packageLock.dump(4);
        outFile.close();
        LOG_F(INFO, "Successfully saved package.lock to {}", filename);
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Error writing package.lock: {}", e.what());
        return false;
    }
}

auto ComponentManager::printDependencyTree() {
    LOG_F(INFO, "Dependency tree:");
    for (const auto& [component, _] : impl_->components) {
        LOG_F(INFO, "{} depends on:", component);
        auto dependencies =
            impl_->dependencyGraph.getAllDependencies(component);
        for (const auto& dep : dependencies) {
            LOG_F(INFO, "  {}", dep);
        }
    }
}

auto ComponentManager::getComponentDoc(const std::string& component_name)
    -> std::string {
    std::lock_guard lock(impl_->mutex);
    if (!impl_->components.contains(component_name)) {
        LOG_F(ERROR, "Component {} is not loaded", component_name);
        return "";
    }
    return impl_->components[component_name].lock()->getDoc();
}

auto ComponentManager::compileAndLoadComponent(
    const std::string& code, const std::string& moduleName,
    const std::string& functionName) -> bool {
    if (!impl_->compiler->compileToSharedLibrary(code, moduleName,
                                                 functionName)) {
        LOG_F(ERROR, "Failed to compile component: {}", moduleName);
        return false;
    }
    // 编译成功后加载组件
    return loadComponent({{"name", moduleName}, {"entry", functionName}});
}
}  // namespace lithium
