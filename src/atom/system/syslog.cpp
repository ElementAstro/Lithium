/*
 * syslog.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-6-17

Description: Simple syslog wrapper for Windows and Linux

**************************************************/

#include "syslog.hpp"

namespace Atom::System
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

/*
int main() {
    SyslogWrapper logger(LogLevel::Debug, "Event");

    logger.log(LogLevel::Debug, "This is a debug message.");
    logger.log(LogLevel::Info, "This is an info message.");
    logger.log(LogLevel::Warning, "This is a warning message.");
    logger.log(LogLevel::Error, "This is an error message.");

    return 0;
}
*/
