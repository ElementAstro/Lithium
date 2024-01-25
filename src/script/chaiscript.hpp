/*
 * chaiscript.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-13

Description: Script Manager

**************************************************/

#pragma once

#include <memory>
#include <vector>

#include "atom/server/message_bus.hpp"

namespace chaiscript
{
    class ChaiScript;
}

namespace Lithium
{
    /**
 * @class ScriptManager
 * @brief A class that manages script execution using ChaiScript.
 */
class ScriptManager
{
public:
    /**
     * @brief Constructor for ScriptManager.
     * @param messageBus The shared pointer to the MessageBus object.
     */
    ScriptManager(std::shared_ptr<MessageBus> messageBus);

    /**
     * @brief Destructor for ScriptManager.
     */
    ~ScriptManager();

    /**
     * @brief Creates a shared pointer to a new instance of ScriptManager.
     * @param messageBus The shared pointer to the MessageBus object.
     * @return A shared pointer to ScriptManager.
     */
    static std::shared_ptr<ScriptManager> createShared(std::shared_ptr<MessageBus> messageBus);

    /**
     * @brief Initializes the ScriptManager by adding function bindings to ChaiScript.
     */
    void Init();

    /**
     * @brief Initializes sub-modules and adds additional function bindings to ChaiScript.
     */
    void InitSubModules();

    /**
     * @brief Initializes the ScriptManager and ChaiScript, and adds additional function bindings.
     */
    void InitMyApp();

    /**
     * @brief Loads a script file and executes its contents.
     * @param filename The name of the script file to load.
     * @return True if the script file is loaded and executed successfully, false otherwise.
     */
    bool loadScriptFile(const std::string &filename);

    /**
     * @brief Loads a script file and returns its content as a string.
     * @param filename The name of the script file to load.
     * @return The content of the script file as a string, or an empty string if the file cannot be opened.
     */
    std::string loadScriptFileContent(const std::string &filename);

    /**
     * @brief Unloads a script file by executing its contents again.
     * @param filename The name of the script file to unload.
     * @return True if the script file is unloaded successfully, false otherwise.
     */
    bool unloadScriptFile(const std::string &filename);

    /**
     * @brief Executes a single command in the ChaiScript environment.
     * @param command The command to execute.
     * @return True if the command is executed successfully, false otherwise.
     */
    bool runCommand(const std::string &command);

    /**
     * @brief Executes multiple commands in the ChaiScript environment.
     * @param commands A vector of commands to execute.
     * @return True if all commands are executed successfully, false otherwise.
     */
    bool runMultiCommand(const std::vector<std::string> &commands);

    /**
     * @brief Runs a script file in the ChaiScript environment.
     * @param filename The name of the script file to run.
     * @return True if the script file is run successfully, false otherwise.
     */
    bool runScript(const std::string &filename);

private:
    std::unique_ptr<chaiscript::ChaiScript> chai_; ///< The unique pointer to the ChaiScript object.
    std::shared_ptr<MessageBus> m_MessageBus;     ///< The shared pointer to the MessageBus object.
};


}
