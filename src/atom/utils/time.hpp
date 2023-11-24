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

Date: 2023-10-27

Description: Some useful functions about time

**************************************************/

#pragma once

#include <string>

/**
 * @brief 获取当前时间的字符串时间戳
 * 
 * @return std::string 返回格式为"%Y-%m-%d %H:%M:%S"的字符串时间戳
 */
std::string GetTimestampString();

/**
 * @brief 将UTC时间转换为东八区时间
 * 
 * @param utcTimeStr UTC时间字符串，格式为"%Y-%m-%d %H:%M:%S"
 * @return std::string 东八区时间字符串，格式为"%Y-%m-%d %H:%M:%S"
 */
std::string ConvertToChinaTime(const std::string& utcTimeStr);

/**
 * @brief 获取当前时间的东八区字符串时间戳
 * 
 * @return std::string 返回格式为"%Y-%m-%d %H:%M:%S"的东八区字符串时间戳
 */
std::string GetChinaTimestampString();
