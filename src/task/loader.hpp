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
#include <shared_mutex>
#include <unordered_map>
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
                              const nlohmann::json& jsonData) -> bool;

    static void asyncReadJsonFile(
        const fs::path& filePath,
        std::function<void(std::optional<nlohmann::json>)> callback);

    static void asyncWriteJsonFile(const fs::path& filePath,
                                   const nlohmann::json& jsonData,
                                   std::function<void(bool)> callback);

    static void mergeJsonObjects(nlohmann::json& base,
                                 const nlohmann::json& toMerge);

    static void deepMergeJsonObjects(nlohmann::json& base,
                                     const nlohmann::json& toMerge);

    static void batchAsyncProcess(
        const std::vector<fs::path>& filePaths,
        const std::function<void(const std::optional<nlohmann::json>&)>&
            process,
        const std::function<void()>& onComplete);

    static void asyncDeleteJsonFile(const fs::path& filePath,
                                    std::function<void(bool)> callback);

    static void asyncQueryJsonValue(
        const fs::path& filePath, const std::string& key,
        std::function<void(std::optional<nlohmann::json>)> callback);

    static void batchProcessDirectory(
        const fs::path& directoryPath,
        const std::function<void(const std::optional<nlohmann::json>&)>&
            process,
        const std::function<void()>& onComplete);

    // 新增功能：JSON模式验证
    static bool validateJson(const nlohmann::json& jsonData,
                             const nlohmann::json& schema);

private:
    static std::unordered_map<fs::path, nlohmann::json> cache_;
    static std::shared_mutex cache_mutex_;

    // 线程池相关
    static void initializeThreadPool();
    static void enqueueTask(std::function<void()> task);
};

}  // namespace lithium

#endif
