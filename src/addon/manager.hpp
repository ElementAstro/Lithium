/*
 * component_manager.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Component Manager (the core of the plugin system)

**************************************************/

#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "dependency.hpp"
#include "atom/components/component.hpp"
#include "atom/components/types.hpp"
#include "atom/system/env.hpp"
#include "atom/type/args.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace lithium {

class AddonManager;
class Compiler;
class ModuleLoader;
class Sandbox;

class ComponentEntry {
public:
    std::string name;
    std::string func_name;
    std::string component_type;
    std::string module_name;
    std::vector<std::string> dependencies;

    ComponentEntry(std::string name, std::string func_name, std::string component_type, std::string module_name)
        : name(std::move(name)), func_name(std::move(func_name)), component_type(std::move(component_type)), module_name(std::move(module_name)) {}
};

class ComponentManager {
public:
    explicit ComponentManager();
    ~ComponentManager();

    bool Initialize();
    bool Destroy();

    static std::shared_ptr<ComponentManager> createShared();

    bool loadComponent(ComponentType component_type, const json& params);
    bool unloadComponent(ComponentType component_type, const json& params);
    bool reloadComponent(ComponentType component_type, const json& params);
    bool reloadAllComponents();

    std::optional<std::weak_ptr<Component>> getComponent(const std::string& component_name);
    std::optional<json> getComponentInfo(const std::string& component_name);
    std::vector<std::string> getComponentList();

private:
    std::vector<std::string> getFilesInDir(const std::string& path);
    std::vector<std::string> getQualifiedSubDirs(const std::string& path);
    bool checkComponent(const std::string& module_name, const std::string& module_path);
    bool loadComponentInfo(const std::string& module_path, const std::string& component_name);
    bool checkComponentInfo(const std::string& module_name, const std::string& component_name);
    bool loadSharedComponent(const std::string& component_name, const std::string& addon_name, const std::string& module_path, const std::string& entry, const std::vector<std::string>& dependencies);
    bool unloadSharedComponent(const std::string& component_name, bool forced);
    bool reloadSharedComponent(const std::string& component_name);

    std::weak_ptr<ModuleLoader> module_loader_;
    std::weak_ptr<atom::utils::Env> env_;
    std::unique_ptr<Sandbox> sandbox_;
    std::unique_ptr<Compiler> compiler_;
    std::weak_ptr<AddonManager> addon_manager_;
    std::unordered_map<std::string, std::shared_ptr<ComponentEntry>> component_entries_;
    std::unordered_map<std::string, json> component_infos_;
    std::unordered_map<std::string, std::weak_ptr<Component>> components_;
    std::string module_path_;
    DependencyGraph dependency_graph_;
};

}  // namespace lithium
