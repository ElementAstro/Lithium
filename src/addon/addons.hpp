/*
 * addon.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Addon manager to solve the dependency problem.

**************************************************/

#pragma once

#include <string>
#include <vector>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {
/**
 * @brief This is the class which contains the dependencies relationships of the
 * modules.
 * @note This class will not contains any module's shared_ptr or unique_ptr.
 * @note So, the AddonManager is not same as the moduleManager.
 */
class AddonManager {
public:
    /**
     * @brief Construct a new Module Manager object.
     */
    explicit AddonManager() = default;

    /**
     * @brief Destroy the Module Manager object.
     */
    ~AddonManager() = default;

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    static std::shared_ptr<AddonManager> createShared() {
        return std::make_shared<AddonManager>();
    }

    // -------------------------------------------------------------------
    // Module methods
    // -------------------------------------------------------------------

    /**
     * @brief Add a module to the Module manager.
     * @param path The path of the addon.
     * @note The name of the module must be unique.
     * @note The name of the module must be the same as the name of the module
     * in the Module.json.
     */
    bool addModule(const std::filesystem::path &path, const std::string &name);

    /**
     * @brief Remove a module from the Module manager.
     * @param name The name of the module.
     * @note If there is no module with the name, this function will do nothing.
     */
    bool removeModule(const std::string &name);

    /**
     * @brief Get a module from the Module manager.
     * @param name The name of the module.
     * @return The module.
     * @note If there is no module with the name, this function will return
     * nullptr.
     */
    json getModule(const std::string &name);

    /**
     * @brief Resolve the dependencies of the module.
     * @param modName The name of the module.
     * @param resolvedDeps The vector of resolved dependencies.
     * @param missingDeps The vector of missing dependencies.
     * @return True if the dependencies are resolved successfully, otherwise
     * false.
     * @note If there is no module with the name, this function will return
     * false.
     */
    bool resolveDependencies(const std::string &modName,
                             std::vector<std::string> &resolvedDeps,
                             std::vector<std::string> &missingDeps);

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, json> m_modules;
#else
    std::unordered_map<std::string, json> m_modules;
#endif

    /**
     * @brief Check the circular dependencies of the module.
     * @param modName The name of the module.
     * @param visited The map of visited modules.
     * @param recursionStack The map of recursion stack.
     * @return True if the dependencies are resolved successfully, otherwise
     * false.
     * @note If there is no module with the name, this function will return
     * false.
     */
    bool checkCircularDependencies(
        const std::string &modName,
        std::unordered_map<std::string, bool> &visited,
        std::unordered_map<std::string, bool> &recursionStack);

    /**
     * @brief Check the missing dependencies of the module.
     * @param modName The name of the module.
     * @param missingDeps The vector of missing dependencies.
     * @return True if the dependencies are resolved successfully, otherwise
     * false.
     * @note If there is no module with the name, this function will return
     * false.
     */
    bool checkMissingDependencies(const std::string &modName,
                                  std::vector<std::string> &missingDeps);
};
}  // namespace lithium
