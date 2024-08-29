/*
 * logger.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-08-19

Description: Optimized Custom Logger Manager Implementation

**************************************************/

#include "logger.hpp"

#include <filesystem>
#include <fstream>
#include <map>
#include <string_view>
#include <thread>
#include <vector>

#include "atom/log/loguru.hpp"
#include "atom/web/curl.hpp"

namespace lithium {

class LoggerManager::Impl {
public:
    void scanLogsFolder(const std::string &folderPath);
    auto searchLogs(std::string_view keyword) -> std::vector<LogEntry>;
    void uploadFile(const std::string &filePath);
    void analyzeLogs();

private:
    void parseLog(const std::string &filePath);
    auto extractErrorMessages() -> std::vector<std::string>;
    auto encryptFileContent(const std::string &content) -> std::string;
    auto getErrorType(std::string_view errorMessage) -> std::string;
    auto getMostCommonErrorMessage(
        const std::vector<std::string> &errorMessages) -> std::string;

    std::vector<LogEntry> logEntries_;  // 存储日志条目的向量
};

LoggerManager::LoggerManager() : pImpl(std::make_unique<Impl>()) {}
LoggerManager::~LoggerManager() = default;

void LoggerManager::scanLogsFolder(const std::string &folderPath) {
    pImpl->scanLogsFolder(folderPath);
}

auto LoggerManager::searchLogs(std::string_view keyword)
    -> std::vector<LogEntry> {
    return pImpl->searchLogs(keyword);
}

void LoggerManager::uploadFile(const std::string &filePath) {
    pImpl->uploadFile(filePath);
}

void LoggerManager::analyzeLogs() { pImpl->analyzeLogs(); }

void LoggerManager::Impl::scanLogsFolder(const std::string &folderPath) {
    std::vector<std::jthread> threads;
    for (const auto &entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            threads.emplace_back(&Impl::parseLog, this, entry.path().string());
        }
    }
}

auto LoggerManager::Impl::searchLogs(std::string_view keyword)
    -> std::vector<LogEntry> {
    std::vector<LogEntry> searchResults;
    for (const auto &logEntry : logEntries_) {
        if (logEntry.message.find(keyword) != std::string::npos) {
            searchResults.push_back(logEntry);
        }
    }
    return searchResults;
}

void LoggerManager::Impl::parseLog(const std::string &filePath) {
    std::ifstream logFile(filePath);
    if (logFile.is_open()) {
        std::string line;
        int lineNumber = 1;
        while (std::getline(logFile, line)) {
            logEntries_.push_back({filePath, lineNumber++, line});
        }
    }
}

void LoggerManager::Impl::uploadFile(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        LOG_F(ERROR, "Failed to open file: {}", filePath);
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    std::string encryptedContent = encryptFileContent(content);

    atom::web::CurlWrapper curl;
    curl.setUrl("https://lightapt.com/upload");
    curl.setRequestMethod("POST");
    curl.setHeader("Content-Type", "application/octet-stream");
    curl.setRequestBody(encryptedContent);

    curl.setOnErrorCallback([](CURLcode error) {
        LOG_F(ERROR, "Failed to upload file: curl error code %d", error);
    });

    curl.setOnResponseCallback([](const std::string &response) {
        DLOG_F(INFO, "File uploaded successfully. Server response: %s",
               response.c_str());
    });

    curl.performRequest();
}

auto LoggerManager::Impl::extractErrorMessages() -> std::vector<std::string> {
    std::vector<std::string> errorMessages;
    for (const auto &logEntry : logEntries_) {
        if (logEntry.message.find("[ERROR]") != std::string::npos) {
            errorMessages.push_back(logEntry.message);
            DLOG_F(INFO, "{}", logEntry.message);
        }
    }
    return errorMessages;
}

void LoggerManager::Impl::analyzeLogs() {
    auto errorMessages = extractErrorMessages();

    if (errorMessages.empty()) {
        DLOG_F(INFO, "No errors found in the logs.");
        return;
    }
    DLOG_F(INFO, "Analyzing logs...");

    std::map<std::string, int> errorTypeCount;
    for (const auto &errorMessage : errorMessages) {
        std::string errorType = getErrorType(errorMessage);
        errorTypeCount[errorType]++;
    }

    DLOG_F(INFO, "Error Type Count:");
    for (const auto &[errorType, count] : errorTypeCount) {
        DLOG_F(INFO, "{} : {}", errorType, count);
    }

    std::string mostCommonErrorMessage =
        getMostCommonErrorMessage(errorMessages);
    DLOG_F(INFO, "Most Common Error Message: {}", mostCommonErrorMessage);
}

std::string LoggerManager::Impl::encryptFileContent(
    const std::string &content) {
    // 简单的加密示例，可以根据需要替换为更复杂的加密算法
    std::string encryptedContent;
    for (char c : content) {
        encryptedContent += c ^ 0xFF;
    }
    return encryptedContent;
}

std::string LoggerManager::Impl::getErrorType(std::string_view errorMessage) {
    auto startPos = errorMessage.find("[");
    auto endPos = errorMessage.find("]");
    if (startPos != std::string::npos && endPos != std::string::npos &&
        endPos > startPos) {
        return std::string(
            errorMessage.substr(startPos + 1, endPos - startPos - 1));
    }
    return "Unknown";
}

std::string LoggerManager::Impl::getMostCommonErrorMessage(
    const std::vector<std::string> &errorMessages) {
    std::map<std::string, int> errorMessageCount;
    for (const auto &errorMessage : errorMessages) {
        errorMessageCount[errorMessage]++;
    }

    std::string mostCommonErrorMessage;
    int maxCount = 0;
    for (const auto &[errorMessage, count] : errorMessageCount) {
        if (count > maxCount) {
            mostCommonErrorMessage = errorMessage;
            maxCount = count;
        }
    }
    return mostCommonErrorMessage;
}

}  // namespace lithium
