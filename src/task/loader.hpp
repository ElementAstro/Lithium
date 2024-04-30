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
/**
 * @brief JSON文件管理器
 * @details
 * 读取、修改、保存JSON文件，支持异步操作
 *
 * JSON file manager
 * Provides asynchronous operations for reading, modifying and saving JSON files
 */
class TaskLoader {
public:
    static std::shared_ptr<TaskLoader> createShared();

    /**
     * @brief 读取JSON文件
     * @param filePath 文件路径
     * @return std::optional<json>
     * 如果文件存在且格式正确，则返回json对象；否则返回std::nullopt
     *
     * Read a JSON file
     * @param filePath Path to the file
     * @return std::optional<json> Returns a json object if the file exists and
     * is correctly formatted; otherwise returns std::nullopt
     */
    static std::optional<json> readJsonFile(const fs::path &filePath);
    static std::optional<json> readJsonFile(const std::string &filePath);

    /**
     * @brief 写入/修改JSON文件
     * @param filePath 文件路径
     * @param j 要写入的json对象
     * @return bool 成功写入返回true，否则返回false
     *
     * Write/Modify a JSON file
     * @param filePath Path to the file
     * @param j json object to write
     * @return bool Returns true if writing was successful, false otherwise
     */
    static bool writeJsonFile(const fs::path &filePath, const json &j);
    static bool writeJsonFile(const std::string &filePath, const json &j);

    /**
     * @brief 异步读取JSON文件
     * @param filePath 文件路径
     * @param callback 完成读取后的回调函数
     *
     * Asynchronously read a JSON file
     * @param filePath Path to the file
     * @param callback Callback function to be called upon completion
     */
    static void asyncReadJsonFile(
        const fs::path &filePath,
        std::function<void(std::optional<json>)> callback);
    static void asyncReadJsonFile(
        const std::string &filePath,
        std::function<void(std::optional<json>)> callback);

    /**
     * @brief 异步写入JSON文件
     * @param filePath 文件路径
     * @param j 要写入的json对象
     * @param callback 完成写入后的回调函数
     *
     * Asynchronously write a JSON file
     * @param filePath Path to the file
     * @param j json object to write
     * @param callback Callback function to be called upon completion
     */
    static void asyncWriteJsonFile(const fs::path &filePath, const json &j,
                                   std::function<void(bool)> callback);
    static void asyncWriteJsonFile(const std::string &filePath, const json &j,
                                   std::function<void(bool)> callback);

    /**
     * @brief 合并两个JSON对象
     * @param base 基础json对象，合并的结果将存储在此对象中
     * @param toMerge 要合并的json对象
     *
     * Merge two JSON objects
     * @param base Base json object, the result of the merge will be stored in
     * this object
     * @param toMerge json object to merge
     */
    static void mergeJsonObjects(json &base, const json &toMerge);

    /**
     * @brief 批量异步处理JSON文件
     * @param filePaths 文件路径集合
     * @param process 对每个文件执行的处理函数
     * @param onComplete 全部文件处理完成后的回调函数
     *
     * Batch process JSON files asynchronously
     * @param filePaths Collection of file paths
     * @param process Processing function to execute on each file
     * @param onComplete Callback function to be called after all files have
     * been processed
     */
    static void batchAsyncProcess(
        const std::vector<fs::path> &filePaths,
        std::function<void(std::optional<json>)> process,
        std::function<void()> onComplete);
    static void batchAsyncProcess(
        const std::vector<std::string> &filePaths,
        std::function<void(std::optional<json>)> process,
        std::function<void()> onComplete);

    /**
     * @brief 异步删除JSON文件
     * @param filePath 文件路径
     * @param callback 完成删除后的回调函数
     *
     * Asynchronously delete a JSON file
     * @param filePath Path to the file
     * @param callback Callback function to be called upon deletion
     */
    static void asyncDeleteJsonFile(const fs::path &filePath,
                                    std::function<void(bool)> callback);
    static void asyncDeleteJsonFile(const std::string &filePath,
                                    std::function<void(bool)> callback);

    /**
     * @brief 异步查询JSON文件中的键值
     * @param filePath 文件路径
     * @param key 要查询的键
     * @param callback 完成查询后的回调函数，返回查询到的值或std::nullopt
     *
     * Asynchronously query a value by key in a JSON file
     * @param filePath Path to the file
     * @param key Key to query
     * @param callback Callback function to be called upon completion, returning
     * the found value or std::nullopt
     */
    static void asyncQueryJsonValue(
        const fs::path &filePath, const std::string &key,
        std::function<void(std::optional<json>)> callback);
    static void asyncQueryJsonValue(
        const std::string &filePath, const std::string &key,
        std::function<void(std::optional<json>)> callback);

    /**
     * @brief 批量处理目录下的所有JSON文件
     * @param directoryPath 目录路径
     * @param process 对每个文件执行的处理函数
     * @param onComplete 全部文件处理完成后的回调函数
     *
     * Batch process all JSON files in a directory
     * @param directoryPath Path to the directory
     * @param process Processing function to execute on each file
     * @param onComplete Callback function to be called after all files in the
     * directory have been processed
     */
    static void batchProcessDirectory(
        const fs::path &directoryPath,
        std::function<void(std::optional<json>)> process,
        std::function<void()> onComplete);
    static void batchProcessDirectory(
        const std::string &directoryPath,
        std::function<void(std::optional<json>)> process,
        std::function<void()> onComplete);
};
}  // namespace lithium

#endif