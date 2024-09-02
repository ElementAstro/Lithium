/**
 * @file loader.hpp
 * @brief JSON File Manager
 *
 * This file provides functionality for managing JSON files, including
 * loading, parsing, and possibly manipulating JSON data.
 *
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#ifndef LITHIUM_TASK_LOADER_HPP
#define LITHIUM_TASK_LOADER_HPP

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "atom/type/json_fwd.hpp"

namespace fs = std::filesystem;

namespace lithium {

class TaskLoader {
public:
    static auto createShared() -> std::shared_ptr<TaskLoader>;

    static auto readJsonFile(const fs::path& filePath)
        -> std::optional<nlohmann::json>;

    static auto writeJsonFile(const fs::path& filePath,
                              const nlohmann::json& j) -> bool;

    static void asyncReadJsonFile(
        const fs::path& filePath,
        std::function<void(std::optional<nlohmann::json>)> callback);

    static void asyncWriteJsonFile(const fs::path& filePath,
                                   const nlohmann::json& j,
                                   std::function<void(bool)> callback);

    static void mergeJsonObjects(nlohmann::json& base,
                                 const nlohmann::json& toMerge);

    static void batchAsyncProcess(
        const std::vector<fs::path>& filePaths,
        std::function<void(std::optional<nlohmann::json>)> process,
        std::function<void()> onComplete);

    static void asyncDeleteJsonFile(const fs::path& filePath,
                                    std::function<void(bool)> callback);

    static void asyncQueryJsonValue(
        const fs::path& filePath, const std::string& key,
        std::function<void(std::optional<nlohmann::json>)> callback);

    static void batchProcessDirectory(
        const fs::path& directoryPath,
        std::function<void(std::optional<nlohmann::json>)> process,
        std::function<void()> onComplete);
};

}  // namespace lithium

#endif
