#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <openssl/evp.h>
#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

/**
 * @brief 日志管理器类，用于扫描、解析和上传日志文件。
 * 
 */
class LogManager {
public:
    /**
     * @brief 构造函数，初始化 OpenSSL 库。
     * 
     */
    LogManager();

    /**
     * @brief 自动扫描指定文件夹内的日志文件，返回一个 JSON 结构体。
     * 
     * @param folderPath 文件夹路径。
     * @return json JSON 结构体对象。
     */
    json scanLogsFolder(const std::string& folderPath) const;

    /**
     * @brief 在给定的 JSON 结构体中查找包含关键词 name 的日志，并返回一个新的 JSON 结构体。
     * 
     * @param j 输入的 JSON 结构体。
     * @param name 关键词。
     * @return json 包含所有符合条件的日志条目的 JSON 结构体。
     */
    json searchLogInJson(const json& j, const std::string& name) const;

    /**
     * @brief 解析指定路径的日志文件，提取其中的 error 和 warning 信息，并返回一个 JSON 结构体。
     * 
     * @param logFilePath 日志文件路径。
     * @return json 包含所有 error 和 warning 的 JSON 结构体。
     */
    json parseLog(const std::string& logFilePath) const;
    
    /**
     * @brief 计算文件的 MD5 值。
     * 
     * @param filePath 文件路径。
     * @return std::string 计算出的 MD5 值，以字符串形式返回。
     */
    std::string md5(const std::string& filePath) const;

    /**
     * @brief 上传本地文件到指定的 URL。
     * 
     * @param localFilePath 本地文件路径。
     * @param remoteUrl 目标 URL。
     * @return true 上传成功。
     * @return false 上传失败。
     */
    bool uploadFile(const std::string& localFilePath, const std::string& remoteUrl) const;

    /**
     * @brief 将指定的日志文件添加到 JSON 结构体中。
     * 
     * @param entry 日志文件的目录项。
     * @param j JSON 结构体对象。
     */
    void addLogToJson(const fs::directory_entry& entry, json& j) const;

    /**
     * @brief 将日志文件转换为 JSON 文件。
     * 
     * @param logFilePath 日志文件路径。
     * @param jsonFilePath 转换后的 JSON 文件路径。
     */
    void convertLogToJson(const std::string& logFilePath, const std::string& jsonFilePath) const;

    /**
     * @brief 执行日志管理器的主要任务流程。
     * 
     * @param folderPath 需要处理的文件夹路径。
     * @param remoteUrl 远程服务器的 URL。
     */
    void run(const std::string& folderPath, const std::string& remoteUrl);

private:
    /**
     * @brief 创建一个 SPDLog 记录器。
     * 
     * @param name 记录器名称。
     * @param filename 输出文件的路径和文件名。
     * @return std::shared_ptr<spdlog::logger> SPDLog 记录器指针。
     */
    std::shared_ptr<spdlog::logger> createLogger(const std::string& name, const std::string& filename) const;

    /**
     * @brief 初始化 MD5 上下文，设置算法和初始值。
     * 
     * @param ctx MD5 上下文指针。
     */
    void initMd5Context(EVP_MD_CTX* ctx) const;

    /**
     * @brief 释放 MD5 上下文，清空资源。
     * 
     * @param ctx MD5 上下文指针。
     */
    void cleanupMd5Context(EVP_MD_CTX* ctx) const;

    /**
     * @brief 逐段读取文件内容，更新 MD5 上下文。
     * 
     * @param ctx MD5 上下文指针。
     * @param file 文件对象。
     */
    void updateMd5Context(EVP_MD_CTX* ctx, std::ifstream& file) const;

    /**
     * @brief 计算文件 MD5 值。
     * 
     * @param ctx MD5 上下文指针。
     * @param md 存储计算结果的缓冲区。
     * @param mdLen 结果缓冲区的大小。
     */
    void computeMd5Hash(EVP_MD_CTX* ctx, unsigned char* md, unsigned int* mdLen) const;
};