/*
 * global_logger.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Global Logger for Atom - Lithium Framework.

**************************************************/

#pragma once

#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};

/**
 * @class Logger
 * @brief 日志记录器类，支持动态调整日志级别、异步记录和多订阅者。
 */
class Logger
{
public:
    /**
     * @class Subscriber
     * @brief 日志订阅者接口类，用于接收日志消息。
     */
    class Subscriber
    {
    public:
        /**
         * @brief 记录日志消息。
         * @param level 日志级别。
         * @param message 日志消息。
         */
        virtual void log(LogLevel level, const std::string &message) = 0;
    };

    /**
     * @brief 构造函数。
     */
    Logger();

    /**
     * @brief 析构函数。
     */
    ~Logger();

    /**
     * @brief 添加日志订阅者。
     * @param subscriber 订阅者对象指针。
     */
    void addSubscriber(std::shared_ptr<Subscriber> subscriber);

    /**
     * @brief 设置日志级别。
     * @param level 日志级别。
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief 记录日志消息。
     * @param level 日志级别。
     * @param message 日志消息。
     */
    void log(LogLevel level, const std::string &message);

    /**
     * @brief 获取日志级别对应的字符串表示。
     * @param level 日志级别。
     * @return 字符串表示。
     */
    static std::string getLogLevelString(LogLevel level);

    /**
     * @brief 工作线程函数，用于异步处理日志消息。
     */
    void workerFunction();

    /**
     * @brief 写入日志文件。
     * @param levelString 日志级别字符串表示。
     * @param message 日志消息。
     */
    void writeToLogFile(const std::string &levelString, const std::string &message);

private:
    LogLevel logLevel_;                                     /**< 日志级别。*/
    std::vector<std::shared_ptr<Subscriber>> subscribers_;                 /**< 订阅者列表。*/
    std::queue<std::pair<LogLevel, std::string>> logQueue_; /**< 日志消息队列。*/
    std::mutex mutex_;                                      /**< 互斥锁，用于保护日志消息队列。*/
    std::condition_variable cv_;                            /**< 条件变量，用于线程同步。*/
    bool running_;                                          /**< 工作线程运行标志。*/
    std::thread workerThread_;                              /**< 工作线程对象。*/
};
