/*
 * log_manager.hpp
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

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "aptlogger.hpp"

namespace OpenAPT::Logger
{
    struct LogEntry
    {
        std::string fileName;
        int lineNumber;
        std::string message;
    };

    /**
     * @brief LoggerManager类用于管理日志。
     */
    class LoggerManager
    {
    public:
        /**
         * @brief 构造函数。
         */
        LoggerManager();

        /**
         * @brief 析构函数。
         */
        ~LoggerManager();

        /**
         * @brief 扫描指定文件夹中的日志文件。
         *
         * @param folderPath 文件夹路径。
         */
        void scanLogsFolder(const std::string &folderPath);

        /**
         * @brief 根据关键字搜索日志条目。
         *
         * @param keyword 关键字。
         * @return 包含匹配的日志条目的向量。
         */
        std::vector<LogEntry> searchLogs(const std::string &keyword);

        /**
         * @brief 解析指定的日志文件。
         *
         * @param filePath 文件路径。
         */
        void parseLog(const std::string &filePath);

        /**
         * @brief 上传文件。
         *
         * @param filePath 文件路径。
         */
        void uploadFile(const std::string &filePath);

        /**
         * @brief 提取错误消息。
         *
         * @return 错误消息的向量。
         */
        std::vector<std::string> extractErrorMessages();

        /**
         * @brief 分析日志。
         */
        void analyzeLogs();

        /**
         * @brief 获取错误类型。
         *
         * @param errorMessage 错误消息。
         * @return 错误类型。
         */
        std::string getErrorType(const std::string &errorMessage);

        /**
         * @brief 获取最常见的错误消息。
         *
         * @param errorMessages 错误消息的向量。
         * @return 最常见的错误消息。
         */
        std::string getMostCommonErrorMessage(const std::vector<std::string> &errorMessages);

    private:
        std::vector<LogEntry> logEntries; /**< 日志条目的向量。 */
        std::vector<std::string> errorMessages; /**< 错误消息的向量。 */

        /**
         * @brief 计算文件的MD5哈希值。
         *
         * @param filePath 文件路径。
         * @return MD5哈希值。
         */
        std::string computeMd5Hash(const std::string &filePath);

        Logger &logger = GlobalLogger::getDefaultLogger();
    };

}