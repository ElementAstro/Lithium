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

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_set>

#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

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

/**
 * @brief The Logger class provides functionality for logging messages to a file
 * with different log levels and patterns.
 *
 * The Logger class allows logging messages to a file with various log levels
 * such as TRACE, DEBUG, INFO, WARNING, ERROR, and CRITICAL. It supports logging
 * messages with different formats and rotating log files when the file size
 * exceeds a certain threshold. Additionally, it provides options to set log
 * level, log pattern, thread name, and register sinks for logging to multiple
 * destinations.
 */
class Logger {
public:
    /**
     * @brief Constructs a Logger object.
     *
     * Initializes the Logger with the specified file name, minimum log level,
     * maximum file size, and maximum number of log files.
     *
     * @param file_name The path to the log file.
     * @param min_level The minimum log level to be logged.
     * @param max_file_size The maximum size of the log file in bytes before
     * rotation.
     * @param max_files The maximum number of log files to retain during
     * rotation.
     */
    explicit Logger(const fs::path& file_name,
                    LogLevel min_level = LogLevel::TRACE,
                    size_t max_file_size = 1048576, int max_files = 10);

    /**
     * @brief Destroys the Logger object.
     *
     * Closes the log file and stops the logger thread.
     */
    ~Logger();

    /**
     * @brief Prevents copying of Logger objects.
     */
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Logs a message with the TRACE log level.
     *
     * Logs a message with the TRACE log level and format. Supports variadic
     * arguments for message formatting.
     *
     * @tparam Args Variadic template for message arguments.
     * @param format The format string of the message.
     * @param args The arguments for message formatting.
     */
    template <typename... Args>
    void trace(const std::string& format, Args&&... args);

    /**
     * @brief Logs a message with the DEBUG log level.
     *
     * Logs a message with the DEBUG log level and format. Supports variadic
     * arguments for message formatting.
     *
     * @tparam Args Variadic template for message arguments.
     * @param format The format string of the message.
     * @param args The arguments for message formatting.
     */
    template <typename... Args>
    void debug(const std::string& format, Args&&... args);

    /**
     * @brief Logs a message with the INFO log level.
     *
     * Logs a message with the INFO log level and format. Supports variadic
     * arguments for message formatting.
     *
     * @tparam Args Variadic template for message arguments.
     * @param format The format string of the message.
     * @param args The arguments for message formatting.
     */
    template <typename... Args>
    void info(const std::string& format, Args&&... args);

    /**
     * @brief Logs a message with the WARN log level.
     *
     * Logs a message with the WARN log level and format. Supports variadic
     * arguments for message formatting.
     *
     * @tparam Args Variadic template for message arguments.
     * @param format The format string of the message.
     * @param args The arguments for message formatting.
     */
    template <typename... Args>
    void warn(const std::string& format, Args&&... args);

    /**
     * @brief Logs a message with the ERROR log level.
     *
     * Logs a message with the ERROR log level and format. Supports variadic
     * arguments for message formatting.
     *
     * @tparam Args Variadic template for message arguments.
     * @param format The format string of the message.
     * @param args The arguments for message formatting.
     */
    template <typename... Args>
    void error(const std::string& format, Args&&... args);

    /**
     * @brief Logs a message with the CRITICAL log level.
     *
     * Logs a message with the CRITICAL log level and format. Supports variadic
     * arguments for message formatting.
     *
     * @tparam Args Variadic template for message arguments.
     * @param format The format string of the message.
     * @param args The arguments for message formatting.
     */
    template <typename... Args>
    void critical(const std::string& format, Args&&... args);

    /**
     * @brief Sets the minimum log level for logging.
     *
     * Sets the minimum log level for logging messages. Log messages with levels
     * below this will not be logged.
     *
     * @param level The minimum log level.
     */
    void setLevel(LogLevel level);

    /**
     * @brief Sets the log message pattern.
     *
     * Sets the pattern for formatting log messages. The pattern may include
     * placeholders for various log components.
     *
     * @param pattern The log message pattern.
     */
    void setPattern(const std::string& pattern);

    /**
     * @brief Sets the name of the current thread.
     *
     * Sets the name of the current thread to be included in log messages.
     *
     * @param name The name of the thread.
     */
    void setThreadName(const std::string& name);

    /**
     * @brief Registers a sink for logging to another Logger.
     *
     * Registers another Logger instance as a sink for logging, allowing
     * messages to be forwarded to multiple destinations.
     *
     * @param logger The Logger instance to register as a sink.
     */
    void registerSink(const std::shared_ptr<Logger>& logger);

    /**
     * @brief Removes a previously registered sink.
     *
     * Removes a previously registered Logger instance from the list of sinks.
     *
     * @param logger The Logger instance to remove from sinks.
     */
    void removeSink(const std::shared_ptr<Logger>& logger);

    /**
     * @brief Clears all registered sinks.
     *
     * Clears all registered Logger instances from the list of sinks.
     */
    void clearSinks();

private:
    fs::path file_name;
    std::ofstream log_file;
    std::queue<std::string> log_queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
    bool finished = false;
    std::jthread worker;
    size_t max_file_size;
    int max_files;
    LogLevel min_level;
    int file_index = 0;
    std::unordered_map<std::thread::id, std::string> thread_names;
    std::string pattern = "[{Y}-{m}-{d} {H}:{M}:{S}.{e}] [{l}] [{t}] {v}";
    std::vector<std::shared_ptr<Logger>> sinks;

    /**
     * @brief Rotates the log file.
     *
     * Rotates the log file by renaming it and creating a new empty file.
     */
    void rotateLogFile();

    /**
     * @brief Gets the name of the current thread.
     *
     * Gets the name of the current thread, or a default name if no name has
     * been set.
     *
     * @return The name of the current thread.
     */
    std::string getThreadName();

    /**
     * @brief Converts a log level to a string.
     *
     * Converts a log level to a string representation.
     *
     * @param level The log level to convert.
     * @return The string representation of the log level.
     */
    static std::string logLevelToString(LogLevel level);

    /**
     * @brief Formats a log message.
     *
     * Formats a log message with the specified log level and message.
     *
     * @param level The log level of the message.
     * @param msg The message to format.
     * @return The formatted log message.
     */
    std::string formatMessage(LogLevel level, const std::string& msg);

    /**
     * @brief Logs a message.
     *
     * Logs a message with the specified log level and message.
     *
     * @param level The log level of the message.
     * @param msg The message to log.
     */
    void log(LogLevel level, const std::string& msg);

    /**
     * @brief The worker thread function.
     *
     * The worker thread function that processes log messages from the queue.
     */
    void run();
};

}  // namespace atom::log

#endif
