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

/**
 * @brief Enum class representing the log levels.
 */
enum class LogLevel {
    TRACE,     ///< Trace level logging.
    DEBUG,     ///< Debug level logging.
    INFO,      ///< Info level logging.
    WARN,      ///< Warn level logging.
    ERROR,     ///< Error level logging.
    CRITICAL,  ///< Critical level logging.
    OFF        ///< Used to disable logging.
};

class LoggerImpl;  // Forward declaration

/**
 * @brief Logger class for logging messages with different severity levels.
 */
class Logger {
public:
    /**
     * @brief Constructs a Logger object.
     * @param file_name The name of the log file.
     * @param min_level The minimum log level to log.
     * @param max_file_size The maximum size of the log file in bytes.
     * @param max_files The maximum number of log files to keep.
     */
    explicit Logger(const fs::path& file_name,
                    LogLevel min_level = LogLevel::TRACE,
                    size_t max_file_size = 1048576, int max_files = 10);

    /**
     * @brief Destructor for the Logger object.
     */
    ~Logger();

    // Delete copy constructor and copy assignment operator
    Logger(const Logger&) = delete;
    auto operator=(const Logger&) -> Logger& = delete;

    /**
     * @brief Logs a trace level message.
     * @tparam Args The types of the arguments.
     * @param format The format string.
     * @param args The arguments to format.
     */
    template <typename... Args>
    void trace(const std::string& format, const Args&... args) {
        log(LogLevel::TRACE,
            std::vformat(format, std::make_format_args(args...)));
    }

    /**
     * @brief Logs a debug level message.
     * @tparam Args The types of the arguments.
     * @param format The format string.
     * @param args The arguments to format.
     */
    template <typename... Args>
    void debug(const std::string& format, const Args&... args) {
        log(LogLevel::DEBUG,
            std::vformat(format, std::make_format_args(args...)));
    }

    /**
     * @brief Logs an info level message.
     * @tparam Args The types of the arguments.
     * @param format The format string.
     * @param args The arguments to format.
     */
    template <typename... Args>
    void info(const std::string& format, const Args&... args) {
        log(LogLevel::INFO,
            std::vformat(format, std::make_format_args(args...)));
    }

    /**
     * @brief Logs a warn level message.
     * @tparam Args The types of the arguments.
     * @param format The format string.
     * @param args The arguments to format.
     */
    template <typename... Args>
    void warn(const std::string& format, const Args&... args) {
        log(LogLevel::WARN,
            std::vformat(format, std::make_format_args(args...)));
    }

    /**
     * @brief Logs an error level message.
     * @tparam Args The types of the arguments.
     * @param format The format string.
     * @param args The arguments to format.
     */
    template <typename... Args>
    void error(const std::string& format, const Args&... args) {
        log(LogLevel::ERROR,
            std::vformat(format, std::make_format_args(args...)));
    }

    /**
     * @brief Logs a critical level message.
     * @tparam Args The types of the arguments.
     * @param format The format string.
     * @param args The arguments to format.
     */
    template <typename... Args>
    void critical(const std::string& format, const Args&... args) {
        log(LogLevel::CRITICAL,
            std::vformat(format, std::make_format_args(args...)));
    }

    /**
     * @brief Sets the logging level.
     * @param level The log level to set.
     */
    void setLevel(LogLevel level);

    /**
     * @brief Sets the logging pattern.
     * @param pattern The pattern to set.
     */
    void setPattern(const std::string& pattern);

    /**
     * @brief Sets the thread name for logging.
     * @param name The thread name to set.
     */
    void setThreadName(const std::string& name);

    /**
     * @brief Registers a sink logger.
     * @param logger The logger to register as a sink.
     */
    void registerSink(const std::shared_ptr<Logger>& logger);

    /**
     * @brief Removes a sink logger.
     * @param logger The logger to remove as a sink.
     */
    void removeSink(const std::shared_ptr<Logger>& logger);

    /**
     * @brief Clears all registered sink loggers.
     */
    void clearSinks();

    /**
     * @brief Enables or disables system logging.
     * @param enable True to enable system logging, false to disable.
     */
    void enableSystemLogging(bool enable);

private:
    std::shared_ptr<LoggerImpl>
        impl_;  ///< Pointer to the Logger implementation.

    /**
     * @brief Logs a message with a specified log level.
     * @param level The log level.
     * @param msg The message to log.
     */
    void log(LogLevel level, const std::string& msg);
};

}  // namespace atom::log

#endif  // ATOM_LOG_ATOMLOG_HPP