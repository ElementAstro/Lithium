/*
 * syslog.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Simple syslog wrapper for Windows and Linux

**************************************************/

#pragma once

#include <ctime>
#include <string>
#include <mutex>

#ifdef _WIN32
#include <Windows.h>
#else
#include <syslog.h>
#endif

namespace Atom::System
{

    // 日志级别枚举
    enum class LogLevel
    {
        Debug,   ///< 调试级别
        Info,    ///< 信息级别
        Warning, ///< 警告级别
        Error    ///< 错误级别
    };

    /// SyslogWrapper类封装了系统日志的功能
    class SyslogWrapper
    {
    public:
        /**
         * 构造函数，默认为Info级别，输出到Event日志
         */
        SyslogWrapper();

        /**
         * 构造函数，指定日志级别和输出目标
         * @param logLevel 日志级别
         * @param target 输出目标，可选值为"Event"、"Console"、""（默认为空，表示输出到syslog或文件）
         */
        SyslogWrapper(LogLevel logLevel, const std::string &target = "");

        /**
         * 析构函数，关闭句柄或清理工作
         */
        ~SyslogWrapper();

        /**
         * 记录日志，支持格式化字符串
         * @tparam Args 可变参数模板参数类型
         * @param level 日志级别
         * @param format 格式化字符串
         * @param args 可变参数列表
         */
        template <typename... Args>
        void log(LogLevel level, const char *format, Args... args);

    private:
#ifdef _WIN32
        HANDLE m_eventHandle;
#endif
        LogLevel m_logLevel;
        std::mutex m_mutex; // 线程安全的互斥量
    };

    template <typename... Args>
    void SyslogWrapper::log(LogLevel level, const char *format, Args... args)
    {
        // 如果当前级别低于指定级别，则不输出
        if (static_cast<int>(level) < static_cast<int>(m_logLevel))
        {
            return;
        }

        // 获取当前时间
        std::time_t now = std::time(nullptr);
        std::tm localTime;
#ifdef _WIN32
        localtime_s(&localTime, &now);
#else
        localtime_r(&now, &localTime);
#endif

        // 将当前时间和日志消息格式化为一个字符串
        char logString[1024];
        snprintf(logString, sizeof(logString), "[%04d-%02d-%02d %02d:%02d:%02d] %s",
                 localTime.tm_year + 1900, localTime.tm_mon + 1, localTime.tm_mday,
                 localTime.tm_hour, localTime.tm_min, localTime.tm_sec, format);

        // 向日志目标输出日志消息
#ifdef _WIN32
        ReportEvent(m_eventHandle, static_cast<WORD>(level), 0, 0, nullptr, 1, 0, (const CHAR **)&logString, nullptr);
#else
        syslog(static_cast<int>(level), "%s", logString);
#endif
    }
}
