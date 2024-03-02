/*
 * sheller.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: System Script Manager

**************************************************/

#pragma once

#include <cstdlib>
#include <ctime>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

using Script = std::string;
#if ENABLE_FASTHASH
using ScriptMap = emhash8::HashMap<std::string, Script>;
#else
using ScriptMap = std::unordered_map<std::string, Script>;
#endif

namespace Lithium {
/**
 * @brief Manages the registration, execution, and monitoring of scripts.
 */
class ScriptManager {
private:
    ScriptMap scripts; /**< Map to store registered scripts. */
    std::unordered_map<std::string, Script>
        powerShellScripts; /**< Map to store registered PowerShell scripts. */
    std::unordered_map<std::string, std::string>
        scriptOutputs; /**< Map to store outputs of executed scripts. */
    std::unordered_map<std::string, int>
        scriptStatus; /**< Map to store status codes of executed scripts. */

public:
    /**
     * @brief Registers a script.
     * @param name The name of the script.
     * @param script The script to register.
     */
    void RegisterScript(const std::string &name, const Script &script);

    /**
     * @brief Registers a PowerShell script.
     * @param name The name of the script.
     * @param script The PowerShell script to register.
     */
    void RegisterPowerShellScript(const std::string &name,
                                  const Script &script);

    /**
     * @brief Displays all registered scripts.
     */
    void ViewScripts();

    /**
     * @brief Deletes a script.
     * @param name The name of the script to delete.
     */
    void DeleteScript(const std::string &name);

    /**
     * @brief Updates a script.
     * @param name The name of the script to update.
     * @param script The updated script.
     */
    void UpdateScript(const std::string &name, const Script &script);

    /**
     * @brief Runs a script.
     * @param name The name of the script to run.
     * @param args Additional command-line arguments for the script (optional).
     * @return True if the script was executed successfully, false otherwise.
     */
    bool RunScript(const std::string &name,
                   const std::vector<std::string> &args = {});

    /**
     * @brief Displays the output of a script.
     * @param name The name of the script.
     */
    void ViewScriptOutput(const std::string &name);

    /**
     * @brief Displays the status code of a script.
     * @param name The name of the script.
     */
    void ViewScriptStatus(const std::string &name);

private:
    /**
     * @brief Executes a command line.
     * @param cmdLine The command line to execute.
     * @param name The name of the script.
     * @param args Additional command-line arguments for the script.
     * @return True if the command was executed successfully, false otherwise.
     */
    bool RunCommand(const std::string &cmdLine, const std::string &name,
                    const std::vector<std::string> &args);

    /**
     * @brief Logs an error message.
     * @param message The error message to log.
     */
    void LogError(const std::string &message);

private:
    std::shared_mutex m_sharedMutex;
};
}  // namespace Lithium
