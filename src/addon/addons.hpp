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
    static auto createShared() -> std::shared_ptr<AddonManager> {
        return std::make_shared<AddonManager>();
    }

    // -------------------------------------------------------------------
    // Module methods
    // -------------------------------------------------------------------
    auto addModule(const std::filesystem::path &path,
                   const std::string &name) -> bool;
    auto removeModule(const std::string &name) -> bool;
    auto getModule(const std::string &name) const -> json;

    auto resolveDependencies(const std::string &modName,
                             std::vector<std::string> &resolvedDeps,
                             std::vector<std::string> &missingDeps) -> bool;

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, json> modules_;
#else
    std::unordered_map<std::string, json> modules_;
#endif

    auto checkCircularDependencies(
        const std::string &modName,
        std::unordered_map<std::string, bool> &visited,
        std::unordered_map<std::string, bool> &recursionStack) const -> bool;

    auto checkMissingDependencies(const std::string &modName,
                                  std::vector<std::string> &missingDeps) const
        -> bool;
};

}  // namespace lithium
