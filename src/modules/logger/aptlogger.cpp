/*
 * aptlogger.cpp
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

#include "aptlogger.hpp"

namespace Lithium::Logger
{

    std::string_view levelToString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::Trace:
            return "TRACE";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Critical:
            return "Critical";
        default:
            return "";
        }
    }
    void Logger::setLogLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        currentLogLevel = level;
    }

    void Logger::setLogToFile(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        outputFileStream.open(filename, std::ios::out | std::ios::app);
        if (outputFileStream.is_open())
        {
            outputStream = &outputFileStream;
        }
        else
        {
            logError("Failed to open log file: {}", filename);
            outputStream = &std::cout; // 将输出流重置为std::cout
        }
    }

    void Logger::setFilterLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        filterLevel = level;
    }

    void Logger::enableAsyncLogging()
    {
        asyncLoggingEnabled = true;
        backgroundThread = std::thread(&Logger::loggingFunction, this);
    }

    void Logger::disableAsyncLogging()
    {
        if (asyncLoggingEnabled)
        {
            asyncLoggingEnabled = false;
            if (backgroundThread.joinable())
            {
                queueCondVar.notify_one();
                backgroundThread.join();
            }
        }
    }

    std::vector<std::string> Logger::getErrorMessages() const
    {
        return errorMessages;
    }

    std::string Logger::getCurrentTime()
    {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm *localTime = std::localtime(&time);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
        return buffer;
    }

    std::string Logger::getCurrentModule()
    {
        return currentModuleName;
    }

    void Logger::loggingFunction()
    {
        while (true)
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondVar.wait(lock, [this]
                              { return !logQueue.empty() || !asyncLoggingEnabled; });

            if (!asyncLoggingEnabled)
            {
                return;
            }

            std::string message = std::get<0>(logQueue.front());
            std::string timestamp = std::get<1>(logQueue.front());
            std::string module = std::get<2>(logQueue.front());
            LogLevel level = std::get<3>(logQueue.front());
            logQueue.pop();
            lock.unlock();

            if (currentLogLevel == LogLevel::Error)
            {
                errorMessages.push_back(message);
            }

            std::string formattedMsg = fmt::format("[{}][{}][{}] {}\n", timestamp, levelToString(level), module, message);
            *outputStream << formattedMsg;
            outputStream->flush();
        }
    }

    bool Logger::shouldLog(LogLevel level)
    {
        return level >= currentLogLevel && level >= filterLevel;
    }

    void Logger::setCurrentModule(std::string module_name)
    {
        currentModuleName = module_name;
    }

    std::string getErrorLogFilename(const std::string &extension)
    {
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm *localTime = std::localtime(&time);
        char buffer[80];
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", localTime);
        return std::string(buffer) + extension;
    }

    void writeErrorLog(const std::vector<std::string> &errorMessages, const std::string &filename)
    {
        std::ofstream file(filename, std::ios::out);
        if (file.is_open())
        {
            for (const std::string &errorMessage : errorMessages)
            {
                file << errorMessage << "\n";
            }
            file.close();
        }
        else
        {
            std::cerr << "Failed to open error log file: " << filename << std::endl;
        }
    }
}