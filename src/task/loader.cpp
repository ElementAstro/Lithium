/*
 * loader.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: Json file manager

**************************************************/

#include "loader.hpp"

#include <atomic>
#include <fstream>
#include <thread>

#include "atom/log/loguru.hpp"

namespace lithium {
std::shared_ptr<TaskLoader> TaskLoader::createShared() {
    return std::make_shared<TaskLoader>();
}

std::optional<json> TaskLoader::readJsonFile(const fs::path& filePath) {
    try {
        if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
            LOG_F(ERROR, "File not found: {}", filePath.string());
            return std::nullopt;
        }

        std::ifstream inputFile(filePath);
        json j = json::parse(inputFile, nullptr, false);

        if (j.is_discarded()) {
            LOG_F(ERROR, "Parse error in file: {}", filePath.string());
            return std::nullopt;
        }

        return j;
    } catch (const json::exception& e) {
        LOG_F(ERROR, "JSON exception in file {}: {}", filePath.string(),
              e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Standard exception in file {}: {}", filePath.string(),
              e.what());
    }
    return std::nullopt;
}

std::optional<json> TaskLoader::readJsonFile(const std::string& filePath) {
    return readJsonFile(fs::path(filePath));
}

bool TaskLoader::writeJsonFile(const fs::path& filePath, const json& j) {
    try {
        std::ofstream outputFile(filePath);
        outputFile << j.dump(4);
        return true;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Failed to write JSON to {}: {}", filePath.string(),
              e.what());
        return false;
    }
}

bool TaskLoader::writeJsonFile(const std::string& filePath, const json& j) {
    return writeJsonFile(fs::path(filePath), j);
}

void TaskLoader::asyncReadJsonFile(
    const fs::path& filePath,
    std::function<void(std::optional<json>)> callback) {
    std::jthread([filePath, callback = std::move(callback)]() {
        auto result = readJsonFile(filePath);
        callback(result);
    });
}

void TaskLoader::asyncReadJsonFile(
    const std::string& filePath,
    std::function<void(std::optional<json>)> callback) {
    asyncReadJsonFile(fs::path(filePath), callback);
}

void TaskLoader::asyncWriteJsonFile(const fs::path& filePath, const json& j,
                                    std::function<void(bool)> callback) {
    std::jthread([filePath, j, callback = std::move(callback)]() {
        bool success = writeJsonFile(filePath, j);
        callback(success);
    });
}

void TaskLoader::asyncWriteJsonFile(const std::string& filePath, const json& j,
                                    std::function<void(bool)> callback) {
    asyncWriteJsonFile(fs::path(filePath), j, callback);
}

void TaskLoader::mergeJsonObjects(json& base, const json& toMerge) {
    for (auto& [key, value] : toMerge.items()) {
        base[key] = value;
    }
}

void TaskLoader::batchAsyncProcess(
    const std::vector<fs::path>& filePaths,
    std::function<void(std::optional<json>)> process,
    std::function<void()> onComplete) {
    std::atomic<int> filesProcessed = 0;
    int totalFiles = filePaths.size();

    for (const auto& path : filePaths) {
        asyncReadJsonFile(path, [&filesProcessed, &totalFiles, process,
                                 onComplete](std::optional<json> j) {
            process(j);
            if (++filesProcessed == totalFiles) {
                onComplete();
            }
        });
    }
}

void TaskLoader::batchAsyncProcess(
    const std::vector<std::string>& filePaths,
    std::function<void(std::optional<json>)> process,
    std::function<void()> onComplete) {
    std::vector<fs::path> paths;
    for (const auto& path : filePaths) {
        paths.push_back(fs::path(path));
    }
    batchAsyncProcess(paths, process, onComplete);
}

void TaskLoader::asyncDeleteJsonFile(const fs::path& filePath,
                                     std::function<void(bool)> callback) {
    std::jthread([filePath, callback = std::move(callback)]() {
        bool success = fs::remove(filePath);
        callback(success);
    });
}

void TaskLoader::asyncDeleteJsonFile(const std::string& filePath,
                                     std::function<void(bool)> callback) {
    asyncDeleteJsonFile(fs::path(filePath), callback);
}

void TaskLoader::asyncQueryJsonValue(
    const fs::path& filePath, const std::string& key,
    std::function<void(std::optional<json>)> callback) {
    asyncReadJsonFile(filePath, [key, callback = std::move(callback)](
                                    std::optional<json> jOpt) {
        if (!jOpt.has_value()) {
            callback(std::nullopt);
            return;
        }
        const json& j = jOpt.value();
        if (j.contains(key)) {
            callback(j[key]);
        } else {
            callback(std::nullopt);
        }
    });
}

void TaskLoader::asyncQueryJsonValue(
    const std::string& filePath, const std::string& key,
    std::function<void(std::optional<json>)> callback) {
    asyncQueryJsonValue(fs::path(filePath), key, callback);
}

void TaskLoader::batchProcessDirectory(
    const fs::path& directoryPath,
    std::function<void(std::optional<json>)> process,
    std::function<void()> onComplete) {
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath)) {
        LOG_F(ERROR, "Invalid directory path: {}", directoryPath.string());
        return;
    }

    std::vector<fs::path> filePaths;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (entry.path().extension() == ".json") {
            filePaths.push_back(entry.path());
        }
    }

    batchAsyncProcess(filePaths, process, onComplete);
}

void TaskLoader::batchProcessDirectory(
    const std::string& directoryPath,
    std::function<void(std::optional<json>)> process,
    std::function<void()> onComplete) {
    batchProcessDirectory(fs::path(directoryPath), process, onComplete);
}
}  // namespace lithium
