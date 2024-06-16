/*
 * syslog.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Simple syslog wrapper for Windows and Linux

**************************************************/

#include "syslog.hpp"

namespace atom::log {
SyslogWrapper::SyslogWrapper(LogLevel logLevel, const std::string &target)
    : m_target(target), m_logLevel(logLevel), m_exitThread(false) {
#ifdef _WIN32
    if (target == "Event") {
        m_eventHandle = RegisterEventSourceW(nullptr, L"SyslogWrapper");
        if (!m_eventHandle) {
            throw std::runtime_error("Failed to register event source");
        }
    } else if (target == "Console") {
    } else {
        m_logFile.open(target, std::ios::out | std::ios::app);
        if (!m_logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: " + target);
        }
    }
#else
    if (target.empty() || target == "Syslog") {
        openlog(nullptr, LOG_PID, LOG_USER);
    } else {
        m_logFile.open(target, std::ios::out | std::ios::app);
        if (!m_logFile.is_open()) {
            throw std::runtime_error("Failed to open log file: " + target);
        }
    }
#endif
    m_logThread = std::thread(&SyslogWrapper::processLogQueue, this);
}

SyslogWrapper::~SyslogWrapper() {
    m_exitThread = true;
    if (m_logThread.joinable()) {
        m_logThread.join();
    }
#ifdef _WIN32
    if (m_eventHandle) {
        DeregisterEventSource(m_eventHandle);
    }
#else
    closelog();
#endif
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

void SyslogWrapper::processLogQueue() {
    while (!m_exitThread || !m_logQueue.empty()) {
        std::string logString;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (!m_logQueue.empty()) {
                logString = m_logQueue.front();
                m_logQueue.pop();
            }
        }

        if (!logString.empty()) {
            writeLog(logString);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void SyslogWrapper::rotateLogFile() {
    if (m_logFile.is_open() && getLogFileSize() > 10 * 1024 * 1024) {  // 10 MB
        m_logFile.close();

        std::string newName = m_target + "." + formatTime();
        std::filesystem::rename(m_target, newName);

        m_logFile.open(m_target, std::ios::out | std::ios::app);
        if (!m_logFile.is_open()) {
            throw std::runtime_error("Failed to reopen log file: " + m_target);
        }
    }
}

std::size_t SyslogWrapper::getLogFileSize() const {
    if (m_logFile.is_open()) {
        // m_logFile.flush();
        return std::filesystem::file_size(m_target);
    }
    return 0;
}

std::string SyslogWrapper::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warning:
            return "WARNING";
        case LogLevel::Error:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void SyslogWrapper::writeLog(const std::string &logString) {
#ifdef _WIN32
    if (m_target == "Event") {
        LPCSTR strings[] = {logString.c_str()};
        ReportEventA(m_eventHandle, static_cast<WORD>(m_logLevel), 0, 0,
                     nullptr, 1, 0, strings, nullptr);
    } else if (m_target == "Console") {
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(consoleHandle, &csbi);
        WORD oldAttributes = csbi.wAttributes;
        SetConsoleTextAttribute(consoleHandle, static_cast<WORD>(m_logLevel));
        std::cout << logString << std::endl;
        SetConsoleTextAttribute(consoleHandle, oldAttributes);
    } else if (m_logFile.is_open()) {
        m_logFile << logString << std::endl;
        rotateLogFile();
    } else {
        OutputDebugStringA(logString.c_str());
    }
#else
    if (m_target.empty()) {
        syslog(static_cast<int>(m_logLevel), "%s", logString.c_str());
    } else if (m_logFile.is_open()) {
        m_logFile << logString << std::endl;
        rotateLogFile();
    } else {
        std::cout << logString << std::endl;
    }
#endif
}

std::string SyslogWrapper::formatLogMessage(LogLevel level,
                                            fmt::string_view format,
                                            fmt::format_args args) {
    std::string logMessage = fmt::vformat(format, args);
    std::string timeStr = formatTime();
    std::string logString = fmt::format("[{}] [{}] {}", timeStr,
                                        logLevelToString(level), logMessage);
    return logString;
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
}  // namespace atom::log
