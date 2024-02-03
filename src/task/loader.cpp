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

#include <fstream>
#include <atomic>
#include <thread>

#include "atom/log/loguru.hpp"

std::optional<json> JsonFileManager::readJsonFile(const fs::path &filePath)
{
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath))
    {
        LOG_F(ERROR, "File not found: {}", filePath.string());
        return std::nullopt;
    }

    std::ifstream inputFile(filePath);
    json j;
    try
    {
        inputFile >> j;
    }
    catch (const json::parse_error &e)
    {
        LOG_F(ERROR, "Parse error in {}: {}", filePath.string(), e.what());
        return std::nullopt;
    }
    catch (const json::type_error &e)
    {
        LOG_F(ERROR, "Type error in {}: {}", filePath.string(), e.what());
        return std::nullopt;
    }
    catch (const std::exception &e)
    {
        LOG_F(ERROR, "Exception in {}: {}", filePath.string(), e.what());
        return std::nullopt;
    }
    inputFile.close();
    return j;
}

static std::optional<json> JsonFileManager::readJsonFile(const std::string &filePath)
{
    return readJsonFile(fs::path(filePath));
}

bool JsonFileManager::writeJsonFile(const fs::path &filePath, const json &j)
{
    std::ofstream outputFile(filePath);
    if (!outputFile.is_open())
    {
        LOG_F(ERROR, "Failed to open: {}", filePath.string());
        return false;
    }
    outputFile << j.dump(4);
    outputFile.close();
    return true;
}

bool JsonFileManager::writeJsonFile(const std::string &filePath, const json &j)
{
    return writeJsonFile(fs::path(filePath), j);
}

void JsonFileManager::asyncReadJsonFile(const fs::path &filePath, std::function<void(std::optional<json>)> callback)
{
    std::jthread([filePath, callback = std::move(callback)]()
                 {
            auto j = readJsonFile(filePath);
            callback(j); });
}

void JsonFileManager::asyncReadJsonFile(const std::string &filePath, std::function<void(std::optional<json>)> callback)
{
    asyncReadJsonFile(fs::path(filePath), callback);
}

void JsonFileManager::asyncWriteJsonFile(const fs::path &filePath, const json &j, std::function<void(bool)> callback)
{
    std::jthread([filePath, j, callback = std::move(callback)]() mutable
                 {
            bool success = writeJsonFile(filePath, j);
            callback(success); });
}

void JsonFileManager::asyncWriteJsonFile(const std::string &filePath, const json &j, std::function<void(bool)> callback)
{
    asyncWriteJsonFile(fs::path(filePath), j, callback);
}

void JsonFileManager::mergeJsonObjects(json &base, const json &toMerge)
{
    for (auto &[key, value] : toMerge.items())
    {
        base[key] = value;
    }
}

void JsonFileManager::batchAsyncProcess(const std::vector<fs::path> &filePaths, std::function<void(std::optional<json>)> process, std::function<void()> onComplete)
{
    std::atomic<int> filesProcessed = 0;
    for (const auto &path : filePaths)
    {
        asyncReadJsonFile(path, [&filesProcessed, &filePaths, process, onComplete](std::optional<json> j)
                          {
                if (j) process(j);
                if (++filesProcessed == filePaths.size()) onComplete(); });
    }
}

void JsonFileManager::batchAsyncProcess(const std::vector<std::string> &filePaths, std::function<void(std::optional<json>)> process, std::function<void()> onComplete)
{
    std::vector<fs::path> paths;
    for (const auto &path : filePaths)
    {
        paths.push_back(fs::path(path));
    }
    batchAsyncProcess(paths, process, onComplete);
}

void JsonFileManager::asyncDeleteJsonFile(const fs::path &filePath, std::function<void(bool)> callback)
{
    std::jthread([filePath, callback = std::move(callback)]()
                 {
            bool success = fs::remove(filePath);
            callback(success); });
}

void JsonFileManager::asyncDeleteJsonFile(const std::string &filePath, std::function<void(bool)> callback)
{
    asyncDeleteJsonFile(fs::path(filePath), callback);
}

void JsonFileManager::asyncQueryJsonValue(const fs::path &filePath, const std::string &key, std::function<void(std::optional<json>)> callback)
{
    asyncReadJsonFile(filePath, [key, callback = std::move(callback)](std::optional<json> jOpt)
                      {
            if (!jOpt.has_value()) {
                callback(std::nullopt);
                return;
            }
            const json& j = jOpt.value();
            if (j.contains(key)) {
                callback(j[key]);
            } else {
                callback(std::nullopt);
            } });
}

void JsonFileManager::asyncQueryJsonValue(const std::string &filePath, const std::string &key, std::function<void(std::optional<json>)> callback)
{
    asyncQueryJsonValue(fs::path(filePath), key, callback);
}

void JsonFileManager::batchProcessDirectory(const fs::path &directoryPath, std::function<void(std::optional<json>)> process, std::function<void()> onComplete)
{
    if (!fs::exists(directoryPath) || !fs::is_directory(directoryPath))
    {
        LOG_F(ERROR, "Invalid directory path: {}", directoryPath.string());
        return;
    }

    std::vector<fs::path> filePaths;
    for (const auto &entry : fs::directory_iterator(directoryPath))
    {
        if (entry.path().extension() == ".json")
        {
            filePaths.push_back(entry.path());
        }
    }

    batchAsyncProcess(filePaths, process, onComplete);
}

void JsonFileManager::batchProcessDirectory(const std::string &directoryPath, std::function<void(std::optional<json>)> process, std::function<void()> onComplete)
{
    batchProcessDirectory(fs::path(directoryPath), process, onComplete);
}

/*
int main()
{
    // 设置要操作的文件路径
    std::filesystem::path filePath1 = "file1.json";
    std::filesystem::path filePath2 = "file2.json";

    // 异步读取、修改、保存第一个JSON文件
    JsonFileManager::asyncReadJsonFile(filePath1, [filePath1](std::optional<json> jOpt)
                                       {
        if (!jOpt) {
            std::cerr << "Failed to read file: " << filePath1 << std::endl;
            return;
        }

        auto& j = jOpt.value();
        std::cout << "Original content of file1.json:\n" << j.dump(4) << std::endl;

        // 修改操作
        j["newKey"] = "newValue";

        // 异步保存修改
        JsonFileManager::asyncWriteJsonFile(filePath1, j, [](bool success) {
            if (success) {
                std::cout << "file1.json saved successfully." << std::endl;
            } else {
                std::cerr << "Failed to save file1.json." << std::endl;
            }
        }); });

    // 批量异步处理多个JSON文件
    std::vector<std::filesystem::path> files = {filePath1, filePath2};
    JsonFileManager::batchAsyncProcess(
        files,
        [](std::optional<json> jOpt)
        {
            if (!jOpt)
            {
                std::cerr << "Failed to read a file." << std::endl;
                return;
            }

            const auto &j = jOpt.value();
            std::cout << "Processing a JSON file:\n"
                      << j.dump(4) << std::endl;
            // 这里可以添加更多处理逻辑
        },
        []()
        {
            std::cout << "All files processed." << std::endl;
        });

    // 主线程等待，因为std::jthread会在析构时自动join
    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}
*/