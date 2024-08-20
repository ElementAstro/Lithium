/*
 * atomlog.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Logger for Atom

**************************************************/

#ifndef ATOM_LOG_ATOMLOG_HPP
#define ATOM_LOG_ATOMLOG_HPP

#include <filesystem>
#include <format>
#include <memory>
#include <string>

namespace fs = std::filesystem;

namespace atom::log {

enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
    OFF  // Used to disable logging
};

class LoggerImpl;  // Forward declaration

class Logger {
public:
    explicit Logger(const fs::path& file_name,
                    LogLevel min_level = LogLevel::TRACE,
                    size_t max_file_size = 1048576, int max_files = 10);

    ~Logger();
    Logger(const Logger&) = delete;
    auto operator=(const Logger&) -> Logger& = delete;

    template <typename... Args>
    void trace(const std::string& format, const Args&... args) {
        log(LogLevel::TRACE,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void debug(const std::string& format, const Args&... args) {
        log(LogLevel::DEBUG,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void info(const std::string& format, const Args&... args) {
        log(LogLevel::INFO,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void warn(const std::string& format, const Args&... args) {
        log(LogLevel::WARN,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void error(const std::string& format, const Args&... args) {
        log(LogLevel::ERROR,
            std::vformat(format, std::make_format_args(args...)));
    }

    template <typename... Args>
    void critical(const std::string& format, const Args&... args) {
        log(LogLevel::CRITICAL,
            std::vformat(format, std::make_format_args(args...)));
    }

    void setLevel(LogLevel level);

    void setPattern(const std::string& pattern);

    void setThreadName(const std::string& name);

    void registerSink(const std::shared_ptr<Logger>& logger);

    void removeSink(const std::shared_ptr<Logger>& logger);

    void clearSinks();

    // New: Enable or disable system logging
    void enableSystemLogging(bool enable);

private:
    std::shared_ptr<LoggerImpl> impl_;
    void log(LogLevel level, const std::string& msg);
};

}  // namespace atom::log

#endif  // ATOM_LOG_ATOMLOG_HPP
