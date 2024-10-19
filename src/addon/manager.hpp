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
#include <vector>

#include "atom/components/component.hpp"

#include "dependency.hpp"

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium {
class ComponentManagerImpl;
class ComponentManager {
public:
    explicit ComponentManager();
    ~ComponentManager();

    auto initialize() -> bool;
    auto destroy() -> bool;

    static auto createShared() -> std::shared_ptr<ComponentManager>;

    auto loadComponent(const json& params) -> bool;
    auto unloadComponent(const json& params) -> bool;
    auto reloadComponent(const json& params) -> bool;
    auto reloadAllComponents() -> bool;

    auto scanComponents(const std::string& path) -> std::vector<std::string>;

    auto getComponent(const std::string& component_name)
        -> std::optional<std::weak_ptr<Component>>;

    auto getComponentInfo(const std::string& component_name)
        -> std::optional<json>;
    auto getComponentList() -> std::vector<std::string>;

    auto getComponentDoc(const std::string& component_name) -> std::string;

    auto hasComponent(const std::string& component_name) -> bool;

    auto savePackageLock(const std::string& filename) -> bool;
    auto printDependencyTree();

private:
    auto getFilesInDir(const std::string& path) -> std::vector<std::string>;
    auto getQualifiedSubDirs(const std::string& path)
        -> std::vector<std::string>;
    auto checkComponent(const std::string& module_name,
                        const std::string& module_path) -> bool;
    auto loadComponentInfo(const std::string& module_path,
                           const std::string& component_name) -> bool;
    auto checkComponentInfo(const std::string& module_name,
                            const std::string& component_name) -> bool;
    auto loadSharedComponent(
        const std::string& component_name, const std::string& addon_name,
        const std::string& module_path, const std::string& entry,
        const std::vector<std::string>& dependencies) -> bool;
    auto unloadSharedComponent(const std::string& component_name,
                               bool forced) -> bool;
    auto reloadSharedComponent(const std::string& component_name) -> bool;

    auto loadStandaloneComponent(
        const std::string& component_name, const std::string& addon_name,
        const std::string& module_path, const std::string& entry,
        const std::vector<std::string>& dependencies) -> bool;
    auto unloadStandaloneComponent(const std::string& component_name,
                                   bool forced) -> bool;
    auto reloadStandaloneComponent(const std::string& component_name) -> bool;

    void updateDependencyGraph(
        const std::string& component_name, const std::string& version,
        const std::vector<std::string>& dependencies,
        const std::vector<std::string>& dependencies_version);

    std::unique_ptr<ComponentManagerImpl> impl_;
};

}  // namespace lithium
