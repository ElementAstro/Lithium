/*
 * log_manager.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-6-30

Description: Custom Logger Manager

**************************************************/

#include "log_manager.hpp"

#include <fstream>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/ssl.h>

#include "cpp_httplib/httplib.h"

namespace OpenAPT::Logger
{
    LoggerManager::LoggerManager()
    {
        logger.setCurrentModule("LogManager");
        logger.enableAsyncLogging();
    }

    LoggerManager::~LoggerManager()
    {
        logger.disableAsyncLogging();
    }

    void LoggerManager::scanLogsFolder(const std::string &folderPath)
    {
        for (const auto &entry : std::filesystem::directory_iterator(folderPath))
        {
            std::wcout << entry.path().c_str() << std::endl;
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
            logger.logInfo("File uploaded successfully");
        }
        else
        {
            logger.logError("Failed to upload file");
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
                logger.logDebug("{}", logEntry.message);
            }
        }
        return errorMessages;
    }

    void LoggerManager::analyzeLogs()
    {
        std::vector<std::string> errorMessages = extractErrorMessages();

        if (errorMessages.empty())
        {
            logger.logInfo("No errors found in the logs.");
            return;
        }
        logger.logInfo("Analyzing logs...");

        // 统计错误类型
        std::map<std::string, int> errorTypeCount;
        for (const auto &errorMessage : errorMessages)
        {
            std::string errorType = getErrorType(errorMessage);
            errorTypeCount[errorType]++;
        }
        logger.logInfo("Error Type Count:");
        for (const auto &[errorType, count] : errorTypeCount)
        {
            logger.logInfo("{} : {}", errorType, count);
        }

        // 查找最常见的错误消息
        std::string mostCommonErrorMessage = getMostCommonErrorMessage(errorMessages);
        logger.logInfo("Most Common Error Message: {}", mostCommonErrorMessage);
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
