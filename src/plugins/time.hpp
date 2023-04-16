/*
 * time.hpp
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

Date: 2023-3-31

Description: Time

**************************************************/

#ifndef TIME_HPP
#define TIME_HPP

#include <ctime>
#include <string>

namespace OpenAPT
{
    namespace Time
    {

        /**
         * @brief Gets the current system time in seconds since January 1, 1970 (Unix epoch).
         *
         * @return The current system time.
         * @note This function retrieves the current system time in seconds since Unix epoch.
         * @note 该函数获取当前时间距Unix纪元以秒的形式表示的系统时间。
         */
        std::time_t getSystemTime();

        /**
         * @brief Sets the system time to a specified date and time.
         *
         * @param year The year.
         * @param month The month (1-12).
         * @param day The day of the month (1-31).
         * @param hour The hour (0-23).
         * @param minute The minute (0-59).
         * @param second The second (0-59).
         * @note This function sets the system time to the specified date and time.
         * @note 该函数将系统时间设置为指定的日期和时间。
         */
        void setSystemTime(int year, int month, int day, int hour, int minute, int second);

        /**
         * @brief Sets the system timezone.
         *
         * @param timezone The name of the timezone to set.
         * @return true if the timezone was set successfully, false otherwise.
         * @note This function sets the system timezone to the specified timezone.
         * @note 该函数将系统时区设置为指定的时区。
         */
        bool set_system_timezone(const std::string &timezone);

        /**
         * @brief Synchronizes the system time with an RTC (real-time clock) device.
         *
         * @return true if the synchronization was successful, false otherwise.
         * @note This function synchronizes the system time with an RTC device.
         * @note 该函数将系统时间与RTC设备同步。
         */
        bool SyncTimeFromRTC();

    } // namespace Time
} // namespace OpenAPT

#endif // TIME_HPP
