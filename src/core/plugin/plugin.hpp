/*
 * plugin.hpp
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

Date: 2023-8-6

Description: Basic Plugin Definition

**************************************************/

#pragma once

#include <string>
#include <memory>
#include <vector>

#include "server/commander.hpp"
#include "server/variables.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

#define SETVAR_STR(name, value)\
    m_VariableRegistry->RegisterVariable<std::string>(name);\
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_INT(name, value)\
    m_VariableRegistry->RegisterVariable<int>(name);\
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_BOOL(name, value)\
    m_VariableRegistry->RegisterVariable<bool>(name);\
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_JSON(name, value)\
    m_VariableRegistry->RegisterVariable<json>(name);\
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_DOUBLE(name, value)\
    m_VariableRegistry->RegisterVariable<double>(name);\
    m_VariableRegistry->SetVariable(name, value);

class Plugin : public std::enable_shared_from_this<Plugin>
{
public:
    /**
     * @brief Constructs a new Plugin object.
     *
     * @param path The path of the plugin file.
     * @param version The version number of the plugin.
     * @param author The author of the plugin.
     * @param description A brief description of the plugin.
     */
    Plugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description);

    /**
     * @brief Destroys the Plugin object.
     */
    virtual ~Plugin();

    /**
     * @brief Gets the information about the plugin.
     *
     * @return The plugin information in JSON format.
     */
    json GetPluginInfo() const;

    /**
     * @brief Gets the path of the plugin.
     *
     * @return The path of the plugin.
     */
    std::string GetPath() const;

    /**
     * @brief Gets the version of the plugin.
     *
     * @return The version of the plugin.
     */
    std::string GetVersion() const;

    /**
     * @brief Gets the author of the plugin.
     *
     * @return The author of the plugin.
     */
    std::string GetAuthor() const;

    /**
     * @brief Gets the brief description of the plugin.
     *
     * @return A brief description of the plugin.
     */
    std::string GetDescription() const;

    /**
     * @brief Registers a member function with a specific name and handler.
     *
     * This function allows plugins to register member functions that can be called by other plugins.
     *
     * @tparam ClassType The type of the class that owns the handler function.
     * @param name The name of the function.
     * @param handler The handler function for the function.
     * @param object The object instance that owns the handler function.
     */
    template <typename T>
    bool RegisterVariable(const std::string &name)
    {
        return m_VariableRegistry->RegisterVariable<T>(name);
    }

    /**
     * @brief Sets the value of the variable with the specified name.
     *
     * @tparam T The type of the variable.
     * @param name The name of the variable.
     * @param value The value to set.
     * @return True if the variable was set successfully, false otherwise.
     */
    template <typename ClassType>
    void RegisterFunc(const std::string &name, void (ClassType::*handler)(const json &), ClassType *object)
    {
        m_CommandDispatcher->RegisterMemberHandler(name, object, handler);
    }

    /**
     * @brief Gets the value of the variable with the specified name.
     *
     * @tparam T The type of the variable.
     * @param name The name of the variable.
     * @return An optional containing the value of the variable, or empty if the variable does not exist.
     */
    template <typename T>
    bool SetVariable(const std::string &name, T &&value)
    {
        return m_VariableRegistry->SetVariable<T>(name, value);
    }

    template <typename T>
    std::optional<T> GetVariable(const std::string &name) const
    {
        return m_VariableRegistry->GetVariable<T>(name);
    }

    /**
     * @brief Adds an observer for the variable with the specified name.
     *
     * This function allows plugins to listen for changes to a registered variable.
     *
     * @param name The name of the variable.
     * @param observer The observer function.
     */
    void AddObserver(const std::string &name, const VariableRegistry::Observer &observer)
    {
        m_VariableRegistry->AddObserver(name, observer);
    }

    /**
     * @brief Removes an observer for the variable with the specified name.
     *
     * This function allows plugins to stop listening for changes to a registered variable.
     *
     * @param name The name of the variable.
     * @param observer The observer function.
    */
    void RemoveObserver(const std::string &name, const std::string &observer)
    {
        m_VariableRegistry->RemoveObserver(name, observer);
    }

    /**
     * @brief Runs the function with the specified name and parameters.
     *
     * This function calls a registered member function with the specified name and parameters.
     *
     * @param name The name of the function.
     * @param params The parameters for the function.
     * @return True if the function was executed successfully, false otherwise.
     */
    bool RunFunc(const std::string &name, const json &params);

    /**
     * @brief Runs the functions with the specified names and parameters.
     *
     * This function calls multiple registered member functions with the specified names and parameters.
     *
     * @param name The names of the functions.
     * @param params The parameters for the functions.
     * @return True if all functions were executed successfully, false otherwise.
     */
    bool RunFunc(const std::vector<std::string> &name, const std::vector<json> &params);

    /**
     * @brief Gets the information about the function with the specified name.
     *
     * @param name The name of the function.
     * @return The function information in JSON format.
     */
    json GetFuncInfo(const std::string &name);

private:
    std::string path_;        ///< The path of the plugin file.
    std::string version_;     ///< The version number of the plugin.
    std::string author_;      ///< The author of the plugin.
    std::string description_; ///< A brief description of the plugin.

    std::unique_ptr<CommandDispatcher<void, json>> m_CommandDispatcher; ///< The command dispatcher for handling functions.
    std::unique_ptr<VariableRegistry> m_VariableRegistry;               ///< The variable registry for managing variables.
};
