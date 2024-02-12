/*
 * global_logger.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Global Logger for Atom - Lithium Framework.

**************************************************/

#include "global_logger.hpp"

#include <fstream>
#include <ctime>
#include <iostream>

Logger::Logger() : logLevel_(LogLevel::Info), running_(true)
{
    workerThread_ = std::thread(&Logger::workerFunction, this);
}

Logger::~Logger()
{
    // 停止工作线程
    running_ = false;
    cv_.notify_all();
    workerThread_.join();
}

void Logger::addSubscriber(std::shared_ptr<Subscriber> subscriber)
{
    subscribers_.push_back(subscriber);
}

void Logger::setLogLevel(LogLevel level)
{
    logLevel_ = level;
}

void Logger::log(LogLevel level, const std::string &message)
{
    if (level < logLevel_)
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        logQueue_.push(std::make_pair(level, message));
    }

    cv_.notify_one();
}

std::string Logger::getLogLevelString(LogLevel level)
{
    switch (level)
    {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    default:
        return "";
    }
}

void Logger::workerFunction()
{
    while (running_)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]()
                 { return !logQueue_.empty() || !running_; });

        while (!logQueue_.empty())
        {
            auto logEntry = logQueue_.front();
            logQueue_.pop();

            // 打印到控制台
            std::cout << "[" << getLogLevelString(logEntry.first) << "] " << logEntry.second << std::endl;

            // 写入日志文件
            writeToLogFile(getLogLevelString(logEntry.first), logEntry.second);

            // 通知所有订阅者
            for (auto subscriber : subscribers_)
            {
                subscriber->log(logEntry.first, logEntry.second);
            }
        }
    }
}

void Logger::writeToLogFile(const std::string &levelString, const std::string &message)
{
    std::ofstream logFile("log.txt", std::ios::app); // 追加模式
    if (logFile.is_open())
    {
        // 获取当前时间
        std::time_t currentTime = std::time(nullptr);
        std::string timeString = std::asctime(std::localtime(&currentTime));

        // 格式化日志条目
        std::string logEntry = "[" + timeString.substr(0, timeString.length() - 1) + "] " +
                               "[" + levelString + "] " + message;

        // 写入日志文件
        logFile << logEntry << std::endl;

        logFile.close();
    }
}

/*
class ConsoleSubscriber : public Logger::Subscriber
{
public:
    void log(LogLevel level, const std::string& message) override
    {
        std::cout << "ConsoleSubscriber: [" << Logger::getLogLevelString(level) << "] " << message << std::endl;
    }
};

class FileSubscriber : public Logger::Subscriber
{
public:
    void log(LogLevel level, const std::string& message) override
    {
        std::ofstream logFile("file_log.txt", std::ios::app); // 追加模式
        if (logFile.is_open())
        {
            logFile << "[" << Logger::getLogLevelString(level) << "] " << message << std::endl;
            logFile.close();
        }
    }
};

class PerformanceMonitor
{
public:
    PerformanceMonitor(Logger& logger) : logger_(logger), running_(true)
    {
        monitorThread_ = std::thread(&PerformanceMonitor::monitorFunction, this);
    }

    ~PerformanceMonitor()
    {
        running_ = false;
        monitorThread_.join();
    }

private:
    void monitorFunction()
    {
        while (running_)
        {
            // 获取性能统计信息
            std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

            // 模拟耗时操作
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            std::chrono::steady_clock::time_point endTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

            // 记录性能统计信息
            logger_.log(LogLevel::Info, "Performance Monitor: Elapsed Time - " + std::to_string(elapsedTime) + "ms");
        }
    }

    Logger& logger_;
    bool running_;
    std::thread monitorThread_;
};

int main()
{
    Logger logger;
    ConsoleSubscriber consoleSubscriber;
    FileSubscriber fileSubscriber;

    // 添加订阅者
    logger.addSubscriber(&consoleSubscriber);
    logger.addSubscriber(&fileSubscriber);

    // 记录日志
    logger.log(LogLevel::Info, "Hello World!");

    // 设置日志级别
    logger.setLogLevel(LogLevel::Debug);

    // 记录调试日志
    logger.log(LogLevel::Debug, "Debug message");

    // 启动性能监视器
    PerformanceMonitor performanceMonitor(logger);

    // 模拟应用程序运行
    std::this_thread::sleep_for(std::chrono::seconds(5));

    return 0;
}
*/
