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

namespace Atom::Log
{
    SyslogWrapper::SyslogWrapper(LogLevel logLevel, const std::string &target) : m_logLevel(logLevel)
    {
#ifdef _WIN32
        if (target == "Event")
        {
            // 初始化Event句柄
            m_eventHandle = CreateEvent(nullptr, TRUE, FALSE, nullptr);
        }
        else if (target == "Console")
        {
            // 输出到控制台
        }
        else
        {
            // 输出到文件
        }
#else
        if (target.empty() || target == "Syslog")
        {
            // 在Linux中默认输出到syslog
            openlog(nullptr, LOG_PID, LOG_USER);
        }
        else
        {
            // 输出到文件
        }
#endif
    }

    SyslogWrapper::~SyslogWrapper()
    {
#ifdef _WIN32
        // 关闭Event句柄
        CloseHandle(m_eventHandle);
#else
        // 清理syslog
        closelog();
#endif
    }
}
