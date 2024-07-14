/*
 * addon.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Addon manager to solve the dependency problem.

**************************************************/

#include "addon.hpp"

#include <fstream>
#include <queue>
#include "atom/log/loguru.hpp"

namespace lithium {

bool AddonManager::addModule(const std::filesystem::path &path,
                             const std::string &name) {
    if (modules_.contains(name)) {
        LOG_F(ERROR, "Addon {} has already been added.", name);
        return false;
    }

    if (!std::filesystem::exists(path)) {
        LOG_F(ERROR, "Addon {} does not exist.", name);
        return false;
    }

    std::filesystem::path packagePath = path / "package.json";
    try {
        modules_[name] = json::parse(std::ifstream(packagePath));
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Addon {} package.json file does not exist or is invalid.",
              name);
        return false;
    }

    return true;
}

bool AddonManager::removeModule(const std::string &name) {
    if (modules_.erase(name)) {
        DLOG_F(INFO, "Addon {} has been removed.", name);
        return true;
    }
    LOG_F(WARNING, "Addon {} does not exist.", name);
    return false;
}

json AddonManager::getModule(const std::string &name) const {
    if (auto it = modules_.find(name); it != modules_.end()) {
        return it->second;
    }
    LOG_F(ERROR, "Addon {} does not exist.", name);
    return nullptr;
}

bool AddonManager::resolveDependencies(const std::string &modName,
                                       std::vector<std::string> &resolvedDeps,
                                       std::vector<std::string> &missingDeps) {
    if (!modules_.contains(modName)) {
        LOG_F(ERROR, "Addon {} does not exist.", modName);
        return false;
    }

    std::unordered_map<std::string, int> inDegree;
    std::queue<json> q;

    for (const auto &[name, module] : modules_) {
        inDegree[name] = 0;
        for (const auto &dep : module["dependencies"]) {
            inDegree[dep]++;
        }
    }

    const auto &mod = modules_.at(modName);
    q.push(mod);

    while (!q.empty()) {
        const auto currentMod = q.front();
        q.pop();

        resolvedDeps.push_back(currentMod["name"]);

        for (const auto &dep : currentMod["dependencies"]) {
            if (--inDegree[dep] == 0) {
                q.push(modules_.at(dep));
            }
        }
    }

    if (resolvedDeps.size() < modules_.size() ||
        !checkMissingDependencies(modName, missingDeps)) {
        return false;
    }

    return true;
}

bool AddonManager::checkMissingDependencies(
    const std::string &modName, std::vector<std::string> &missingDeps) const {
    std::unordered_map<std::string, bool> expectedDeps;
    for (const auto &dep : modules_.at(modName)["dependencies"]) {
        expectedDeps[dep] = true;
    }

    for (const auto &dep : modules_.at(modName)["dependencies"]) {
        expectedDeps.erase(dep);
    }

    for (const auto &[dep, _] : expectedDeps) {
        missingDeps.push_back(dep);
    }

    return missingDeps.empty();
}

bool AddonManager::checkCircularDependencies(
    const std::string &modName, std::unordered_map<std::string, bool> &visited,
    std::unordered_map<std::string, bool> &recursionStack) const {
    if (!visited[modName]) {
        visited[modName] = true;
        recursionStack[modName] = true;

        for (const auto &dep : modules_.at(modName)["dependencies"]) {
            if (!visited[dep] &&
                checkCircularDependencies(dep, visited, recursionStack)) {
                return true;
            } else if (recursionStack[dep]) {
                return true;
            }
        }
    }

    recursionStack[modName] = false;
    return false;
}

}  // namespace lithium
