/*
 * daemon.hpp
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

Date: 2023-11-11

Description: Daemon thread implementation

**************************************************/

#pragma once

/**
 * @file process_info.h
 * @brief This file contains the definition of ProcessInfoMgr and ProcessInfo classes for managing process information.
 */

#include <sstream>
#include <functional>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

/**
 * @brief Converts timestamp to string in format "YYYY-MM-DD HH:MM:SS".
 * 
 * @param timestamp The timestamp to be converted.
 * @return The string representation of the timestamp in "YYYY-MM-DD HH:MM:SS" format.
 */
std::string Time2Str(time_t timestamp);

/**
 * @brief Singleton class for managing process information.
 */
class ProcessInfoMgr
{
public:
    /**
     * @brief Gets the singleton instance of ProcessInfoMgr.
     * 
     * @return Pointer to the singleton instance of ProcessInfoMgr.
     */
    static ProcessInfoMgr *GetInstance();

    int parent_id = 0; /**< The parent process ID. */
    int main_id = 0; /**< The main process ID. */
    time_t parent_start_time = 0; /**< The start time of the parent process. */
    time_t main_start_time = 0; /**< The start time of the main process. */
    int restart_count = 0; /**< The number of times the main process has been restarted. */
};

/**
 * @brief Class for managing process information and starting the process.
 */
class ProcessInfo
{
public:
    /**
     * @brief Returns a string representation of the process information.
     * 
     * @return The string representation of the process information.
     */
    std::string toString() const;

    /**
     * @brief Starts the process without daemonizing it.
     * 
     * @param argc The number of command-line arguments.
     * @param argv The array of command-line arguments.
     * @param main_cb The callback function to be executed by the main process.
     * @return The exit code of the main process.
     */
    static int real_start(int argc, char **argv,
                          std::function<int(int argc, char **argv)> main_cb);

    /**
     * @brief Starts the process and daemonizes it.
     * 
     * @param argc The number of command-line arguments.
     * @param argv The array of command-line arguments.
     * @param main_cb The callback function to be executed by the main process.
     * @return The exit code of the main process.
     */
    static int real_daemon(int argc, char **argv,
                           std::function<int(int argc, char **argv)> main_cb);

    /**
     * @brief Starts the process with or without daemonizing it.
     * 
     * @param argc The number of command-line arguments.
     * @param argv The array of command-line arguments.
     * @param main_cb The callback function to be executed by the main process.
     * @param is_daemon Whether to daemonize the process or not.
     * @return The exit code of the main process.
     */
    int start_daemon(int argc, char **argv,
                     std::function<int(int argc, char **argv)> main_cb,
                     bool is_daemon);
};

