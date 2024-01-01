/*
 * device_utils.hpp
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

Date: 2023-3-29

Description: Device Utilities

**************************************************/

#pragma once

#include <string>

/**
 * @brief Execute a command and return the output.
 * @param cmd The command to execute.
 * @return The output of the command.
 * @note The output is in UTF-8.
*/
[[nodiscard]] std::string executeCommand(const std::string &cmd);

/**
 * @brief Check if the string is a time format.
 * @param str The string to check.
 * @return If the string is a time format, true, otherwise false.
*/
bool checkTimeFormat(const std::string &str);

/**
 * @brief Convert the number to time format.
 * @param num The number to convert.
 * @return The time format of the number.
*/
std::string convertToTimeFormat(int num);

/**
 * @brief Check if the string is a number.
 * @param str The string to check.
 * @return If the string is a number, true, otherwise false.
*/
bool checkDigits(const std::string &str);