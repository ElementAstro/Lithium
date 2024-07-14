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

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>
#include "atom/type/json.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#endif

using json = nlohmann::json;

namespace lithium {

/**
 * @brief This class manages the dependencies of modules.
 * @note This class does not contain any module's shared_ptr or unique_ptr.
 */
class AddonManager {
public:
    /**
     * @brief Construct a new Addon Manager object.
     */
    explicit AddonManager() = default;

    /**
     * @brief Destroy the Addon Manager object.
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
    bool addModule(const std::filesystem::path &path, const std::string &name);
    bool removeModule(const std::string &name);
    json getModule(const std::string &name) const;

    bool resolveDependencies(const std::string &modName,
                             std::vector<std::string> &resolvedDeps,
                             std::vector<std::string> &missingDeps);

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, json> modules_;
#else
    std::unordered_map<std::string, json> modules_;
#endif

    bool checkCircularDependencies(
        const std::string &modName,
        std::unordered_map<std::string, bool> &visited,
        std::unordered_map<std::string, bool> &recursionStack) const;

    bool checkMissingDependencies(const std::string &modName,
                                  std::vector<std::string> &missingDeps) const;
};

}  // namespace lithium
