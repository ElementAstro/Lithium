/**
 * @file loader.cpp
 * @brief JSON File Manager
 *
 * This file provides functionality for managing JSON files, including
 * loading, parsing, and possibly manipulating JSON data.
 *
 * @date 2023-04-03
 * @autor Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#include "loader.hpp"

#include <atomic>
#include <fstream>
#include <thread>
#include <utility>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium {

auto TaskLoader::createShared() -> std::shared_ptr<TaskLoader> {
    return std::make_shared<TaskLoader>();
}

auto TaskLoader::readJsonFile(const fs::path& filePath) -> std::optional<json> {
    try {
        if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
            LOG_F(ERROR, "File not found: {}", filePath.string());
            return std::nullopt;
        }

        std::ifstream inputFile(filePath);
        json jsonData = json::parse(inputFile, nullptr, false);

        if (jsonData.is_discarded()) {
            LOG_F(ERROR, "Parse error in file: {}", filePath.string());
            return std::nullopt;
        }

        return jsonData;
    } catch (const json::exception& e) {
        LOG_F(ERROR, "JSON exception in file {}: {}", filePath.string(),
              e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Standard exception in file {}: {}", filePath.string(),
              e.what());
    }
    return std::nullopt;
}

auto TaskLoader::writeJsonFile(const fs::path& filePath,
                               const json& jsonData) -> bool {
    try {
        std::ofstream outputFile(filePath);
        outputFile << jsonData.dump(4);
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to write JSON to {}: {}", filePath.string(),
              e.what());
        return false;
    }
}

void TaskLoader::asyncReadJsonFile(
    const fs::path& filePath,
    std::function<void(std::optional<json>)> callback) {
    std::jthread([filePath, callback = std::move(callback)]() {
        auto result = readJsonFile(filePath);
        callback(result);
    });
}

void TaskLoader::asyncWriteJsonFile(const fs::path& filePath,
                                    const json& jsonData,
                                    std::function<void(bool)> callback) {
    std::jthread([filePath, jsonData, callback = std::move(callback)]() {
        bool success = writeJsonFile(filePath, jsonData);
        callback(success);
    });
}

void TaskLoader::mergeJsonObjects(json& base, const json& toMerge) {
#pragma unroll
    for (const auto& [key, value] : toMerge.items()) {
        base[key] = value;
    }
}

void TaskLoader::batchAsyncProcess(
    const std::vector<fs::path>& filePaths,
    const std::function<void(const std::optional<json>&)>& process,
    const std::function<void()>& onComplete) {
    std::atomic<int> filesProcessed = 0;
    int totalFiles = static_cast<int>(filePaths.size());

#pragma unroll
    for (const auto& path : filePaths) {
        asyncReadJsonFile(path,
                          [&filesProcessed, totalFiles, &process,
                           &onComplete](const std::optional<json>& jsonData) {
                              process(jsonData);
                              if (++filesProcessed == totalFiles) {
                                  onComplete();
                              }
                          });
    }
}

void TaskLoader::asyncDeleteJsonFile(const fs::path& filePath,
                                     std::function<void(bool)> callback) {
    std::jthread([filePath, callback = std::move(callback)]() {
        bool success = fs::remove(filePath);
        callback(success);
    });
}

void TaskLoader::asyncQueryJsonValue(
    const fs::path& filePath, const std::string& key,
    std::function<void(std::optional<json>)> callback) {
    asyncReadJsonFile(filePath, [key, callback = std::move(callback)](
                                    const std::optional<json>& jsonOpt) {
        if (!jsonOpt.has_value()) {
            callback(std::nullopt);
            return;
        }
        const json& jsonData = jsonOpt.value();
        if (jsonData.contains(key)) {
            callback(jsonData[key]);
        } else {
            callback(std::nullopt);
        }
    });
}

void TaskLoader::batchProcessDirectory(
    const fs::path& directoryPath,
    const std::function<void(const std::optional<json>&)>& process,
    const std::function<void()>& onComplete) {
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        LOG_F(ERROR, "Invalid directory path: {}", directoryPath.string());
        return;
    }

    std::vector<fs::path> filePaths;
#pragma unroll
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".json") {
            filePaths.push_back(entry.path());
        }
    }

    batchAsyncProcess(filePaths, std::move(process), std::move(onComplete));
}

}  // namespace lithium
