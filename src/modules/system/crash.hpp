/*
 * crash.hpp
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

Date: 2023-4-4

Description: Crash Report

**************************************************/

#pragma once

#include <string>

namespace Lithium::CrashReport
{
    /**
     * @brief 保存崩溃日志
     * @param error_msg 崩溃日志详细信息
     *
     * 该函数用于保存程序崩溃时的日志信息，方便后续调试和分析。
     *
     * @note 调用该函数前需要确保崩溃日志已经被记录下来，否则调用该函数不会有任何效果。
     *
     * @param error_msg The detailed information of the crash log.
     *
     * This function is used to save the log information when the program crashes,
     * which is helpful for further debugging and analysis.
     *
     * @note Make sure the crash log has been recorded before calling this function,
     * otherwise calling this function will have no effect.
     */
    void saveCrashLog(const std::string &error_msg);
} // namespace Lithium::CrashReport
