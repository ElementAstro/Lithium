/*
 * syslog.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Simple syslog wrapper for Windows and Linux

**************************************************/

#ifndef ATOM_LOG_SYSLOG_HPP
#define ATOM_LOG_SYSLOG_HPP

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#else
#include <syslog.h>
#endif

namespace Atom::Log {
enum class LogLevel {
    Debug,    ///< 调试级别
    Info,     ///< 信息级别
    Warning,  ///< 警告级别
    Error     ///< 错误级别
};

/**
 * SyslogWrapper类封装了系统日志的功能
 */
class SyslogWrapper {
public:
    /**
     * 构造函数，默认为Info级别，输出到Event日志
     */
    SyslogWrapper() : SyslogWrapper(LogLevel::Info, "Event") {}

    /**
     * 构造函数，指定日志级别和输出目标
     * @param logLevel 日志级别
     * @param target
     * 输出目标，可选值为"Event"、"Console"、""（默认为空，表示输出到syslog或文件）
     */
    SyslogWrapper(LogLevel logLevel, const std::string &target = "");

    /**
     * 析构函数，关闭句柄或清理工作
     */
    ~SyslogWrapper() noexcept;

    /**
     * 记录日志，支持格式化字符串
     * @tparam Args 可变参数模板参数类型
     * @param level 日志级别
     * @param format 格式化字符串
     * @param args 可变参数列表
     */
    template <typename... Args>
    void log(LogLevel level, const char *format, Args &&...args);

    /**
     * 记录日志，支持格式化字符串
     * @tparam Arg 可变参数模板参数类型
     * @tparam Args 可变参数模板参数类型
     * @param level 日志级别
     * @param format 格式化字符串
     * @param arg 可变参数列表
     * @param args 可变参数列表
     */
    template <typename Arg, typename... Args>
    void log(LogLevel level, const char *format, Arg &&arg, Args &&...args);

    /**
     * 设置日志级别
     * @param logLevel 日志级别
     */
    void setLogLevel(LogLevel logLevel);

    /**
     * 获取日志级别
     * @return 日志级别
     */
    LogLevel getLogLevel() const;

    /**
     * 输出调试信息
     * @tparam Args 可变参数模板参数类型
     * @param format 格式化字符串
     * @param args 可变参数列表
     */
    template <typename... Args>
    void debug(const char *format, Args &&...args);

    /**
     * 输出信息
     * @tparam Args 可变参数模板参数类型
     * @param format 格式化字符串
     * @param args 可变参数列表
     */
    template <typename... Args>
    void info(const char *format, Args &&...args);

    /**
     * 输出警告信息
     * @tparam Args 可变参数模板参数类型
     * @param format 格式化字符串
     * @param args 可变参数列表
     */
    template <typename... Args>
    void warning(const char *format, Args &&...args);

    /**
     * 输出错误信息
     * @tparam Args 可变参数模板参数类型
     * @param format 格式化字符串
     * @param args 可变参数列表
     */
    template <typename... Args>
    void error(const char *format, Args &&...args);

private:
#ifdef _WIN32
    HANDLE m_eventHandle;
#endif
    std::string m_target;
    LogLevel m_logLevel;
    std::mutex m_mutex;  // 线程安全的互斥量

    /**
     * 将当前时间格式化为字符串
     * @return 格式化后的时间字符串
     */
    std::string formatTime() const;
};

template <typename... Args>
void SyslogWrapper::log(LogLevel level, const char *format, Args &&...args) {
    // 如果当前级别低于指定级别，则不输出
    if (static_cast<int>(level) < static_cast<int>(m_logLevel)) {
        return;
    }

    // 获取当前时间
    std::string timeStr = formatTime();

    // 将当前时间和日志消息格式化为一个字符串
    char logString[1024];
    snprintf(logString, sizeof(logString), "[%s] %s", timeStr.c_str(), format);

    // 向日志目标输出日志消息
#ifdef _WIN32
    if (m_target == "Event") {
        ReportEvent(m_eventHandle, static_cast<WORD>(level), 0, 0, nullptr, 1,
                    0, (const CHAR **)&logString, nullptr);
    } else if (m_target == "Console") {
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(consoleHandle, &csbi);
        WORD oldAttributes = csbi.wAttributes;
        SetConsoleTextAttribute(consoleHandle, static_cast<WORD>(level));
        printf("%s\n", logString);
        SetConsoleTextAttribute(consoleHandle, oldAttributes);
    } else {
        OutputDebugStringA(logString);
    }
#else
    if (m_target.empty()) {
        syslog(static_cast<int>(level), "%s", logString);
    } else {
        printf("%s\n", logString);
    }
#endif
}

template <typename Arg, typename... Args>
void SyslogWrapper::log(LogLevel level, const char *format, Arg &&arg,
                        Args &&...args) {
    // 重载的log方法用于处理参数转发，逐个处理参数并将其传递给格式化字符串
    log(level, format, std::forward<Arg>(arg));
    log(level, format, std::forward<Args>(args)...);
}

void SyslogWrapper::setLogLevel(LogLevel logLevel) { m_logLevel = logLevel; }

LogLevel SyslogWrapper::getLogLevel() const { return m_logLevel; }

std::string SyslogWrapper::formatTime() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch())
                  .count() %
              1000;
    std::tm tm_now{};
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
    char buf[40] = {0};
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d.%03lld",
             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
             tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, ms);
    return buf;
}

template <typename... Args>
void SyslogWrapper::debug(const char *format, Args &&...args) {
    log(LogLevel::Debug, format, std::forward<Args>(args)...);
}

template <typename... Args>
void SyslogWrapper::info(const char *format, Args &&...args) {
    log(LogLevel::Info, format, std::forward<Args>(args)...);
}

template <typename... Args>
void SyslogWrapper::warning(const char *format, Args &&...args) {
    log(LogLevel::Warning, format, std::forward<Args>(args)...);
}

template <typename... Args>
void SyslogWrapper::error(const char *format, Args &&...args) {
    log(LogLevel::Error, format, std::forward<Args>(args)...);
}
}  // namespace Atom::Log

#endif
