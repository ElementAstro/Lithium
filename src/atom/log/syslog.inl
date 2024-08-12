#ifndef ATOM_LOG_SYSLOG_INL
#define ATOM_LOG_SYSLOG_INL

#include "syslog.hpp"

namespace atom::log {

template <typename... Args>
void SyslogWrapper::log(LogLevel level, fmt::format_string<Args...> format,
                        Args &&...args) {
    if (static_cast<int>(level) < static_cast<int>(m_logLevel)) {
        return;
    }

    std::string logString =
        formatLogMessage(level, format, fmt::make_format_args(args...));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_logQueue.push(logString);
    }
}

template <typename... Args>
void SyslogWrapper::debug(fmt::format_string<Args...> format, Args &&...args) {
    log(LogLevel::Debug, format, std::forward<Args>(args)...);
}

template <typename... Args>
void SyslogWrapper::info(fmt::format_string<Args...> format, Args &&...args) {
    log(LogLevel::Info, format, std::forward<Args>(args)...);
}

template <typename... Args>
void SyslogWrapper::warning(fmt::format_string<Args...> format,
                            Args &&...args) {
    log(LogLevel::Warning, format, std::forward<Args>(args)...);
}

template <typename... Args>
void SyslogWrapper::error(fmt::format_string<Args...> format, Args &&...args) {
    log(LogLevel::Error, format, std::forward<Args>(args)...);
}

}  // namespace atom::log

#endif
