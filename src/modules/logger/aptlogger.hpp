/*
 * aptlogger.hpp
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

Date: 2023-6-28

Description: Custom Logger (last)

**************************************************/

#pragma once

#include <iostream>
#include <fstream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <ctime>
#include <string>
#include <vector>
#include <regex>
#include <fmt/core.h>

namespace Lithium::Logger
{
    /**
     * @brief 日志级别枚举类
     */
    enum class LogLevel
    {
        None,    // 无日志
        Trace,   // 追踪日志
        Debug,   // 调试日志
        Info,    // 信息日志
        Warning, // 警告日志
        Error,   // 错误日志
        Critical // 严重错误日志
    };

    /**
     * @brief 将日志级别转换为字符串表示
     * @param level 日志级别
     * @return 日志级别的字符串表示
     */
    std::string_view levelToString(LogLevel level);

    /**
     * @brief 日志记录器类
     */
    class Logger
    {
    public:
        /**
         * @brief 构造函数
         * @param stream 输出流
         * @param module_name 模块名称（默认为 "Main"）
         */
        Logger(std::ostream &stream, std::string module_name = "Main") : outputStream(&std::cout), currentModuleName(module_name) {}


        /**
         * @brief 析构函数
         */
        ~Logger()
        {
            if(asyncLoggingEnabled)
            {
                disableAsyncLogging();
            }
        }

        /**
         * @brief 记录日志
         * @tparam Args 可变参数类型
         * @param level 日志级别
         * @param formatStr 格式化字符串
         * @param args 可变参数
         */
        template <typename... Args>
        void log(LogLevel level, const std::string &formatStr, Args... args)
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (shouldLog(level))
            {
                // TODO : 增加日志大小的限制，防止出错，可能会炸
                char buffer[512];
                std::snprintf(buffer, sizeof(buffer), formatStr.c_str(), args...);
                std::string message(buffer);

                std::string timestamp = getCurrentTime();
                std::string module = getCurrentModule();
                logQueue.push({message, timestamp, module, level});
                queueCondVar.notify_one();
                if (level == LogLevel::Error)
                {
                    errorMessages.push_back(message);
                }
            }
        }

        /**
         * @brief 记录追踪级别日志
         * @tparam Args 可变参数类型
         * @param formatStr 格式化字符串
         * @param args 可变参数
         */
        template <typename... Args>
        void logTrace(const std::string &formatStr, Args... args)
        {
            log(LogLevel::Trace, formatStr, args...);
        }

        /**
         * @brief 记录调试级别日志
         * @tparam Args 可变参数类型
         * @param formatStr 格式化字符串
         * @param args 可变参数
         */
        template <typename... Args>
        void logDebug(const std::string &formatStr, Args... args)
        {
            log(LogLevel::Debug, formatStr, args...);
        }

        /**
         * @brief 记录信息级别日志
         * @tparam Args 可变参数类型
         * @param formatStr 格式化字符串
         * @param args 可变参数
         */
        template <typename... Args>
        void logInfo(const std::string &formatStr, Args... args)
        {
            log(LogLevel::Info, formatStr, args...);
        }

        /**
         * @brief 记录警告级别日志
         * @tparam Args 可变参数类型
         * @param formatStr 格式化字符串
         * @param args 可变参数
         */
        template <typename... Args>
        void logWarn(const std::string &formatStr, Args... args)
        {
            log(LogLevel::Warning, formatStr, args...);
        }

        /**
         * @brief 记录错误级别日志
         * @tparam Args 可变参数类型
         * @param formatStr 格式化字符串
         * @param args 可变参数
         */
        template <typename... Args>
        void logError(const std::string &formatStr, Args... args)
        {
            log(LogLevel::Error, formatStr, args...);
        }

        /**
         * @brief 记录严重错误级别日志
         * @tparam Args 可变参数类型
         * @param formatStr 格式化字符串
         * @param args 可变参数
         */
        template <typename... Args>
        void logCritical(const std::string &formatStr, Args... args)
        {
            log(LogLevel::Critical, formatStr, args...);
        }

        /**
         * @brief 设置日志记录器的日志级别
         * @param level 日志级别
         */
        void setLogLevel(LogLevel level);

        /**
         * @brief 将日志输出到文件中
         * @param filename 文件名
         */
        void setLogToFile(const std::string &filename);

        /**
         * @brief 设置日志过滤级别
         * @param level 过滤级别
         */
        void setFilterLevel(LogLevel level);

        /**
         * @brief 启用异步日志记录
         */
        void enableAsyncLogging();

        /**
         * @brief 禁用异步日志记录
         */
        void disableAsyncLogging();

        /**
         * @brief 获取错误日志消息列表
         * @return 错误日志消息列表
         */
        std::vector<std::string> getErrorMessages() const;

        /**
         * @brief 设置当前模块名称
         * @param module_name 模块名称
         */
        void setCurrentModule(std::string module_name);

    private:
        std::ostream *outputStream = nullptr;
        std::ofstream outputFileStream;
        std::queue<std::tuple<std::string, std::string, std::string, LogLevel>> logQueue;
        std::mutex queueMutex;
        std::condition_variable queueCondVar;
        LogLevel currentLogLevel = LogLevel::Info;
        LogLevel filterLevel = LogLevel::None;
        bool asyncLoggingEnabled = false;
        std::thread backgroundThread;
        std::vector<std::string> errorMessages;
        std::string currentModuleName;

        /**
         * @brief 获取当前时间
         * @return 当前时间的字符串表示
         */
        std::string getCurrentTime();

        /**
         * @brief 获取当前模块名称
         * @return 当前模块名称
         */
        std::string getCurrentModule();

        /**
         * @brief 日志记录函数
         */
        void loggingFunction();

        /**
         * @brief 判断是否需要记录指定级别的日志
         * @param level 日志级别
         * @return 如果需要记录指定级别的日志，则返回true；否则返回false
         */
        bool shouldLog(LogLevel level);
    };

    /**
     * @brief 全局日志记录器类
     */
    class GlobalLogger
    {
    public:
        /**
         * @brief 获取默认日志记录器
         * @return 默认日志记录器的引用
         */
        static Logger &getDefaultLogger()
        {
            static Logger logger(std::cout);
            return logger;
        }
    };

    /**
     * @brief 获取错误日志文件名（带扩展名）
     * @param extension 文件扩展名
     * @return 错误日志文件名（带扩展名）
     */
    std::string getErrorLogFilename(const std::string &extension);

    /**
     * @brief 将错误日志写入文件中
     * @param errorMessages 错误日志消息列表
     * @param filename 文件名
     */
    void writeErrorLog(const std::vector<std::string> &errorMessages, const std::string &filename);

} // namespace Lithium::Logger