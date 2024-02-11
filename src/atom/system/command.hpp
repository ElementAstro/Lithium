/*
 * command.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-24

Description: Simple wrapper for executing commands.

**************************************************/

#ifndef ATOM_SYSTEM_COMMAND_HPP
#define ATOM_SYSTEM_COMMAND_HPP

#include <string>
#include <map>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/types.h>
#endif

namespace Atom::System
{
#ifdef _WIN32
    struct ProcessHandle
    {
        HANDLE handle;
    };
#else
    struct ProcessHandle
    {
        pid_t pid;
    };
#endif

    /**
     * @brief Execute a command and return the command output as a string.
     *
     * @param command The command to execute.
     * @return The output of the command as a string.
     *
     * @note The function throws a std::runtime_error if the command fails to execute.
     */
    [[nodiscard]] std::string executeCommand(const std::string &command, bool openTerminal);

    /**
     * @brief Execute a list of commands.
     *
     * @param commands The list of commands to execute.
     *
     * @note The function throws a std::runtime_error if any of the commands fail to execute.
     */
    void executeCommands(const std::vector<std::string> &commands);

    /**
     * @brief Execute a command and return the process handle.
     *
     * @param command The command to execute.
     * @return The handle of the process.
     *
     * @note The function throws a std::runtime_error if the command fails to execute.
     */
    [[nodiscard]] ProcessHandle executeCommand(const std::string &command);

    /**
     * @brief Kill a process.
     *
     * @param handle The handle of the process to kill.
     *
     * @note The function throws a std::runtime_error if the command fails to execute.
     */
    void killProcess(const ProcessHandle &handle);

    /**
     * @brief Execute a command with environment variables and return the command output as a string.
     *
     * @param command The command to execute.
     * @param envVars The environment variables as a map of variable name to value.
     * @return The output of the command as a string.
     *
     * @note The function throws a std::runtime_error if the command fails to execute.
     */
    [[nodiscard]] std::string executeCommandWithEnv(const std::string &command, const std::map<std::string, std::string> &envVars);

    /**
     * @brief Execute a command and return the command output along with the exit status.
     *
     * @param command The command to execute.
     * @return A pair containing the output of the command as a string and the exit status as an integer.
     *
     * @note The function throws a std::runtime_error if the command fails to execute.
     */
    [[nodiscard]] std::pair<std::string, int> executeCommandWithStatus(const std::string &command);
}

#endif