/*
 * error_stack.hpp
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

Description: Error Stack

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <ostream>
#include <algorithm>

/**
 * @brief Error information structure.
 */
struct ErrorInfo
{
    std::string errorMessage; /**< Error message. */
    std::string moduleName;   /**< Module name. */
    time_t timestamp;         /**< Timestamp of the error. */
};

/**
 * @brief Overloaded stream insertion operator to print ErrorInfo object.
 * @param os Output stream.
 * @param error ErrorInfo object to be printed.
 * @return Reference to the output stream.
 */
std::ostream &operator<<(std::ostream &os, const ErrorInfo &error);

/**
 * @brief Overloaded string concatenation operator to concatenate ErrorInfo object with a string.
 * @param str Input string.
 * @param error ErrorInfo object to be concatenated.
 * @return Concatenated string.
 */
std::string operator<<(const std::string &str, const ErrorInfo &error);

/**
 * @brief Error stack class for managing and handling errors.
 */
class ErrorStack
{
public:
    /**
     * @brief Default constructor for ErrorStack.
     */
    ErrorStack();

    /**
     * @brief Insert an error into the stack.
     * @param errorMessage Error message.
     * @param moduleName Module name where the error occurred.
     */
    void InsertError(const std::string &errorMessage, const std::string &moduleName);

    /**
     * @brief Set the filtered modules for printing the error stack.
     * @param modules List of module names to filter.
     */
    void SetFilteredModules(const std::vector<std::string> &modules);

    /**
     * @brief Clear the filtered modules.
     */
    void ClearFilteredModules();

    /**
     * @brief Print the filtered error stack.
     */
    void PrintFilteredErrorStack() const;

    /**
     * @brief Get the filtered errors by module.
     * @param moduleName Module name to filter.
     * @return Vector of ErrorInfo objects filtered by module.
     */
    std::vector<ErrorInfo> GetFilteredErrorsByModule(const std::string &moduleName) const;

    /**
     * @brief Insert an error into the stack and compress duplicate errors.
     * @param errorMessage Error message.
     * @param moduleName Module name where the error occurred.
     */
    void InsertErrorCompressed(const std::string &errorMessage, const std::string &moduleName);

    /**
     * @brief Get the compressed errors as a string.
     * @return Compressed errors as a string.
     */
    std::string GetCompressedErrors() const;

private:
    std::vector<ErrorInfo> errorStack;           /**< Error stack to store all errors. */
    std::vector<std::string> filteredModules;    /**< Filtered modules for printing the error stack. */
    std::vector<ErrorInfo> compressedErrorStack; /**< Compressed error stack with unique errors only. */

    /**
     * @brief Update the compressed error stack by removing duplicate errors.
     */
    void UpdateCompressedErrors();

    /**
     * @brief Sort the compressed error stack based on timestamp.
     */
    void SortCompressedErrorStack();
};
