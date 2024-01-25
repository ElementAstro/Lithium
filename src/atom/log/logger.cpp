/*
 * log_manager.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-30

Description: Custom Logger Manager

**************************************************/

#include "logger.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <map>
#include <iomanip>
#include <future>

#include <openssl/evp.h>

#include "atom/web/httplib.h"
#include "atom/log/loguru.hpp"

namespace Lithium
{
    void LoggerManager::scanLogsFolder(const std::string &folderPath)
    {
        for (const auto &entry : std::filesystem::directory_iterator(folderPath))
        {
            DLOG_F(INFO, "Scanning log file: {}", entry.path().generic_string());
            if (entry.is_regular_file())
            {
                parseLog(entry.path().string());
            }
        }
    }

    std::vector<LogEntry> LoggerManager::searchLogs(const std::string &keyword)
    {
        std::vector<LogEntry> searchResults;
        for (const auto &logEntry : logEntries)
        {
            if (logEntry.message.find(keyword) != std::string::npos)
            {
                searchResults.push_back(logEntry);
            }
        }
        return searchResults;
    }

    void LoggerManager::parseLog(const std::string &filePath)
    {
        std::ifstream logFile(filePath);
        if (logFile.is_open())
        {
            std::string line;
            int lineNumber = 1;
            while (std::getline(logFile, line))
            {
                LogEntry logEntry;
                logEntry.fileName = filePath;
                logEntry.lineNumber = lineNumber++;
                logEntry.message = line;
                logEntries.push_back(logEntry);
            }
            logFile.close();
        }
    }

    void LoggerManager::uploadFile(const std::string &filePath)
    {
        httplib::Client client("https://lightapt.com"); // 替换为服务器地址
        auto res = client.Post("/upload", filePath.c_str(), "application/octet-stream");
        if (res && res->status == 200)
        {
            DLOG_F(INFO, "File uploaded successfully");
        }
        else
        {
            LOG_F(ERROR, "Failed to upload file");
        }
    }

    std::vector<std::string> LoggerManager::extractErrorMessages()
    {
        std::vector<std::string> errorMessages;
        for (const auto &logEntry : logEntries)
        {
            if (logEntry.message.find("[ERROR]") != std::string::npos)
            {
                errorMessages.push_back(logEntry.message);
                DLOG_F(INFO, "{}", logEntry.message);
            }
        }
        return errorMessages;
    }

    void LoggerManager::analyzeLogs()
    {
        std::vector<std::string> errorMessages = extractErrorMessages();

        if (errorMessages.empty())
        {
            DLOG_F(INFO, "No errors found in the logs.");
            return;
        }
        DLOG_F(INFO, "Analyzing logs...");

        // 统计错误类型
        std::map<std::string, int> errorTypeCount;
        for (const auto &errorMessage : errorMessages)
        {
            std::string errorType = getErrorType(errorMessage);
            errorTypeCount[errorType]++;
        }
        DLOG_F(INFO, "Error Type Count:");
        for (const auto &[errorType, count] : errorTypeCount)
        {
            DLOG_F(INFO, "{} : {}", errorType, count);
        }

        // 查找最常见的错误消息
        std::string mostCommonErrorMessage = getMostCommonErrorMessage(errorMessages);
        DLOG_F(INFO, "Most Common Error Message: {}", mostCommonErrorMessage);
    }

    std::string computeMd5Hash(const std::string &filePath)
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file)
        {
            return "";
        }

        EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
        const EVP_MD *md = EVP_md5(); // 获取 MD5 哈希算法对象
        unsigned char digest[EVP_MAX_MD_SIZE];
        unsigned int digestLen;

        EVP_DigestInit_ex(mdctx, md, NULL);

        constexpr size_t bufferSize = 4096;
        char buffer[bufferSize];
        while (file.read(buffer, bufferSize))
        {
            EVP_DigestUpdate(mdctx, buffer, bufferSize);
        }
        EVP_DigestUpdate(mdctx, buffer, file.gcount());

        EVP_DigestFinal_ex(mdctx, digest, &digestLen);
        EVP_MD_CTX_free(mdctx);

        std::stringstream ss;
        for (unsigned int i = 0; i < digestLen; i++)
        {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
        }

        return ss.str();
    }

    std::string LoggerManager::getErrorType(const std::string &errorMessage)
    {
        std::size_t startPos = errorMessage.find("[");
        std::size_t endPos = errorMessage.find("]");
        if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos)
        {
            return errorMessage.substr(startPos + 1, endPos - startPos - 1);
        }
        return "Unknown";
    }

    std::string LoggerManager::getMostCommonErrorMessage(const std::vector<std::string> &errorMessages)
    {
        std::map<std::string, int> errorMessageCount;
        for (const auto &errorMessage : errorMessages)
        {
            errorMessageCount[errorMessage]++;
        }
        std::string mostCommonErrorMessage;
        int maxCount = 0;
        for (const auto &[errorMessage, count] : errorMessageCount)
        {
            if (count > maxCount)
            {
                mostCommonErrorMessage = errorMessage;
                maxCount = count;
            }
        }
        return mostCommonErrorMessage;
    }
}
