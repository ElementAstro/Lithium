/*
 * connection.hpp
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

Date: 2023-12-26

Description: Basic Component Definition

**************************************************/

#pragma once

#define ATOM_COMPONENT_DECLARATION

#include <string>
#include <memory>
#include <vector>

#include "atom/server/commander.hpp"
#include "atom/server/variables.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

#define SETVAR_STR(name, value)                              \
    m_VariableRegistry->RegisterVariable<std::string>(name); \
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_INT(name, value)                      \
    m_VariableRegistry->RegisterVariable<int>(name); \
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_BOOL(name, value)                      \
    m_VariableRegistry->RegisterVariable<bool>(name); \
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_JSON(name, value)                      \
    m_VariableRegistry->RegisterVariable<json>(name); \
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_DOUBLE(name, value)                      \
    m_VariableRegistry->RegisterVariable<double>(name); \
    m_VariableRegistry->SetVariable(name, value);

class PackageInfo; // Forward declaration

class Component : public std::enable_shared_from_this<Component>
{
public:
    /**
     * @brief Constructs a new Component object.
     */
    explicit Component();

    /**
     * @brief Destroys the Component object.
     */
    virtual ~Component();

    /**
     * @brief Gets the information about the plugin.
     *
     * @return The plugin information in JSON format.
     */
    json GetComponentInfo() const;

    /**
     * @brief Gets the version of the plugin.
     *
     * @return The version of the plugin.
     */
    std::string GetVersion() const;

    /**
     * @brief Gets the name of the plugin.
     *
     * @return The name of the plugin.
     */
    std::string GetName() const;

    /**
     * @brief Initializes the plugin.
     *
     * @return True if the plugin was initialized successfully, false otherwise.
     * @note This function is called by the server when the plugin is loaded.
     * @note This function should be overridden by the plugin.
     */
    virtual bool Initialize();

    /**
     * @brief Destroys the plugin.
     *
     * @return True if the plugin was destroyed successfully, false otherwise.
     * @note This function is called by the server when the plugin is unloaded.
     * @note This function should be overridden by the plugin.
     * @note The plugin should not be used after this function is called.
     * @note This is for the plugin to release any resources it has allocated.
     */
    virtual bool Destroy();

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
     * @brief Gets the information about the function with the specified name.
     *
     * @param name The name of the function.
     * @return The information about the function in JSON format.
     */
    json Component::GetFuncInfo(const std::string &name);

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

    /**
     * @brief Gets the value of the variable with the specified name.
     *
     * @tparam T The type of the variable.
     * @param name The name of the variable.
     * @return An optional containing the value of the variable, or empty if the variable does not exist
     * @note When you try to get a variable that does not exist, an exception will be thrown
     * @note This function is thread-safe
     * @note This function is for the server to get the value of a variable of the plugin.
     */
    template <typename T>
    std::optional<T> GetVariable(const std::string &name) const
    {
        return m_VariableRegistry->GetVariable<T>(name);
    }

    /**
     * @brief Gets the information about the variable with the specified name.
     *
     * @param name The name of the variable.
     * @return The information about the variable.
     */
    std::string GetVariableInfo(const std::string &name) const
    {
        return m_VariableRegistry->GetDescription(name);
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

    std::function<void(const json &)> GetFunc(const std::string &name)
    {
        return m_CommandDispatcher->GetHandler(name);
    }

    /**
     * @brief Gets the information about the function with the specified name.
     *
     * @param name The name of the function.
     * @return The function information in JSON format.
     */
    json GetFuncInfo(const std::string &name);

private:
    std::string m_Name;

    std::unique_ptr<CommandDispatcher<void, json>> m_CommandDispatcher; ///< The command dispatcher for handling functions.
    std::unique_ptr<VariableRegistry> m_VariableRegistry;               ///< The variable registry for managing variables.
    std::shared_ptr<PackageInfo> m_PackageInfo;                         ///< The package information for the plugin.
};
