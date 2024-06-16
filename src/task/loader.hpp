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
#include <memory>
#include <atomic>

#include "atom/type/json.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace lithium {

class TaskLoader {
public:
    static std::shared_ptr<TaskLoader> createShared();

    static std::optional<json> readJsonFile(const fs::path& filePath);

    static bool writeJsonFile(const fs::path& filePath, const json& j);

    static void asyncReadJsonFile(const fs::path& filePath, std::function<void(std::optional<json>)> callback);

    static void asyncWriteJsonFile(const fs::path& filePath, const json& j, std::function<void(bool)> callback);

    static void mergeJsonObjects(json& base, const json& toMerge);

    static void batchAsyncProcess(const std::vector<fs::path>& filePaths, std::function<void(std::optional<json>)> process, std::function<void()> onComplete);

    static void asyncDeleteJsonFile(const fs::path& filePath, std::function<void(bool)> callback);

    static void asyncQueryJsonValue(const fs::path& filePath, const std::string& key, std::function<void(std::optional<json>)> callback);

    static void batchProcessDirectory(const fs::path& directoryPath, std::function<void(std::optional<json>)> process, std::function<void()> onComplete);
};

}  // namespace lithium

#endif