#include "logger.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <curl/curl.h>
#include "spdlog/sinks/basic_file_sink.h"
#include <openssl/md5.h>

LogManager::LogManager()
{
    // 初始化 OpenSSL 库
    OpenSSL_add_all_digests();
}

// 自动扫描 logs 文件夹
json LogManager::scanLogsFolder(const std::string &folderPath) const
{
    json result;
    result["logs"] = json::array();

    for (const auto &entry : fs::directory_iterator(folderPath))
    {
        addLogToJson(entry, result);
    }

    return result;
}

// 在 json 结构中查找日志
json LogManager::searchLogInJson(const json &j, const std::string &name) const
{
    json result;
    result["logs"] = json::array();

    for (const auto &logObj : j["logs"])
    {
        const auto &logName = logObj.at("name").get<std::string>();
        if (logName.find(name) != std::string::npos)
        {
            result["logs"].push_back(logObj);
        }
    }

    return result;
}

// 解析日志，提取 error 和 warning
json LogManager::parseLog(const std::string &logFilePath) const
{
    json result;
    result["errors"] = json::array();
    result["warnings"] = json::array();

    // 创建 SPDLog 记录器
    auto fileLogger = createLogger("file_logger", logFilePath);

    // 逐行读取日志文件
    std::ifstream file(logFilePath);
    std::string line;
    while (std::getline(file, line))
    {
        // 检查每一行，提取 error 和 warning
        if (line.find("error") != std::string::npos)
        {
            result["errors"].push_back(line);
        }
        else if (line.find("warn") != std::string::npos)
        {
            result["warnings"].push_back(line);
        }

        // 记录到 SPDLog 中
        fileLogger->info(line);
    }

    return result;
}

// 计算 MD5 值
std::string LogManager::md5(const std::string &filePath) const
{
    static constexpr size_t BufferSize = 1024;

    // 打开文件
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open file " + filePath);
    }

    // 创建 MD5 上下文
    auto ctx = EVP_MD_CTX_new();
    if (!ctx)
    {
        throw std::runtime_error("Failed to create MD5 context");
    }

    initMd5Context(ctx);

    // 读取文件并更新 MD5 上下文
    char buffer[BufferSize];
    while (file.read(buffer, BufferSize))
    {
        updateMd5Context(ctx, file);
    }
    if (file.gcount() > 0)
    {
        updateMd5Context(ctx, file);
    }

    // 计算最终的 MD5 值
    unsigned char md[16];
    unsigned int mdLen = sizeof(md);
    computeMd5Hash(ctx, md, &mdLen);

    // 转换为字符串
    std::string result(mdLen * 2, '0');
    for (unsigned int i = 0; i < mdLen; ++i)
    {
        sprintf(&result[i * 2], "%02x", (unsigned int)md[i]);
    }

    // 释放资源
    cleanupMd5Context(ctx);

    return result;
}

