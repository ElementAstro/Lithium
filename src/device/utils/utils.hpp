/*
 * utils.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Device Utilities

**************************************************/

#pragma once

#include <string>

namespace lithium
{
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
    [[nodiscard]] std::string convertToTimeFormat(int num);

    /**
     * @brief Check if the string is a number.
     * @param str The string to check.
     * @return If the string is a number, true, otherwise false.
     */
    bool checkDigits(const std::string &str);
}
