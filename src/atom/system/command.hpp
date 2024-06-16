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

#include <functional>
#include <unordered_map>
#include <string>

namespace atom::system {

/**
 * @brief Execute a command and return the command output as a string.
 *
 * @param command The command to execute.
 * @param openTerminal Whether to open a terminal window for the command.
 * @param processLine A callback function to process each line of output.
 * @return The output of the command as a string.
 *
 * @note The function throws a std::runtime_error if the command fails to
 * execute.
 */
[[nodiscard]] std::string executeCommand(
    const std::string &command, bool openTerminal = false,
    std::function<void(const std::string &)> processLine =
        [](const std::string &) {});

/**
 * @brief Execute a list of commands.
 *
 * @param commands The list of commands to execute.
 *
 * @note The function throws a std::runtime_error if any of the commands fail to
 * execute.
 */
void executeCommands(const std::vector<std::string> &commands);

/**
 * @brief Kill a process by its name.
 *
 * @param processName The name of the process to kill.
 */
void killProcessByName(const std::string &processName, int signal);

/**
 * @brief Kill a process by its PID.
 *
 * @param pid The PID of the process to kill.
 */
void killProcessByPID(int pid, int signal);

/**
 * @brief Execute a command with environment variables and return the command
 * output as a string.
 *
 * @param command The command to execute.
 * @param envVars The environment variables as a map of variable name to value.
 * @return The output of the command as a string.
 *
 * @note The function throws a std::runtime_error if the command fails to
 * execute.
 */
[[nodiscard]] std::string executeCommandWithEnv(
    const std::string &command,
    const std::unordered_map<std::string, std::string> &envVars);

/**
 * @brief Execute a command and return the command output along with the exit
 * status.
 *
 * @param command The command to execute.
 * @return A pair containing the output of the command as a string and the exit
 * status as an integer.
 *
 * @note The function throws a std::runtime_error if the command fails to
 * execute.
 */
[[nodiscard]] std::pair<std::string, int> executeCommandWithStatus(
    const std::string &command);
}  // namespace atom::system

#endif
