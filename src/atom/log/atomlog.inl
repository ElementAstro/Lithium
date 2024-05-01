/*
 * atomlog.inl
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-10

Description: Logger for Atom

**************************************************/

#ifndef ATOM_LOG_ATOMLOG_INL
#define ATOM_LOG_ATOMLOG_INL

#include "atomlog.hpp"

namespace atom::log {
template <typename... Args>
void Logger::trace(const std::string& format, Args&&... args) {
    log(LogLevel::TRACE,
        fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
}

template <typename... Args>
void Logger::debug(const std::string& format, Args&&... args) {
    log(LogLevel::DEBUG,
        fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
}

template <typename... Args>
void Logger::info(const std::string& format, Args&&... args) {
    log(LogLevel::INFO,
        fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
}

template <typename... Args>
void Logger::warn(const std::string& format, Args&&... args) {
    log(LogLevel::WARN,
        fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
}

template <typename... Args>
void Logger::error(const std::string& format, Args&&... args) {
    log(LogLevel::ERROR,
        fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
}

template <typename... Args>
void Logger::critical(const std::string& format, Args&&... args) {
    log(LogLevel::CRITICAL,
        fmt::format(fmt::runtime(format), std::forward<Args>(args)...));
}
}  // namespace atom::log

#endif