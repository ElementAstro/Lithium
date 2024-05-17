/*
 * loader.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Json file manager

**************************************************/

#ifndef LITHIUM_TASK_LOADER_HPP
#define LITHIUM_TASK_LOADER_HPP

#include <filesystem>
#include <functional>
#include <optional>
#include <vector>

#include "atom/type/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace lithium {
class TaskLoader {
public:
    static std::shared_ptr<TaskLoader> createShared();

    static std::optional<json> readJsonFile(const fs::path& filePath);

    static std::optional<json> readJsonFile(const std::string& filePath);

    static bool writeJsonFile(const fs::path& filePath, const json& j);

    static bool writeJsonFile(const std::string& filePath, const json& j);

    static void asyncReadJsonFile(
        const fs::path& filePath,
        std::function<void(std::optional<json>)> callback);

    static void asyncReadJsonFile(
        const std::string& filePath,
        std::function<void(std::optional<json>)> callback);

    static void asyncWriteJsonFile(const fs::path& filePath, const json& j,
                                   std::function<void(bool)> callback);

    static void asyncWriteJsonFile(const std::string& filePath, const json& j,
                                   std::function<void(bool)> callback);

    static void mergeJsonObjects(json& base, const json& toMerge);

    static void batchAsyncProcess(
        const std::vector<fs::path>& filePaths,
        std::function<void(std::optional<json>)> process,
        std::function<void()> onComplete);

    static void batchAsyncProcess(
        const std::vector<std::string>& filePaths,
        std::function<void(std::optional<json>)> process,
        std::function<void()> onComplete);

    static void asyncDeleteJsonFile(const fs::path& filePath,
                                    std::function<void(bool)> callback);

    static void asyncDeleteJsonFile(const std::string& filePath,
                                    std::function<void(bool)> callback);

    static void asyncQueryJsonValue(
        const fs::path& filePath, const std::string& key,
        std::function<void(std::optional<json>)> callback);

    static void asyncQueryJsonValue(
        const std::string& filePath, const std::string& key,
        std::function<void(std::optional<json>)> callback);

    static void batchProcessDirectory(
        const fs::path& directoryPath,
        std::function<void(std::optional<json>)> process,
        std::function<void()> onComplete);

    static void batchProcessDirectory(
        const std::string& directoryPath,
        std::function<void(std::optional<json>)> process,
        std::function<void()> onComplete);
};
}  // namespace lithium

#endif
