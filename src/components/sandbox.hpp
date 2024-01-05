/*
 * sandbox.hpp
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

Date: 2024-1-4

Description: A sandbox for alone componnents, such as executables.

**************************************************/

#pragma once

#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#include <sys/time.h>
#endif

namespace Lithium
{
    /**
     * @class Sandbox
     * @brief Represents a sandbox for running programs with specified time and memory limits.
     * @details This class is used to run programs with specified time and memory limits.
     * @note The implementation of this class on windows is not complete.
     * @note If we run this on windows, we can't create a sandbox properly.
     */
    class Sandbox
    {
    public:
        /**
         * @brief Constructor for Sandbox class.
         */
        Sandbox() {}

        /**
         * @brief Destructor for Sandbox class.
         */
        ~Sandbox() {}

        /**
         * @brief Sets the time limit for program execution.
         * @param timeLimitMs The time limit in milliseconds.
         * @return True if the time limit is set successfully, false otherwise.
         */
        bool setTimeLimit(int timeLimitMs);

        /**
         * @brief Sets the memory limit for program execution.
         * @param memoryLimitKb The memory limit in kilobytes.
         * @return True if the memory limit is set successfully, false otherwise.
         */
        bool setMemoryLimit(long memoryLimitKb);

        /**
         * @brief Sets the root directory for program execution.
         * @param rootDirectory The root directory path.
         * @return True if the root directory is set successfully, false otherwise.
         */
        bool setRootDirectory(const std::string &rootDirectory);

        /**
         * @brief Sets the user ID for program execution.
         * @param userId The user ID.
         * @return True if the user ID is set successfully, false otherwise.
         */
        bool setUserId(int userId);

        /**
         * @brief Sets the program path for execution.
         * @param programPath The program path.
         * @return True if the program path is set successfully, false otherwise.
         */
        bool setProgramPath(const std::string &programPath);

        /**
         * @brief Sets the program arguments for execution.
         * @param programArgs The program arguments.
         * @return True if the program arguments are set successfully, false otherwise.
         */
        bool setProgramArgs(const std::vector<std::string> &programArgs);

        /**
         * @brief Runs the program in the sandbox.
         * @return True if the program runs successfully, false otherwise.
         */
        bool run();

        /**
         * @brief Gets the time used by the program during execution.
         * @return The time used in milliseconds.
         */
        int getTimeUsed() const;

        /**
         * @brief Gets the memory used by the program during execution.
         * @return The memory used in kilobytes.
         */
        long getMemoryUsed() const;

    private:
        int m_timeLimit = 0;
        long m_memoryLimit = 0;
        std::string m_rootDirectory;
        int m_userId = 0;
        std::string m_programPath;
        std::vector<std::string> m_programArgs;
        int m_timeUsed = 0;
        long m_memoryUsed = 0;
    };

}
