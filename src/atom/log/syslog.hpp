/*
 * syslog.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Enhanced syslog wrapper for Windows and Linux

**************************************************/

#ifndef ATOM_LOG_SYSLOG_HPP
#define ATOM_LOG_SYSLOG_HPP

#include <chrono>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <syslog.h>
#endif

#include <fmt/core.h>
#include <fmt/format.h>

namespace atom::log {

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
    explicit SyslogWrapper(LogLevel logLevel, const std::string &target = "");

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
    void log(LogLevel level, fmt::format_string<Args...> format,
             Args &&...args);

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
    void debug(fmt::format_string<Args...> format, Args &&...args);

    /**
     * 输出信息
     * @tparam Args 可变参数模板参数类型
     * @param format 格式化字符串
     * @param args 可变参数列表
     */
    template <typename... Args>
    void info(fmt::format_string<Args...> format, Args &&...args);

    /**
     * 输出警告信息
     * @tparam Args 可变参数模板参数类型
     * @param format 格式化字符串
     * @param args 可变参数列表
     */
    template <typename... Args>
    void warning(fmt::format_string<Args...> format, Args &&...args);

    /**
     * 输出错误信息
     * @tparam Args 可变参数模板参数类型
     * @param format 格式化字符串
     * @param args 可变参数列表
     */
    template <typename... Args>
    void error(fmt::format_string<Args...> format, Args &&...args);

private:
#ifdef _WIN32
    HANDLE m_eventHandle;
#endif
    std::string m_target;
    LogLevel m_logLevel;
    std::mutex m_mutex;                  // 线程安全的互斥量
    std::ofstream m_logFile;             // 日志文件
    std::queue<std::string> m_logQueue;  // 日志队列
    std::thread m_logThread;             // 日志线程
    bool m_exitThread;                   // 退出线程标志

    /**
     * 将当前时间格式化为字符串
     * @return 格式化后的时间字符串
     */
    std::string formatTime() const;

    /**
     * 异步日志处理函数
     */
    void processLogQueue();

    /**
     * 轮替日志文件
     */
    void rotateLogFile();

    /**
     * 获取当前日志文件大小
     * @return 日志文件大小
     */
    std::size_t getLogFileSize() const;

    /**
     * 日志级别转换为字符串
     * @param level 日志级别
     * @return 字符串表示的日志级别
     */
    std::string logLevelToString(LogLevel level) const;

    /**
     * 日志写入函数
     * @param logString 日志字符串
     */
    void writeLog(const std::string &logString);

    /**
     * 日志格式化函数
     * @param level 日志级别
     * @param format 格式化字符串
     * @param args 格式化参数
     * @return 格式化后的日志字符串
     */
    std::string formatLogMessage(LogLevel level, fmt::string_view format,
                                 fmt::format_args args);
};
}

#include "syslog.inl"

#endif
