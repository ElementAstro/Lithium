/*
 * time.cpp
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

Date: 2023-10-27

Description: Some useful functions about time

**************************************************/

#include "time.hpp"

#include <iomanip>
#include <sstream>
#include <chrono>

namespace Atom::Utils
{
    std::string GetTimestampString()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << ms.count();

        return ss.str();
    }

    std::string ConvertToChinaTime(const std::string &utcTimeStr)
    {
        // 解析UTC时间字符串
        std::tm tm = {};
        std::istringstream iss(utcTimeStr);
        iss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

        // 转换为时间点
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

        // 转换为东八区时间
        std::chrono::hours offset(8);
        auto local_tp = std::chrono::time_point_cast<std::chrono::hours>(tp) + offset;

        // 格式化为字符串
        auto local_time = std::chrono::system_clock::to_time_t(local_tp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&local_time), "%Y-%m-%d %H:%M:%S");

        return ss.str();
    }

    std::string GetChinaTimestampString()
    {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();

        // 转换为东八区时间点
        std::chrono::hours offset(8);
        auto local_tp = std::chrono::time_point_cast<std::chrono::hours>(now) + offset;

        // 格式化为字符串
        auto local_time = std::chrono::system_clock::to_time_t(local_tp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&local_time), "%Y-%m-%d %H:%M:%S");

        return ss.str();
    }

    std::string TimeStampToString(time_t timestamp)
    {
        char buffer[80];
        std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", std::localtime(&timestamp));
        return std::string(buffer);
    }
}