// 上传文件
bool LogManager::uploadFile(const std::string &localFilePath, const std::string &remoteUrl) const
{
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        return false;
    }

    curl_httppost *post = NULL;
    curl_httppost *last = NULL;

    curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_FILE, localFilePath.c_str(),
                 CURLFORM_END);

    curl_easy_setopt(curl, CURLOPT_URL, remoteUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    CURLcode res = curl_easy_perform(curl);

    curl_formfree(post);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

// 添加日志到 json 结构
void LogManager::addLogToJson(const fs::directory_entry &entry, json &j) const
{
    if (entry.is_regular_file())
    {
        // 只添加 .log 扩展名的文件
        const auto &path = entry.path();
        if (path.extension() == ".log")
        {
            j["logs"].push_back({{"name", path.stem().string()}, {"path", path.string()}});
        }
    }
}

// 将日志转换为 JSON 文件
void LogManager::convertLogToJson(const std::string &logFilePath, const std::string &jsonFilePath) const
{
    json result = parseLog(logFilePath);

    std::ofstream outFile(jsonFilePath);
    outFile << std::setw(4) << result << std::endl;
}

void LogManager::run(const std::string &folderPath, const std::string &remoteUrl)
{
    // 扫描 logs 文件夹
    auto logJson = scanLogsFolder(folderPath);

    // 输出所有日志的名称和路径
    std::cout << "All logs:\n"
              << logJson.dump(2) << "\n\n";

    // 搜索日志
    const std::string searchName = "debug";
    auto searchResult = searchLogInJson(logJson, searchName);

    // 输出搜索结果
    std::cout << "Search for '" << searchName << "'\n"
              << searchResult.dump(2) << "\n\n";

    // 解析日志，并上传到远程服务器
    for (const auto &logObj : logJson["logs"])
    {
        const auto &logPath = logObj.at("path").get<std::string>();

        auto parsedLogJson = parseLog(logPath);
        std::cout << "Parsed log file: " << logPath << "\n";
        std::cout << "Errors:\n"
                  << parsedLogJson["errors"].dump(2) << "\n";
        std::cout << "Warnings:\n"
                  << parsedLogJson["warnings"].dump(2) << "\n";

        // 转换为 JSON 文件
        std::string jsonPath = logPath + ".json";
        convertLogToJson(logPath, jsonPath);

        // 计算 MD5 值
        std::string md5Value = md5(logPath);

        // 上传文件
        int retryCount = 0;
        while (retryCount < 3)
        {
            if (uploadFile(jsonPath, remoteUrl))
            {
                std::cout << "JSON file " << jsonPath << " uploaded successfully\n";
                break;
            }
            std::cout << "Failed to upload JSON file " << jsonPath << ", retrying...\n";
            retryCount++;
        }
        if (retryCount >= 3)
        {
            std::cerr << "Failed to upload JSON file " << jsonPath << ", max retry count exceeded\n";
        }

        // 删除临时文件
        fs::remove(jsonPath);

        // 输出 MD5 值
        std::cout << "MD5 of log file " << logPath << ": " << md5Value << "\n\n";
    }
}

// 创建 SPDLog 记录器
std::shared_ptr<spdlog::logger> LogManager::createLogger(const std::string &name, const std::string &filePath) const
{
    auto logger = spdlog::basic_logger_mt(name, filePath);
    logger->flush_on(spdlog::level::err);
    return logger;
}

// 初始化 MD5 上下文
void LogManager::initMd5Context(EVP_MD_CTX *ctx) const
{
    EVP_MD_CTX_init(ctx);
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
}

// 更新 MD5 上下文
void LogManager::updateMd5Context(EVP_MD_CTX *ctx, std::ifstream &file) const
{
    char buffer[1024];
    file.read(buffer, sizeof(buffer));
    EVP_DigestUpdate(ctx, buffer, file.gcount());
}

// 计算 MD5 值
void LogManager::computeMd5Hash(EVP_MD_CTX *ctx, unsigned char *md, unsigned int *mdLen) const
{
    EVP_DigestFinal_ex(ctx, md, mdLen);
}

// 清理 MD5 上下文
void LogManager::cleanupMd5Context(EVP_MD_CTX *ctx) const
{
}

/*
int main() {
    LogManager manager;

    // 扫描指定文件夹内的日志文件，返回 JSON 数据。
    json logs = manager.scanLogsFolder("/var/logs");

    // 在返回的 JSON 数据中搜索包含关键词 "error" 的日志条目。
    // 返回搜索结果数据。
    json errors = manager.searchLogInJson(logs, "error");

    // 遍历搜索结果数据，提取日志文件路径，并解析其中的错误信息。
    for (auto& entry : errors.items()) {
        std::string logFilePath = entry.value()["log_file_path"];
        json logErrors = manager.parseLog(logFilePath);
        // ...处理解析结果...
    }

    return 0;
}
*/

