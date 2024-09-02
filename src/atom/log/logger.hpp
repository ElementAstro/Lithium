/*
 * logger.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-08-19

Description: Optimized Custom Logger Manager

**************************************************/

#ifndef ATOM_LOG_LOGGER_HPP
#define ATOM_LOG_LOGGER_HPP

#include <memory>
#include <string>
#include <vector>
#include "macro.hpp"

namespace lithium {
struct LogEntry {
    std::string fileName;
    int lineNumber;
    std::string message;
} ATOM_ALIGNAS(128);

/**
 * @brief 日志管理器类，用于扫描、分析和上传日志文件
 */
class LoggerManager {
public:
    LoggerManager();
    ~LoggerManager();

    void scanLogsFolder(const std::string &folderPath);
    std::vector<LogEntry> searchLogs(std::string_view keyword);
    void uploadFile(const std::string &filePath);
    void analyzeLogs();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;  // Pimpl 设计模式
};

}  // namespace lithium

#endif
