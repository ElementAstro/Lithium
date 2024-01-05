/*
 * log_manager.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-11-10

Description: Custom Logger Manager

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Lithium
{
    struct LogEntry
    {
        std::string fileName;
        int lineNumber;
        std::string message;
    };

    /**
     * @brief 日志管理器类，用于扫描、分析和上传日志文件
     */
    class LoggerManager
    {
    public:
        /**
         * @brief 默认构造函数
         */
        LoggerManager() = default;

        /**
         * @brief 默认析构函数
         */
        ~LoggerManager() = default;

        /**
         * @brief 扫描指定文件夹下的日志文件
         * @param folderPath 待扫描的文件夹路径
         */
        void scanLogsFolder(const std::string &folderPath);

        /**
         * @brief 根据关键词搜索日志内容
         * @param keyword 关键词
         * @return 包含关键词的日志条目集合
         */
        std::vector<LogEntry> searchLogs(const std::string &keyword);

        /**
         * @brief 上传指定文件
         * @param filePath 待上传的文件路径
         */
        void uploadFile(const std::string &filePath);

        /**
         * @brief 分析日志文件内容
         */
        void analyzeLogs();

    private:
        /**
         * @brief 解析日志文件
         * @param filePath 日志文件路径
         */
        void parseLog(const std::string &filePath);

        /**
         * @brief 提取错误消息
         * @return 包含所有错误消息的字符串向量
         */
        std::vector<std::string> extractErrorMessages();

        /**
         * @brief 计算文件的 MD5 哈希值
         * @param filePath 文件路径
         * @return MD5 哈希值的字符串表示
         */
        std::string computeMd5Hash(const std::string &filePath);

        /**
         * @brief 获取错误消息中的错误类型
         * @param errorMessage 错误消息
         * @return 错误类型
         */
        std::string getErrorType(const std::string &errorMessage);

        /**
         * @brief 获取出现频率最高的错误消息
         * @param errorMessages 错误消息集合
         * @return 出现频率最高的错误消息
         */
        std::string getMostCommonErrorMessage(const std::vector<std::string> &errorMessages);

        std::vector<LogEntry> logEntries; // 存储日志条目的向量
    };

}